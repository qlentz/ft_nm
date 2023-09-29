/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printunbr.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 18:19:03 by qlentz            #+#    #+#             */
/*   Updated: 2022/11/04 15:23:59 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_printunbr(unsigned int nb)
{
	int		size;

	size = 0;
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
