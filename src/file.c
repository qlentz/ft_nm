/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static void	close_fd(int *fd)
{
	if (*fd >= 0)
		close(*fd);
	*fd = -1;
}

static const char	*stat_error(const struct stat *st)
{
	if (S_ISDIR(st->st_mode))
		return ("is a directory");
	if (!S_ISREG(st->st_mode))
		return ("not an ordinary file");
	if (st->st_size <= 0)
		return ("file format not recognized");
	return (NULL);
}

static int	open_for_map(const char *path, t_file *file)
{
	const char	*err;

	file->fd = open(path, O_RDONLY);
	if (file->fd < 0)
		return (nm_error(path, strerror(errno)));
	if (fstat(file->fd, &file->st) < 0)
		err = strerror(errno);
	else
		err = stat_error(&file->st);
	if (err)
	{
		close_fd(&file->fd);
		return (nm_error(path, err));
	}
	file->size = file->st.st_size;
	return (0);
}

int	map_file(const char *path, t_file *file)
{
	if (!path || !file)
		return (1);
	ft_bzero(file, sizeof(*file));
	file->fd = -1;
	file->path = path;
	if (open_for_map(path, file) != 0)
		return (1);
	file->mem = mmap(NULL, file->size, PROT_READ, MAP_PRIVATE, file->fd, 0);
	if (file->mem == MAP_FAILED)
	{
		file->mem = NULL;
		close_fd(&file->fd);
		return (nm_error(path, strerror(errno)));
	}
	close_fd(&file->fd);
	return (0);
}

void	free_file(t_file *file)
{
	if (!file)
		return ;
	if (file->mem)
	{
		munmap(file->mem, file->size);
		file->mem = NULL;
	}
	close_fd(&file->fd);
}
