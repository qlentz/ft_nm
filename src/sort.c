/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/*
** Insertion sort on strcoll: the order follows LC_COLLATE like nm, and
** the sort is stable so equal names keep their symbol-table order.
*/
void	sort_symbols(t_symbol *symbols, size_t count)
{
	size_t		i;
	size_t		j;
	t_symbol	tmp;

	i = 1;
	while (symbols && i < count)
	{
		tmp = symbols[i];
		j = i;
		while (j > 0 && strcoll(tmp.name, symbols[j - 1].name) < 0)
		{
			symbols[j] = symbols[j - 1];
			j--;
		}
		symbols[j] = tmp;
		i++;
	}
}
