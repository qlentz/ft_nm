/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   archive.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:09:49 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:09:49 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

int	is_archive(const t_file *file)
{
	return (file->size >= AR_MAGIC_LEN
		&& (!ft_memcmp(file->mem, AR_MAGIC, AR_MAGIC_LEN)
			|| !ft_memcmp(file->mem, AR_THIN_MAGIC, AR_MAGIC_LEN)));
}

/* Reads the 60-byte member header at ar->offset; 1 if it is malformed. */
static int	read_member(t_archive *ar, t_ar_member *m)
{
	size_t	left;

	left = ar->file->size - ar->offset;
	if (left < AR_HEADER_LEN)
		return (1);
	m->header = (const char *)ar->file->mem + ar->offset;
	if (m->header[58] != '`' || m->header[59] != '\n'
		|| parse_decimal(m->header + 48, 10, &m->size) != 0
		|| m->size > left - AR_HEADER_LEN)
		return (1);
	m->data = (const unsigned char *)m->header + AR_HEADER_LEN;
	ar->offset += AR_HEADER_LEN + m->size + (m->size & 1);
	return (0);
}

static int	is_special(const char *header, const char *name)
{
	size_t	len;

	len = ft_strlen(name);
	return (!ft_strncmp(header, name, len) && header[len] == ' ');
}

/* Lists one member like a file of its own, skipping the index members. */
static int	process_member(t_archive *ar, const t_ar_member *m)
{
	t_file	member;
	int		ret;

	if (is_special(m->header, "//"))
	{
		ar->names = (const char *)m->data;
		ar->names_size = m->size;
		return (0);
	}
	if (is_special(m->header, "/") || is_special(m->header, "/SYM64/"))
		return (0);
	ft_bzero(&member, sizeof(member));
	ret = member_name(ar, m->header, (char **)&member.path);
	if (ret != 0 && ret != 2)
		return (nm_error(ar->file->path, "malformed archive member name"));
	if (ret == 2)
		return (nm_error(ar->file->path, strerror(ENOMEM)));
	member.fd = -1;
	member.mem = (void *)m->data;
	member.size = m->size;
	ret = process_elf(&member, 1);
	free((char *)member.path);
	return (ret);
}

/*
** Choices: a malformed archive stops with an error and exit status 1
** (nm exits 0), and thin archives, which only reference outside files,
** are not supported.
*/
int	process_archive(const t_file *file, int show_name)
{
	t_archive	ar;
	t_ar_member	m;
	int			status;

	if (!ft_memcmp(file->mem, AR_THIN_MAGIC, AR_MAGIC_LEN))
		return (nm_error(file->path, "thin archives are not supported"));
	if (show_name)
		print_file_header(file->path);
	ft_bzero(&ar, sizeof(ar));
	ar.file = file;
	ar.offset = AR_MAGIC_LEN;
	status = 0;
	while (ar.offset < file->size)
	{
		if (read_member(&ar, &m) != 0)
			return (nm_error(file->path, "malformed archive"));
		if (process_member(&ar, &m) != 0)
			status = 1;
	}
	return (status);
}
