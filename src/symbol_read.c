/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   symbol_read.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static void	fill_symbol64(const t_elf64sym *sym, int le, t_symbol_view *view)
{
	view->name = r_u32(&sym->st_name, le);
	view->info = sym->st_info;
	view->shndx = r_u16(&sym->st_shndx, le);
	view->value = r_u64(&sym->st_value, le);
}

static void	fill_symbol32(const t_elf32sym *sym, int le, t_symbol_view *view)
{
	view->name = r_u32(&sym->st_name, le);
	view->info = sym->st_info;
	view->shndx = r_u16(&sym->st_shndx, le);
	view->value = r_u32(&sym->st_value, le);
}

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
	return (0);
}

int	should_skip_symbol(const t_symbol_view *view, const char *name)
{
	unsigned char	type;

	if (!name || !name[0])
		return (1);
	type = elf_st_type(view->info);
	if (type == STT_FILE || type == STT_SECTION)
		return (1);
	return (0);
}
