/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/18 16:00:28 by qlentz            #+#    #+#             */
/*   Updated: 2022/12/01 19:10:15 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static char	*read_next_newline(int fd)
{
	char	*buf;
	char	*newline;
	int		r;

	newline = NULL;
	buf = (char *)malloc(sizeof(char) * BUFFER_SIZE + 1);
	if (!buf)
		return (NULL);
	r = read(fd, buf, BUFFER_SIZE);
	if (r < 1)
	{
		free_data(&buf);
		return (NULL);
	}
	buf[r] = 0;
	newline = ft_strjoin_gnl(newline, buf);
	while (!ft_strchr(buf, '\n') && r)
	{
		r = read(fd, buf, BUFFER_SIZE);
		buf[r] = 0;
		newline = ft_strjoin_gnl(newline, buf);
	}
	free_data(&buf);
	return (newline);
}

static char	*process_rest(char *rest, int fd)
{
	char	*newline;

	if (!rest)
	{
		rest = (char *)malloc(sizeof(char) * 1);
		if (!rest)
			return (NULL);
		rest[0] = 0;
	}
	if (!ft_strchr(rest, '\n'))
	{
		newline = read_next_newline(fd);
		if (newline)
		{
			rest = ft_strjoin_gnl(rest, newline);
			free_data(&newline);
		}
	}
	return (rest);
}

static size_t	ft_scandale(char	*string)
{
	size_t	size;

	size = (ft_strchr(string, '\n') - string + 1);
	return (size);
}

char	*get_next_line(int fd)
{
	char		*line;
	static char	*rest[4096];
	char		*temp;

	if (fd < 0)
		return (NULL);
	if (!rest[fd])
		rest[fd] = NULL;
	rest[fd] = process_rest(rest[fd], fd);
	if (ft_strlen(rest[fd]) == 0)
	{
		free_data(&rest[fd]);
		return (NULL);
	}
	if (ft_strchr(rest[fd], '\n'))
	{
		line = ft_substr(rest[fd], 0, ft_scandale(rest[fd]));
		temp = ft_strdup(ft_strchr(rest[fd], '\n') + 1);
		free_data(&rest[fd]);
		rest[fd] = temp;
		return (line);
	}
	line = ft_strdup(rest[fd]);
	free_data(&rest[fd]);
	return (line);
}
