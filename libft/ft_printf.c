/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/10 18:18:12 by qlentz            #+#    #+#             */
/*   Updated: 2022/11/04 15:22:41 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_printarg(va_list args, char format)
{
	int	size;

	size = 0;
	if (format == 'd' || format == 'i')
		size += ft_printnbr(va_arg(args, int));
	else if (format == 's')
		size += ft_printstr(va_arg(args, char *));
	else if (format == 'c')
		size += ft_printchar(va_arg(args, int));
	else if (format == 'x')
		size += ft_printhex(va_arg(args, unsigned int));
	else if (format == 'X')
		size += ft_printhexcap(va_arg(args, unsigned int));
	else if (format == '%')
		size += ft_printchar('%');
	else if (format == 'u')
		size += ft_printunbr(va_arg(args, unsigned int));
	else if (format == 'p')
		size += ft_printptr(va_arg(args, unsigned long));
	return (size);
}

int	ft_printf(const char *str, ...)
{
	va_list	args;
	int		size;

	size = 0;
	va_start(args, str);
	while (*str)
	{
		if (*str == '%')
		{
			size += ft_printarg(args, *(str + 1));
			str++;
		}
		else
			size += ft_printchar(*str);
		str++;
	}
	va_end(args);
	return (size);
}
