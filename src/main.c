#include "ft_nm.h"

typedef struct s_section_meta
{
	uint32_t	type;
	uint32_t	name_index;
	size_t		offset;
	size_t		size;
	uint32_t	link;
	uint32_t	info;
	size_t		entsize;
}	t_section_meta;

static void	free_file(t_file *file)
{
	if (!file)
		return;
	if (file->mem) {
		munmap(file->mem, file->size);
		file->mem = NULL;
	}
	if (file->fd >= 0) {
		close(file->fd);
		file->fd = -1;
	}
}

static int	map_file(const char *path, t_file *container)
{
	if (!path || !container)
		return 1;
	container->mem = NULL;
	container->size = 0;
	container->fd = open(path, O_RDONLY);
	if (container->fd < 0)
		return 1;
	if (fstat(container->fd, &container->st) < 0) {
		close(container->fd);
		container->fd = -1;
		return 1;
	}
	if (container->st.st_size <= 0) {
		close(container->fd);
		container->fd = -1;
		return 1;
	}
	container->size = container->st.st_size;
	container->mem = mmap(NULL, container->size, PROT_READ, MAP_PRIVATE,
			container->fd, 0);
	if (container->mem == MAP_FAILED) {
		container->mem = NULL;
		close(container->fd);
		container->fd = -1;
		return 1;
	}
	close(container->fd);
	container->fd = -1;
	return 0;
}

static int	init_header_view(const t_file *file, t_nmctx *ctx)
{
	size_t			header_size;
	const void		*view;

	if (!file || !ctx)
		return 1;
	header_size = ctx->meta.is_64 ? sizeof(t_elf64header) : sizeof(t_elf32header);
	view = offptr(file, 0, header_size);
	if (!view) {
		ft_putendl_fd("ft_nm: truncated ELF header", STDERR_FILENO);
		return 1;
	}
	if (ctx->meta.is_64) {
		ctx->hdr64 = view;
		ctx->hdr32 = NULL;
	} else {
		ctx->hdr32 = view;
		ctx->hdr64 = NULL;
	}
	return 0;
}

static int	init_nmctx(t_file *file, t_nmctx *ctx)
{
	if (!file || !ctx)
		return 1;
	ft_bzero(ctx, sizeof(*ctx));
	ctx->file = file;
	if (elf_ident(file, &ctx->meta) != 0)
		return 1;
	if (init_header_view(file, ctx) != 0)
		return 1;
	return 0;
}

static int	load_section_table(t_nmctx *ctx)
{
	size_t				offset;
	uint16_t			entry_size;
	uint16_t			count;
	uint16_t			str_index;
	size_t				table_bytes;
	const unsigned char	*table_base;

	if (!ctx)
		return 1;
	if (ctx->meta.is_64) {
		offset = (size_t)r_u64(&ctx->hdr64->e_shoff, ctx->meta.is_le);
		entry_size = r_u16(&ctx->hdr64->e_shentsize, ctx->meta.is_le);
		count = r_u16(&ctx->hdr64->e_shnum, ctx->meta.is_le);
		str_index = r_u16(&ctx->hdr64->e_shstrndx, ctx->meta.is_le);
	} else {
		offset = (size_t)r_u32(&ctx->hdr32->e_shoff, ctx->meta.is_le);
		entry_size = r_u16(&ctx->hdr32->e_shentsize, ctx->meta.is_le);
		count = r_u16(&ctx->hdr32->e_shnum, ctx->meta.is_le);
		str_index = r_u16(&ctx->hdr32->e_shstrndx, ctx->meta.is_le);
	}
	if (entry_size == 0 || count == 0) {
		ft_putendl_fd("ft_nm: invalid section header table", STDERR_FILENO);
		return 1;
	}
	if (str_index >= count) {
		ft_putendl_fd("ft_nm: invalid section string table index", STDERR_FILENO);
		return 1;
	}
	if (mul_overflows_size_t((size_t)entry_size, (size_t)count, &table_bytes)) {
		ft_putendl_fd("ft_nm: section header table overflow", STDERR_FILENO);
		return 1;
	}
	table_base = offptr(ctx->file, offset, table_bytes);
	if (!table_base) {
		ft_putendl_fd("ft_nm: section header table truncated", STDERR_FILENO);
		return 1;
	}
	ctx->sht.offset = offset;
	ctx->sht.entry_size = entry_size;
	ctx->sht.count = count;
	ctx->sht.str_index = str_index;
	ctx->sht.base = table_base;
	return 0;
}

static const unsigned char	*section_header_view(const t_nmctx *ctx, size_t index)
{
	size_t	offset;

	if (!ctx || !ctx->sht.base || index >= ctx->sht.count)
		return NULL;
	if (mul_overflows_size_t(index, ctx->sht.entry_size, &offset))
		return NULL;
	return ctx->sht.base + offset;
}

