/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   section_table.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:12:02 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static void	read_sht_fields(const t_nmctx *ctx, t_sht_fields *f)
{
	int	le;

	le = ctx->meta.is_le;
	if (ctx->meta.is_64)
	{
		f->type = r_u16(&ctx->hdr64->e_type, le);
		f->offset = (size_t)r_u64(&ctx->hdr64->e_shoff, le);
		f->entsize = r_u16(&ctx->hdr64->e_shentsize, le);
		f->count = r_u16(&ctx->hdr64->e_shnum, le);
		f->strndx = r_u16(&ctx->hdr64->e_shstrndx, le);
	}
	else
	{
		f->type = r_u16(&ctx->hdr32->e_type, le);
		f->offset = (size_t)r_u32(&ctx->hdr32->e_shoff, le);
		f->entsize = r_u16(&ctx->hdr32->e_shentsize, le);
		f->count = r_u16(&ctx->hdr32->e_shnum, le);
		f->strndx = r_u16(&ctx->hdr32->e_shstrndx, le);
	}
}

/*
** Extended section numbering: with too many sections for the 16-bit
** header fields, the real count is in section 0's sh_size and the real
** string table index in its sh_link.
*/
static int	resolve_extended(t_nmctx *ctx, t_sht_fields *f)
{
	t_section_meta	first;

	ctx->sht.base = offptr(ctx->file, f->offset, f->entsize);
	ctx->sht.entry_size = f->entsize;
	ctx->sht.count = 1;
	ctx->sht.size_bytes = f->entsize;
	if (read_section_meta(ctx, 0, &first) != 0)
		return (nm_error(ctx->file->path, "section header table truncated"));
	if (f->count == 0)
		f->count = first.size;
	if (f->strndx == SHN_XINDEX)
		f->strndx = first.link;
	return (0);
}

/* Section names come from the e_shstrndx string table, if it is valid. */
static void	load_section_names(t_nmctx *ctx, size_t strndx)
{
	t_section_meta	meta;

	if (strndx == SHN_UNDEF || read_section_meta(ctx, strndx, &meta) != 0
		|| meta.type != SHT_STRTAB)
		return ;
	ctx->sht.names = offptr(ctx->file, meta.offset, meta.size);
	if (ctx->sht.names)
		ctx->sht.names_size = meta.size;
}

/*
** Without a section table (e_shoff == 0) there are no symbols; like nm,
** that is only accepted for executables and shared objects.
*/
int	load_section_table(t_nmctx *ctx)
{
	t_sht_fields	f;
	size_t			bytes;

	read_sht_fields(ctx, &f);
	if (f.offset == 0 && f.type == ET_REL)
		return (nm_error(ctx->file->path, "file format not recognized"));
	if (f.offset == 0)
		return (0);
	if (f.entsize != elf_shdr_size(&ctx->meta))
		return (nm_error(ctx->file->path, "invalid section header table"));
	if ((f.count == 0 || f.strndx == SHN_XINDEX)
		&& resolve_extended(ctx, &f) != 0)
		return (1);
	if (!f.count || f.strndx >= f.count
		|| mul_overflows_size_t(f.entsize, f.count, &bytes))
		return (nm_error(ctx->file->path, "invalid section header table"));
	ctx->sht.base = offptr(ctx->file, f.offset, bytes);
	if (!ctx->sht.base)
		return (nm_error(ctx->file->path, "section header table truncated"));
	ctx->sht.entry_size = f.entsize;
	ctx->sht.count = f.count;
	ctx->sht.size_bytes = bytes;
	load_section_names(ctx, f.strndx);
	return (0);
}

const char	*section_name(const t_nmctx *ctx, const t_section_meta *meta)
{
	const char	*name;

	name = safe_cstr_in_table(ctx->sht.names, ctx->sht.names_size,
			meta->name_index);
	if (!name)
		return ("");
	return (name);
}
