/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printnbr.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 18:18:34 by qlentz            #+#    #+#             */
/*   Updated: 2022/11/04 15:23:25 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_printnbr(int n)
{
	long int	nb;
	int			size;

	nb = n;
	size = 0;
	if (nb < 0)
	{
		size += ft_printchar('-');
		nb *= -1;
	}
	if (nb > 9)
	{
		size += ft_printnbr((nb / 10));
		size += ft_printnbr((nb % 10));
	}
	else
	{
		size += ft_printchar((nb + '0'));
	}
	return (size);
}
