/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   symbol_tables.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:08:43 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:08:43 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/*
** Symbols in sections numbered past 0xfeff store SHN_XINDEX; their real
** index is in the SHT_SYMTAB_SHNDX section linked to the symbol table.
*/
static void	load_shndx_table(t_nmctx *ctx, size_t symtab_index)
{
	t_section_meta	meta;
	size_t			i;

	i = 0;
	while (i < ctx->sht.count)
	{
		if (read_section_meta(ctx, i, &meta) == 0
			&& meta.type == SHT_SYMTAB_SHNDX && meta.link == symtab_index)
		{
			ctx->symtab.shndx_base = offptr(ctx->file, meta.offset, meta.size);
			if (ctx->symtab.shndx_base)
				ctx->symtab.shndx_count = meta.size / sizeof(uint32_t);
			return ;
		}
		i++;
	}
}

int	load_symbol_tables(t_nmctx *ctx)
{
	t_section_meta	meta;
	size_t			i;

	i = 0;
	while (i < ctx->sht.count)
	{
		if (read_section_meta(ctx, i, &meta) != 0)
			return (nm_error(ctx->file->path, "invalid section header"));
		if (meta.type == SHT_SYMTAB)
		{
			if (populate_symtab_slice(ctx, &meta, &ctx->symtab) != 0)
				return (1);
			load_shndx_table(ctx, i);
			ctx->symtab.present = 1;
			return (0);
		}
		i++;
	}
	return (0);
}
