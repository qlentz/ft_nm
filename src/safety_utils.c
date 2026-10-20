/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   safety_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/16 21:39:12 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/*
** Returns a pointer to [off, off + len) inside the mapped file,
** or NULL if that range does not fit. Written to avoid overflow.
*/
const void	*offptr(const t_file *f, size_t off, size_t len)
{
	if (!f || !f->mem || off > f->size || len > f->size - off)
		return (NULL);
	return ((const unsigned char *)f->mem + off);
}

int	mul_overflows_size_t(size_t a, size_t b, size_t *out)
{
	if (!out)
		return (1);
	if (a == 0 || b == 0)
	{
		*out = 0;
		return (0);
	}
	if (a > SIZE_MAX / b)
		return (1);
	*out = a * b;
	return (0);
}

/*
** Returns the string at `off` in a string table of `sz` bytes,
** or NULL if the offset is out of range or the string is not terminated.
*/
const char	*safe_cstr_in_table(const char *base, size_t sz, size_t off)
{
	size_t	i;

	if (!base || off >= sz)
		return (NULL);
	i = off;
	while (i < sz)
	{
		if (base[i] == '\0')
			return (base + off);
		i++;
	}
	return (NULL);
}
