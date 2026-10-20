/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   symbol_type.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static char	lower_if_local(char c, unsigned char info)
{
	if (c >= 'A' && c <= 'Z' && elf_st_bind(info) == STB_LOCAL)
		return (c - 'A' + 'a');
	return (c);
}

static char	weak_letter(unsigned char type, const t_symbol_view *sym)
{
	if (type == STT_OBJECT && sym->shndx == SHN_UNDEF)
		return ('v');
	if (type == STT_OBJECT)
		return ('V');
	if (sym->shndx == SHN_UNDEF)
		return ('w');
	return ('W');
}

static char	binding_letter(const t_symbol_view *sym)
{
	unsigned char	bind;

	bind = elf_st_bind(sym->info);
	if (bind == STB_GNU_UNIQUE && sym->shndx != SHN_UNDEF)
		return ('u');
	if (bind == STB_WEAK)
		return (weak_letter(elf_st_type(sym->info), sym));
	return (0);
}

static char	section_letter(const t_nmctx *ctx, const t_symbol_view *sym)
{
	t_section_meta	sect;

	if (sym->shndx >= ctx->sht.count)
		return ('?');
	if (read_section_meta(ctx, sym->shndx, &sect) != 0)
		return ('?');
	if (sect.type == SHT_NOBITS && (sect.flags & SHF_ALLOC)
		&& (sect.flags & SHF_WRITE))
		return ('B');
	if ((sect.flags & SHF_ALLOC) && (sect.flags & SHF_EXECINSTR))
		return ('T');
	if ((sect.flags & SHF_ALLOC) && (sect.flags & SHF_WRITE))
		return ('D');
	if (sect.flags & SHF_ALLOC)
		return ('R');
	return ('N');
}

/*
** Same order of checks as binutils' bfd_decode_symclass:
** IFUNC first, then weak/unique bindings, then undefined/absolute/common,
** then the letter of the section the symbol lives in.
*/
char	classify_symbol(const t_nmctx *ctx, const t_symbol_view *sym)
{
	char	c;

	if (elf_st_type(sym->info) == STT_GNU_IFUNC && sym->shndx != SHN_UNDEF)
		return ('i');
	c = binding_letter(sym);
	if (!c && sym->shndx == SHN_UNDEF)
		c = 'U';
	else if (!c && sym->shndx == SHN_ABS)
		c = 'A';
	else if (!c && sym->shndx == SHN_COMMON)
		c = 'C';
	else if (!c)
		c = section_letter(ctx, sym);
	return (lower_if_local(c, sym->info));
}
