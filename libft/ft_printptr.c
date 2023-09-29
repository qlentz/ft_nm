/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printptr.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 18:18:44 by qlentz            #+#    #+#             */
/*   Updated: 2022/11/04 15:23:37 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_printptr(unsigned long n)
{
	int		size;

	size = 0;
	if (n == 0)
	{
		size += ft_printstr("0x0");
		return (size);
	}
	else
	{
		size += ft_printstr("0x");
		size += ft_printhex(n);
	}
	return (size);
}
