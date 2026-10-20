/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   symtab.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static int	check_symtab_shape(const t_nmctx *ctx, const t_section_meta *meta)
{
	if (meta->entsize != elf_sym_size(&ctx->meta))
		return (nm_error(ctx->file->path, "invalid symbol entry size"));
	if (meta->size % meta->entsize != 0)
		return (nm_error(ctx->file->path, "corrupted symbol table size"));
	return (0);
}

static int	load_string_table(const t_nmctx *ctx, const t_section_meta *meta,
		t_symtab_slice *dst)
{
	t_section_meta	str;

	if (meta->link >= ctx->sht.count)
		return (nm_error(ctx->file->path, "invalid string table link"));
	if (read_section_meta(ctx, meta->link, &str) != 0)
		return (nm_error(ctx->file->path, "truncated string table header"));
	if (str.type != SHT_STRTAB)
		return (nm_error(ctx->file->path, "symbol table link is not strtab"));
	dst->str_base = offptr(ctx->file, str.offset, str.size);
	if (!dst->str_base && str.size)
		return (nm_error(ctx->file->path, "string table truncated"));
	dst->str_size = str.size;
	return (0);
}

int	populate_symtab_slice(t_nmctx *ctx, const t_section_meta *meta,
		t_symtab_slice *dst)
{
	if (check_symtab_shape(ctx, meta) != 0)
		return (1);
	dst->sym_base = offptr(ctx->file, meta->offset, meta->size);
	if (!dst->sym_base && meta->size)
		return (nm_error(ctx->file->path, "symbol table truncated"));
	dst->sym_size = meta->size;
	dst->entry_size = meta->entsize;
	dst->count = meta->size / meta->entsize;
	return (load_string_table(ctx, meta, dst));
}
