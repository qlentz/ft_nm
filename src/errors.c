/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:52:15 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 23:17:31 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/*
** stdout is buffered and stderr is not: flushing stdout first keeps the
** two streams in order when they go to the same place (2>&1).
*/
int	nm_error(const char *path, const char *msg)
{
	fflush(stdout);
	if (path)
		fprintf(stderr, "ft_nm: %s: %s\n", path, msg);
	else
		fprintf(stderr, "ft_nm: %s\n", msg);
	return (1);
}
