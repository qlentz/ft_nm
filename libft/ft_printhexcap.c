/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printhexcap.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 18:18:28 by qlentz            #+#    #+#             */
/*   Updated: 2022/11/04 15:23:13 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_printhexcap(unsigned long nb)
{
	int			size;

	size = 0;
	if (nb > 15)
	{
		size += ft_printhexcap((nb / 16));
		size += ft_printhexcap((nb % 16));
	}
	else if (nb > 9)
	{
		size += ft_printchar('A' + nb - 10);
	}
	else
	{
		size += ft_printchar((nb + '0'));
	}
	return (size);
}
