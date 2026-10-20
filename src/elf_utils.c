/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

size_t	elf_header_size(const t_elfmeta *meta)
{
	if (meta->is_64)
		return (sizeof(t_elf64header));
	return (sizeof(t_elf32header));
}

size_t	elf_shdr_size(const t_elfmeta *meta)
{
	if (meta->is_64)
		return (sizeof(t_elf64shdr));
	return (sizeof(t_elf32shdr));
}

size_t	elf_sym_size(const t_elfmeta *meta)
{
	if (meta->is_64)
		return (sizeof(t_elf64sym));
	return (sizeof(t_elf32sym));
}

/* High 4 bits of st_info: binding (LOCAL, GLOBAL, WEAK, ...). */
unsigned char	elf_st_bind(unsigned char info)
{
	return (info >> 4);
}

/* Low 4 bits of st_info: type (OBJECT, FUNC, SECTION, ...). */
unsigned char	elf_st_type(unsigned char info)
{
	return (info & 0xf);
}
