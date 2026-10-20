/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   formatting.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/17 18:30:13 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static void	print_padding(int width)
{
	while (width-- > 0)
		ft_putchar_fd(' ', STDOUT_FILENO);
}

static int	hex_digit_count(uint64_t value)
{
	int	count;

	count = 1;
	while (value >= 16)
	{
		value /= 16;
		count++;
	}
	return (count);
}

static void	print_hex_value(uint64_t value, int width)
{
	int	digits;

	digits = hex_digit_count(value);
	while (digits < width)
	{
		ft_putchar_fd('0', STDOUT_FILENO);
		digits++;
	}
	ft_printhex((unsigned long)value);
}

static void	print_symbol_row(const t_nmctx *ctx, const t_symbol *sym)
{
	int	width;

	width = 8;
	if (ctx->meta.is_64)
		width = 16;
	if (sym->is_undef)
		print_padding(width);
	else
		print_hex_value(sym->value, width);
	ft_putchar_fd(' ', STDOUT_FILENO);
	ft_putchar_fd(sym->type_char, STDOUT_FILENO);
	ft_putchar_fd(' ', STDOUT_FILENO);
	ft_putendl_fd(sym->name, STDOUT_FILENO);
}

void	print_symbols(const t_nmctx *ctx, t_symbol *symbols, size_t count)
{
	size_t	i;

	i = 0;
	while (i < count)
	{
		print_symbol_row(ctx, &symbols[i]);
		i++;
	}
}
