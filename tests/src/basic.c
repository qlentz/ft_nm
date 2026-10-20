/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:45:29 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 23:35:09 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Text, data, bss and read-only symbols, global and local. */
int			g_data = 1;
static int	g_local_bss;
const int	g_rodata = 42;

int	f(void)
{
	return (g_local_bss + g_data);
}
