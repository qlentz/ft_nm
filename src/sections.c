#include "ft_nm.h"

const unsigned char	*section_header_view(const t_nmctx *ctx, size_t index)
{
	size_t	offset;
	size_t	stride;

	if (!ctx || !ctx->sht.base || index >= ctx->sht.count)
		return (NULL);
	stride = ctx->sht.entry_size;
	if (mul_overflows_size_t(index, stride, &offset))
		return (NULL);
	if (offset + stride > ctx->sht.size_bytes)
		return (NULL);
	return (ctx->sht.base + offset);
}

static void	fill_meta64(const t_nmctx *ctx,
		const t_elf64shdr *sh, t_section_meta *meta)
{
	meta->name_index = r_u32(&sh->sh_name, ctx->meta.is_le);
	meta->type = r_u32(&sh->sh_type, ctx->meta.is_le);
	meta->offset = (size_t)r_u64(&sh->sh_offset, ctx->meta.is_le);
	meta->size = (size_t)r_u64(&sh->sh_size, ctx->meta.is_le);
	meta->link = r_u32(&sh->sh_link, ctx->meta.is_le);
	meta->info = r_u32(&sh->sh_info, ctx->meta.is_le);
	meta->entsize = (size_t)r_u64(&sh->sh_entsize, ctx->meta.is_le);
	meta->flags = r_u64(&sh->sh_flags, ctx->meta.is_le);
}

static void	fill_meta32(const t_nmctx *ctx,
		const t_elf32shdr *sh, t_section_meta *meta)
{
	meta->name_index = r_u32(&sh->sh_name, ctx->meta.is_le);
	meta->type = r_u32(&sh->sh_type, ctx->meta.is_le);
	meta->offset = (size_t)r_u32(&sh->sh_offset, ctx->meta.is_le);
	meta->size = (size_t)r_u32(&sh->sh_size, ctx->meta.is_le);
	meta->link = r_u32(&sh->sh_link, ctx->meta.is_le);
	meta->info = r_u32(&sh->sh_info, ctx->meta.is_le);
	meta->entsize = (size_t)r_u32(&sh->sh_entsize, ctx->meta.is_le);
	meta->flags = r_u32(&sh->sh_flags, ctx->meta.is_le);
}

int	read_section_meta(const t_nmctx *ctx, size_t index, t_section_meta *meta)
{
	const unsigned char	*raw;

	if (!ctx || !meta)
		return (1);
	raw = section_header_view(ctx, index);
	if (!raw)
		return (1);
	ft_bzero(meta, sizeof(*meta));
	if (ctx->meta.is_64)
		fill_meta64(ctx, (const t_elf64shdr *)raw, meta);
	else
		fill_meta32(ctx, (const t_elf32shdr *)raw, meta);
	return (0);
}

static int	check_symtab_shape(const t_section_meta *meta)
{
	if (!meta->entsize)
	{
		ft_putendl_fd("ft_nm: invalid symbol entry size", STDERR_FILENO);
		return (1);
	}
	if (meta->size % meta->entsize != 0)
	{
		ft_putendl_fd("ft_nm: corrupted symbol table size", STDERR_FILENO);
		return (1);
	}
	return (0);
}

static int	load_string_table(t_nmctx *ctx, const t_section_meta *meta,
		const char **base, size_t *size)
{
	t_section_meta	str;

	if (meta->link >= ctx->sht.count)
	{
		ft_putendl_fd("ft_nm: invalid string table link", STDERR_FILENO);
		return (1);
	}
	if (read_section_meta(ctx, meta->link, &str) != 0)
	{
		ft_putendl_fd("ft_nm: truncated string table header", STDERR_FILENO);
		return (1);
	}
	if (str.type != SHT_STRTAB)
	{
		ft_putendl_fd("ft_nm: symbol table link is not strtab", STDERR_FILENO);
		return (1);
	}
	*base = (const char *)offptr(ctx->file, str.offset, str.size);
	if (!*base && str.size)
	{
		ft_putendl_fd("ft_nm: string table truncated", STDERR_FILENO);
		return (1);
	}
	*size = str.size;
	return (0);
}

int	populate_symtab_slice(t_nmctx *ctx, const t_section_meta *meta,
		t_symtab_slice *dst)
{
	if (!ctx || !meta || !dst)
		return (1);
	if (check_symtab_shape(meta) != 0)
		return (1);
	dst->sym_base = offptr(ctx->file, meta->offset, meta->size);
	if (!dst->sym_base && meta->size)
	{
		ft_putendl_fd("ft_nm: symbol table truncated", STDERR_FILENO);
		return (1);
	}
	dst->sym_size = meta->size;
	dst->entry_size = meta->entsize;
	dst->count = (meta->entsize == 0) ? 0 : meta->size / meta->entsize;
	if (load_string_table(ctx, meta, &dst->str_base, &dst->str_size) != 0)
		return (1);
	return (0);
}
