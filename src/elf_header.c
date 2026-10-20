/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_header.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 19:01:07 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static int	init_header_view(const t_file *file, t_nmctx *ctx)
{
	const void	*view;

	view = offptr(file, 0, elf_header_size(&ctx->meta));
	if (!view)
		return (nm_error(file->path, "truncated ELF header"));
	if (ctx->meta.is_64)
		ctx->hdr64 = view;
	else
		ctx->hdr32 = view;
	return (0);
}

static int	init_nmctx(t_file *file, t_nmctx *ctx)
{
	ft_bzero(ctx, sizeof(*ctx));
	ctx->file = file;
	if (elf_ident(file, &ctx->meta) != 0)
		return (1);
	return (init_header_view(file, ctx));
}

int	process_elf(t_file *file, int show_name)
{
	t_nmctx	ctx;

	if (init_nmctx(file, &ctx) != 0)
		return (1);
	if (load_section_table(&ctx) != 0)
		return (1);
	if (load_symbol_tables(&ctx) != 0)
		return (1);
	return (render_symbols(&ctx, show_name));
}
