/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printhex.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 18:18:19 by qlentz            #+#    #+#             */
/*   Updated: 2022/11/04 15:23:01 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_printhex(unsigned long nb)
{
	int			size;

	size = 0;
	if (nb > 15)
	{
		size += ft_printhex((nb / 16));
		size += ft_printhex((nb % 16));
	}
	else if (nb > 9)
	{
		size += ft_printchar('a' + nb - 10);
	}
	else
	{
		size += ft_printchar((nb + '0'));
	}
	return (size);
}
