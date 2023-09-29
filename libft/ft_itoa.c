/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 12:03:32 by qlentz            #+#    #+#             */
/*   Updated: 2022/10/10 14:25:18 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_intsize(int n)
{
	int	size;

	size = 0;
	if (n == 0)
		return (1);
	if (n < 0)
		size++;
	while (n > 0 || n < 0)
	{
		n /= 10;
		size++;
	}
	return (size);
}

char	*ft_itoa(int n)
{
	char	*s;
	int		i;
	long	n2;

	n2 = n;
	i = ft_intsize(n2);
	s = (char *)malloc(sizeof(char) * (i + 1));
	if (!s)
		return (NULL);
	s[i--] = 0;
	if (n2 == 0)
		s[0] = '0';
	if (n2 < 0)
	{
		s[0] = '-';
		n2 *= -1;
	}
	while (n2 > 0)
	{
		s[i--] = (n2 % 10) + 48;
		n2 /= 10;
	}
	return (s);
}
