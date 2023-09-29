/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atod.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 13:23:38 by qlentz            #+#    #+#             */
/*   Updated: 2022/11/21 13:28:39 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static double	pre_point(const char *str)
{
	double	d;

	d = 0.0;
	while (ft_isspace(*str))
		str++;
	if (*str == '-')
		str++;
	while (*str && *str <= '9' && *str >= '0')
	{
		d *= 10;
		d += *str - '0';
		str++;
	}
	return (d);
}

static double	after_point(const char *str)
{
	double	d2;

	d2 = 0.0;
	while (ft_isspace(*str))
		str++;
	if (*str == '-')
		str++;
	while (*str && *str <= '9' && *str >= '0')
		str++;
	if (*str == '.')
		str++;
	while (*str && *str <= '9' && *str >= '0')
		str++;
	str--;
	while (*str && *str <= '9' && *str >= '0')
	{
		d2 += *str - '0';
		d2 /= 10;
		str--;
	}
	return (d2);
}

double	ft_atod(const char *str)
{
	double	d;
	double	d2;
	int		sign;

	sign = 1;
	d = pre_point(str);
	d2 = after_point(str);
	while (ft_isspace(*str))
		str++;
	if (*str == '-')
	{
		str++;
		sign *= -1;
	}
	return (sign * (d + d2));
}