static int	read_section_meta(const t_nmctx *ctx, size_t index, t_section_meta *meta)
{
	const unsigned char	*raw;

	if (!ctx || !meta)
		return 1;
	raw = section_header_view(ctx, index);
	if (!raw)
		return 1;
	ft_bzero(meta, sizeof(*meta));
	if (ctx->meta.is_64)
	{
		const t_elf64shdr	*sh = (const t_elf64shdr *)raw;

		meta->name_index = r_u32(&sh->sh_name, ctx->meta.is_le);
		meta->type = r_u32(&sh->sh_type, ctx->meta.is_le);
		meta->offset = (size_t)r_u64(&sh->sh_offset, ctx->meta.is_le);
		meta->size = (size_t)r_u64(&sh->sh_size, ctx->meta.is_le);
		meta->link = r_u32(&sh->sh_link, ctx->meta.is_le);
		meta->info = r_u32(&sh->sh_info, ctx->meta.is_le);
		meta->entsize = (size_t)r_u64(&sh->sh_entsize, ctx->meta.is_le);
	}
	else
	{
		const t_elf32shdr	*sh = (const t_elf32shdr *)raw;

		meta->name_index = r_u32(&sh->sh_name, ctx->meta.is_le);
		meta->type = r_u32(&sh->sh_type, ctx->meta.is_le);
		meta->offset = (size_t)r_u32(&sh->sh_offset, ctx->meta.is_le);
		meta->size = (size_t)r_u32(&sh->sh_size, ctx->meta.is_le);
		meta->link = r_u32(&sh->sh_link, ctx->meta.is_le);
		meta->info = r_u32(&sh->sh_info, ctx->meta.is_le);
		meta->entsize = (size_t)r_u32(&sh->sh_entsize, ctx->meta.is_le);
	}
	return 0;
}

static int	populate_symtab_slice(t_nmctx *ctx, const t_section_meta *meta,
		t_symtab_slice *dst)
{
	const unsigned char	*sym_base;
	t_section_meta		str_meta;

	if (!ctx || !meta || !dst)
		return 1;
	if (meta->entsize == 0)
		return ft_putendl_fd("ft_nm: invalid symbol entry size", STDERR_FILENO), 1;
	if (meta->size % meta->entsize != 0)
		return ft_putendl_fd("ft_nm: corrupted symbol table size", STDERR_FILENO), 1;
	if (meta->link >= ctx->sht.count)
		return ft_putendl_fd("ft_nm: invalid string table link", STDERR_FILENO), 1;
	if (read_section_meta(ctx, meta->link, &str_meta) != 0)
		return ft_putendl_fd("ft_nm: truncated string table header", STDERR_FILENO), 1;
	if (str_meta.type != SHT_STRTAB)
		return ft_putendl_fd("ft_nm: symbol table link is not strtab", STDERR_FILENO), 1;
	sym_base = offptr(ctx->file, meta->offset, meta->size);
	if (!sym_base && meta->size)
		return ft_putendl_fd("ft_nm: symbol table truncated", STDERR_FILENO), 1;
	dst->sym_base = sym_base;
	dst->sym_size = meta->size;
	dst->entry_size = meta->entsize;
	dst->count = (meta->entsize == 0) ? 0 : meta->size / meta->entsize;
	dst->str_base = (const char *)offptr(ctx->file, str_meta.offset, str_meta.size);
	if (!dst->str_base && str_meta.size)
		return ft_putendl_fd("ft_nm: string table truncated", STDERR_FILENO), 1;
	dst->str_size = str_meta.size;
	dst->present = 1;
	return 0;
}

static int	load_symbol_tables(t_nmctx *ctx)
{
	size_t			i;
	t_section_meta	meta;

	if (!ctx)
		return 1;
	i = 0;
	while (i < ctx->sht.count && (!ctx->symtab.present || !ctx->dynsym.present))
	{
		if (read_section_meta(ctx, i, &meta) != 0)
			return 1;
		if (meta.type == SHT_SYMTAB && !ctx->symtab.present)
		{
			if (populate_symtab_slice(ctx, &meta, &ctx->symtab) != 0)
				return 1;
		}
		else if (meta.type == SHT_DYNSYM && !ctx->dynsym.present)
		{
			if (populate_symtab_slice(ctx, &meta, &ctx->dynsym) != 0)
				return 1;
		}
		++i;
	}
	return 0;
}

static int	process_elf(t_file *file)
{
	t_nmctx	ctx;

	if (init_nmctx(file, &ctx) != 0)
		return 1;
	if (load_section_table(&ctx) != 0)
		return 1;
	if (load_symbol_tables(&ctx) != 0)
		return 1;
	return 0;
}
int	ft_nm(char *filename)
{
	t_file	file;
	int		ret;

	ft_bzero(&file, sizeof(file));
	file.fd = -1;
	ret = 1;
	if (map_file(filename, &file) != 0) {
		ft_putstr_fd("ft_nm: failed to map ", STDERR_FILENO);
		ft_putendl_fd(filename, STDERR_FILENO);
		return ret;
	}
	if (process_elf(&file) == 0)
		ret = 0;
	free_file(&file);
	return ret;
}

int main(int ac, char **av) {
	if (ac == 1) {
		ft_nm(DEFAULT_FILE);
	} else {
		while (--ac) {
			ft_nm(*++av);
		}
	}
	return 0;
}
