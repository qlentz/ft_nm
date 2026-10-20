/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   symbol_read.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:12:02 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static void	fill_symbol64(const t_elf64sym *sym, int le, t_symbol_view *view)
{
	view->name = r_u32(&sym->st_name, le);
	view->info = sym->st_info;
	view->shndx = r_u16(&sym->st_shndx, le);
	view->value = r_u64(&sym->st_value, le);
	view->size = r_u64(&sym->st_size, le);
}

static void	fill_symbol32(const t_elf32sym *sym, int le, t_symbol_view *view)
{
	view->name = r_u32(&sym->st_name, le);
	view->info = sym->st_info;
	view->shndx = r_u16(&sym->st_shndx, le);
	view->value = r_u32(&sym->st_value, le);
	view->size = r_u32(&sym->st_size, le);
}

static int	resolve_xindex(const t_nmctx *ctx, size_t index,
		t_symbol_view *view)
{
	if (!ctx->symtab.shndx_base || index >= ctx->symtab.shndx_count)
		return (2);
	view->shndx = r_u32(ctx->symtab.shndx_base + index * sizeof(uint32_t),
			ctx->meta.is_le);
	return (0);
}

/*
** Returns 0 on success, 1 if the entry is out of bounds, and 2 if it uses
** SHN_XINDEX without an SHT_SYMTAB_SHNDX entry.
*/
int	read_symbol_view(const t_nmctx *ctx, size_t index, t_symbol_view *view)
{
	size_t				offset;
	size_t				stride;
	const unsigned char	*raw;

	stride = ctx->symtab.entry_size;
	if (mul_overflows_size_t(index, stride, &offset))
		return (1);
	if (offset + stride > ctx->symtab.sym_size)
		return (1);
	raw = ctx->symtab.sym_base + offset;
	if (ctx->meta.is_64)
		fill_symbol64((const t_elf64sym *)raw, ctx->meta.is_le, view);
	else
		fill_symbol32((const t_elf32sym *)raw, ctx->meta.is_le, view);
	if (view->shndx == SHN_XINDEX)
		return (resolve_xindex(ctx, index, view));
	return (0);
}

/* Like nm, section and file symbols are not listed. */
int	should_skip_symbol(const t_symbol_view *view)
{
	unsigned char	type;

	type = elf_st_type(view->info);
	return (type == STT_FILE || type == STT_SECTION);
}
