/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/16 21:39:12 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/*
** Reads an unsigned integer of `size` bytes stored in the file's byte order.
** Byte-by-byte access also avoids unaligned reads from the mapped file.
*/
static uint64_t	read_uint(const void *p, size_t size, int is_le)
{
	const uint8_t	*bytes;
	uint64_t		value;
	size_t			i;

	bytes = p;
	value = 0;
	i = 0;
	while (i < size)
	{
		if (is_le)
			value |= (uint64_t)bytes[i] << (8 * i);
		else
			value = (value << 8) | bytes[i];
		i++;
	}
	return (value);
}

uint16_t	r_u16(const void *p, int is_le)
{
	return ((uint16_t)read_uint(p, 2, is_le));
}

uint32_t	r_u32(const void *p, int is_le)
{
	return ((uint32_t)read_uint(p, 4, is_le));
}

uint64_t	r_u64(const void *p, int is_le)
{
	return (read_uint(p, 8, is_le));
}
