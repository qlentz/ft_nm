/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lib.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:45:29 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 23:35:09 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Shared library and executable test code. */
int			g_lib_data = 4;
static int	g_lib_local;

int	lib_func(int x)
{
	return (x + g_lib_data + g_lib_local);
}
