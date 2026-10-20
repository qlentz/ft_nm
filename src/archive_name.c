/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   archive_name.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:09:49 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:09:49 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/* Parses a decimal field padded with spaces, as used in ar headers. */
int	parse_decimal(const char *field, size_t len, size_t *out)
{
	size_t	i;

	*out = 0;
	i = 0;
	while (i < len && field[i] >= '0' && field[i] <= '9')
	{
		if (*out > (SIZE_MAX - (size_t)(field[i] - '0')) / 10)
			return (1);
		*out = *out * 10 + (size_t)(field[i] - '0');
		i++;
	}
	if (i == 0)
		return (1);
	while (i < len && field[i] == ' ')
		i++;
	return (i != len);
}

/* Archive names are not NUL-terminated, so they are copied by length. */
static int	dup_name(const char *src, size_t len, char **out)
{
	if (len == 0)
		return (1);
	*out = malloc(len + 1);
	if (!*out)
		return (2);
	ft_memcpy(*out, src, len);
	(*out)[len] = '\0';
	return (0);
}

/* GNU short name: at most 16 characters, ended by '/'. */
static int	short_name(const char *header, char **out)
{
	size_t	len;

	len = 0;
	while (len < 16 && header[len] != '/')
		len++;
	if (len == 16)
	{
		while (len > 0 && header[len - 1] == ' ')
			len--;
	}
	return (dup_name(header, len, out));
}

/* GNU long name "/<offset>": the name is in the "//" member, ended by "/\n". */
static int	long_name(const t_archive *ar, const char *header, char **out)
{
	size_t	offset;
	size_t	len;

	if (!ar->names || parse_decimal(header + 1, 15, &offset) != 0
		|| offset >= ar->names_size)
		return (1);
	len = 0;
	while (offset + len < ar->names_size && ar->names[offset + len] != '\n')
		len++;
	if (len > 0 && ar->names[offset + len - 1] == '/')
		len--;
	return (dup_name(ar->names + offset, len, out));
}

/* Returns 0 with a malloc'd name, 1 if the name is malformed, 2 on ENOMEM. */
int	member_name(const t_archive *ar, const char *header, char **out)
{
	if (header[0] == '/' && header[1] >= '0' && header[1] <= '9')
		return (long_name(ar, header, out));
	return (short_name(header, out));
}
