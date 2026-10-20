/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   section_table.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static void	read_sht_fields(const t_nmctx *ctx, t_sht_fields *f)
{
	int	le;

	le = ctx->meta.is_le;
	if (ctx->meta.is_64)
	{
		f->offset = (size_t)r_u64(&ctx->hdr64->e_shoff, le);
		f->entsize = r_u16(&ctx->hdr64->e_shentsize, le);
		f->count = r_u16(&ctx->hdr64->e_shnum, le);
		f->strndx = r_u16(&ctx->hdr64->e_shstrndx, le);
	}
	else
	{
		f->offset = (size_t)r_u32(&ctx->hdr32->e_shoff, le);
		f->entsize = r_u16(&ctx->hdr32->e_shentsize, le);
		f->count = r_u16(&ctx->hdr32->e_shnum, le);
		f->strndx = r_u16(&ctx->hdr32->e_shstrndx, le);
	}
}

int	load_section_table(t_nmctx *ctx)
{
	t_sht_fields		f;
	const unsigned char	*base;

	read_sht_fields(ctx, &f);
	if (f.entsize != elf_shdr_size(&ctx->meta) || !f.count
		|| f.strndx >= f.count)
		return (nm_error(ctx->file->path, "invalid section header table"));
	base = offptr(ctx->file, f.offset, (size_t)f.entsize * f.count);
	if (!base)
		return (nm_error(ctx->file->path, "section header table truncated"));
	ctx->sht.entry_size = f.entsize;
	ctx->sht.count = f.count;
	ctx->sht.size_bytes = (size_t)f.entsize * f.count;
	ctx->sht.base = base;
	return (0);
}

int	load_symbol_tables(t_nmctx *ctx)
{
	t_section_meta	meta;
	size_t			i;

	i = 0;
	while (i < ctx->sht.count)
	{
		if (read_section_meta(ctx, i, &meta) != 0)
			return (nm_error(ctx->file->path, "invalid section header"));
		if (meta.type == SHT_SYMTAB)
		{
			if (populate_symtab_slice(ctx, &meta, &ctx->symtab) != 0)
				return (1);
			ctx->symtab.present = 1;
			return (0);
		}
		i++;
	}
	return (0);
}
