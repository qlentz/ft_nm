/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:12:02 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

void	print_file_header(const char *path)
{
	ft_putchar_fd('\n', STDOUT_FILENO);
	ft_putstr_fd(path, STDOUT_FILENO);
	ft_putendl_fd(":", STDOUT_FILENO);
}

/*
** Returns 1 if symbol `index` is listed and was stored in `dst`.
** Symbol 0 is the reserved null entry and is never listed. Like nm, a
** common symbol shows its size (st_size) instead of its alignment.
*/
static int	fill_symbol(const t_nmctx *ctx, size_t index, t_symbol *dst)
{
	t_symbol_view	view;
	const char		*name;

	if (read_symbol_view(ctx, index, &view) != 0)
		return (0);
	name = symbol_name(ctx, view.name);
	if (should_skip_symbol(&view))
		return (0);
	dst->name = name;
	dst->value = view.value;
	if (view.shndx == SHN_COMMON)
		dst->value = view.size;
	dst->type_char = classify_symbol(ctx, &view);
	dst->is_undef = (view.shndx == SHN_UNDEF);
	return (1);
}

static int	collect_symbols(const t_nmctx *ctx, t_symbol **out, size_t *count)
{
	size_t	i;

	*out = NULL;
	*count = 0;
	if (!ctx->symtab.sym_base || !ctx->symtab.count)
		return (0);
	*out = malloc(sizeof(**out) * ctx->symtab.count);
	if (!*out)
		return (1);
	i = 1;
	while (i < ctx->symtab.count)
	{
		*count += fill_symbol(ctx, i, *out + *count);
		i++;
	}
	return (0);
}

int	render_symbols(t_nmctx *ctx, int show_name)
{
	t_symbol	*symbols;
	size_t		count;

	if (show_name)
		print_file_header(ctx->file->path);
	if (!ctx->symtab.present || check_symbol_kinds(ctx) != 0)
	{
		nm_error(ctx->file->path, "no symbols");
		return (0);
	}
	if (collect_symbols(ctx, &symbols, &count) != 0)
		return (nm_error(ctx->file->path, strerror(ENOMEM)));
	sort_symbols(symbols, count);
	print_symbols(ctx, symbols, count);
	free(symbols);
	return (0);
}
