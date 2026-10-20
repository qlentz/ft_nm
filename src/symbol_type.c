/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   symbol_type.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:12:02 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/* Non-allocated sections that binutils treats as debugging info. */
static int	is_debug_name(const char *name)
{
	return (!ft_strncmp(name, ".debug", 6) || !ft_strncmp(name, ".zdebug", 7)
		|| !ft_strncmp(name, ".gnu.debuglto_.debug_", 21)
		|| !ft_strncmp(name, ".gnu.linkonce.wi.", 17)
		|| !ft_strncmp(name, ".line", 5) || !ft_strncmp(name, ".stab", 5)
		|| !ft_strcmp(name, ".gdb_index"));
}

/*
** Letters binutils picks from the section name alone (PE sections).
** The name may be followed by '.', '$' or a digit, as in ".idata$2".
*/
static char	name_letter(const char *name)
{
	static const char	*names[] = {".drectve", ".edata", ".idata", ".pdata"};
	static const char	letters[] = "ieip";
	size_t				len;
	size_t				i;

	i = 0;
	while (i < 4)
	{
		len = ft_strlen(names[i]);
		if (!ft_strncmp(name, names[i], len)
			&& (name[len] == '\0' || ft_strchr(".$0123456789", name[len])))
			return (letters[i]);
		i++;
	}
	return (0);
}

/* Same rules as binutils' decode_section_type, from the ELF flags. */
static char	section_letter(const t_nmctx *ctx, size_t index)
{
	t_section_meta	s;
	const char		*name;

	if (read_section_meta(ctx, index, &s) != 0)
		return ('?');
	name = section_name(ctx, &s);
	if (name_letter(name))
		return (name_letter(name));
	if (s.flags & SHF_EXECINSTR)
		return ('t');
	if ((s.flags & SHF_ALLOC) && s.type != SHT_NOBITS
		&& !(s.flags & SHF_WRITE))
		return ('r');
	if ((s.flags & SHF_ALLOC) && s.type != SHT_NOBITS)
		return ('d');
	if (s.type == SHT_NOBITS)
		return ('b');
	if (!(s.flags & SHF_ALLOC) && is_debug_name(name))
		return ('N');
	if (!(s.flags & SHF_WRITE))
		return ('n');
	return ('?');
}

static char	weak_letter(const t_symbol_view *sym)
{
	unsigned char	type;
	char			c;

	type = elf_st_type(sym->info);
	c = 'W';
	if (type == STT_OBJECT || type == STT_COMMON)
		c = 'V';
	if (sym->shndx == SHN_UNDEF)
		c = ft_tolower(c);
	return (c);
}

/*
** Same order of checks as binutils' bfd_decode_symclass.
** Choice: like nm, a section index past the table counts as absolute.
*/
char	classify_symbol(const t_nmctx *ctx, const t_symbol_view *sym)
{
	unsigned char	bind;
	char			c;

	bind = elf_st_bind(sym->info);
	if (sym->shndx == SHN_COMMON)
		return ('C');
	if (sym->shndx == SHN_UNDEF && bind == STB_WEAK)
		return (weak_letter(sym));
	if (sym->shndx == SHN_UNDEF)
		return ('U');
	if (elf_st_type(sym->info) == STT_GNU_IFUNC)
		return ('i');
	if (bind == STB_WEAK)
		return (weak_letter(sym));
	if (bind == STB_GNU_UNIQUE)
		return ('u');
	if (bind != STB_LOCAL && bind != STB_GLOBAL)
		return ('?');
	c = 'a';
	if (sym->shndx != SHN_ABS && sym->shndx < ctx->sht.count)
		c = section_letter(ctx, sym->shndx);
	if (bind == STB_GLOBAL)
		c = ft_toupper(c);
	return (c);
}
