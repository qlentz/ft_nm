#include "ft_nm.h"

static void	close_fd(int *fd)
{
	if (*fd >= 0)
		close(*fd);
	*fd = -1;
}

static int	open_for_map(const char *path, t_file *file)
{
	if (!path || !file)
		return (1);
	file->fd = open(path, O_RDONLY);
	if (file->fd < 0)
		return (1);
	if (fstat(file->fd, &file->st) < 0 || file->st.st_size <= 0)
	{
		close_fd(&file->fd);
		return (1);
	}
	file->size = file->st.st_size;
	return (0);
}

int	map_file(const char *path, t_file *file)
{
	if (!file)
		return (1);
	ft_bzero(file, sizeof(*file));
	file->fd = -1;
	if (open_for_map(path, file) != 0)
		return (1);
	file->mem = mmap(NULL, file->size, PROT_READ, MAP_PRIVATE, file->fd, 0);
	if (file->mem == MAP_FAILED)
	{
		file->mem = NULL;
		close_fd(&file->fd);
		return (1);
	}
	close_fd(&file->fd);
	return (0);
}

void	free_file(t_file *file)
{
	if (!file)
		return ;
	if (file->mem)
	{
		munmap(file->mem, file->size);
		file->mem = NULL;
	}
	close_fd(&file->fd);
}

static int	init_header_view(const t_file *file, t_nmctx *ctx)
{
	size_t	header_size;
	void	*view;

	header_size = ctx->meta.is_64 ? sizeof(t_elf64header) : sizeof(t_elf32header);
	view = (void *)offptr(file, 0, header_size);
	if (!view)
	{
		ft_putendl_fd("ft_nm: truncated ELF header", STDERR_FILENO);
		return (1);
	}
	if (ctx->meta.is_64)
		ctx->hdr64 = view;
	else
		ctx->hdr32 = view;
	return (0);
}

int	init_nmctx(t_file *file, t_nmctx *ctx)
{
	if (!file || !ctx)
		return (1);
	ft_bzero(ctx, sizeof(*ctx));
	ctx->file = file;
	if (elf_ident(file, &ctx->meta) != 0)
		return (1);
	return (init_header_view(file, ctx));
}

static void	store_sht_info(t_nmctx *ctx, size_t offset,
		uint16_t entsize, uint16_t count, uint16_t str,
		const unsigned char *base)
{
	ctx->sht.offset = offset;
	ctx->sht.entry_size = entsize;
	ctx->sht.count = count;
	ctx->sht.str_index = str;
	ctx->sht.size_bytes = (size_t)entsize * count;
	ctx->sht.base = base;
}

static void	read_sht_fields(t_nmctx *ctx, size_t *offset,
		uint16_t *entsize, uint16_t *count, uint16_t *str)
{
	if (ctx->meta.is_64)
	{
		*offset = (size_t)r_u64(&ctx->hdr64->e_shoff, ctx->meta.is_le);
		*entsize = r_u16(&ctx->hdr64->e_shentsize, ctx->meta.is_le);
		*count = r_u16(&ctx->hdr64->e_shnum, ctx->meta.is_le);
		*str = r_u16(&ctx->hdr64->e_shstrndx, ctx->meta.is_le);
	}
	else
	{
		*offset = (size_t)r_u32(&ctx->hdr32->e_shoff, ctx->meta.is_le);
		*entsize = r_u16(&ctx->hdr32->e_shentsize, ctx->meta.is_le);
		*count = r_u16(&ctx->hdr32->e_shnum, ctx->meta.is_le);
		*str = r_u16(&ctx->hdr32->e_shstrndx, ctx->meta.is_le);
	}
}

int	load_section_table(t_nmctx *ctx)
{
	size_t		offset;
	uint16_t	entsize;
	uint16_t	count;
	uint16_t	str;
	const unsigned char	*base;

	if (!ctx)
		return (1);
	read_sht_fields(ctx, &offset, &entsize, &count, &str);
	if (!entsize || !count || str >= count)
	{
		ft_putendl_fd("ft_nm: invalid section header table", STDERR_FILENO);
		return (1);
	}
	base = offptr(ctx->file, offset, (size_t)entsize * count);
	if (!base)
	{
		ft_putendl_fd("ft_nm: section header table truncated", STDERR_FILENO);
		return (1);
	}
	store_sht_info(ctx, offset, entsize, count, str, base);
	return (0);
}

static int	store_slice(t_nmctx *ctx, const t_section_meta *meta,
		t_symtab_slice *slice)
{
	if (slice->present)
		return (0);
	if (populate_symtab_slice(ctx, meta, slice) != 0)
		return (1);
	slice->present = 1;
	return (0);
}

static int	process_section(t_nmctx *ctx, size_t index)
{
	t_section_meta	meta;

	if (read_section_meta(ctx, index, &meta) != 0)
		return (1);
	if (meta.type == SHT_SYMTAB)
		return (store_slice(ctx, &meta, &ctx->symtab));
	if (meta.type == SHT_DYNSYM)
		return (store_slice(ctx, &meta, &ctx->dynsym));
	return (0);
}

int	load_symbol_tables(t_nmctx *ctx)
{
	size_t	i;

	if (!ctx)
		return (1);
	i = 0;
	while (i < ctx->sht.count)
	{
		if (process_section(ctx, i) != 0)
			return (1);
		if (ctx->symtab.present && ctx->dynsym.present)
			return (0);
		++i;
	}
	return (0);
}

int	process_elf(t_file *file, const char *filename)
{
	t_nmctx	ctx;

	if (init_nmctx(file, &ctx) != 0)
		return (1);
	if (load_section_table(&ctx) != 0)
		return (1);
	if (load_symbol_tables(&ctx) != 0)
		return (1);
	return (render_symbols(&ctx, filename));
}
