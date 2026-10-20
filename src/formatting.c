/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   formatting.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/17 18:30:13 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 23:17:31 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/* Undefined symbols have no address: nm prints spaces instead. */
static void	print_symbol_row(const t_symbol *sym, int width)
{
	if (sym->is_undef)
		printf("%*s %c %s\n", width, "", sym->type_char, sym->name);
	else
		printf("%0*" PRIx64 " %c %s\n", width, sym->value, sym->type_char,
			sym->name);
}

/* Output goes through stdio: stdout is buffered, unlike raw write calls. */
void	print_symbols(const t_nmctx *ctx, t_symbol *symbols, size_t count)
{
	size_t	i;
	int		width;

	width = 8;
	if (ctx->meta.is_64)
		width = 16;
	i = 0;
	while (i < count)
	{
		print_symbol_row(&symbols[i], width);
		i++;
	}
}
