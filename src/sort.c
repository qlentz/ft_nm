/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 23:04:31 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/* Merges the sorted halves [0, mid) and [mid, count) through tmp. */
static void	merge(t_symbol *symbols, t_symbol *tmp, size_t mid, size_t count)
{
	size_t	i;
	size_t	j;
	size_t	k;

	i = 0;
	j = mid;
	k = 0;
	while (k < count)
	{
		if (j == count
			|| (i < mid && strcoll(symbols[i].name, symbols[j].name) <= 0))
			tmp[k++] = symbols[i++];
		else
			tmp[k++] = symbols[j++];
	}
	ft_memcpy(symbols, tmp, sizeof(*symbols) * count);
}

static void	merge_sort(t_symbol *symbols, t_symbol *tmp, size_t count)
{
	if (count < 2)
		return ;
	merge_sort(symbols, tmp, count / 2);
	merge_sort(symbols + count / 2, tmp, count - count / 2);
	merge(symbols, tmp, count / 2, count);
}

/*
** Merge sort on strcoll: the order follows LC_COLLATE like nm, and the
** sort is stable (`<= 0` takes the left one first), so equal names keep
** their symbol-table order. Returns 1 if the work buffer can't be allocated.
*/
int	sort_symbols(t_symbol *symbols, size_t count)
{
	t_symbol	*tmp;

	if (count < 2)
		return (0);
	tmp = malloc(sizeof(*tmp) * count);
	if (!tmp)
		return (1);
	merge_sort(symbols, tmp, count);
	free(tmp);
	return (0);
}
