/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   section_names.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 22:41:11 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:41:11 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

/* Every section name must be inside the name table and NUL-terminated. */
static int	check_section_names(t_nmctx *ctx)
{
	t_section_meta	meta;
	size_t			i;

	i = 0;
	while (i < ctx->sht.count)
	{
		if (read_section_meta(ctx, i, &meta) != 0
			|| !safe_cstr_in_table(ctx->sht.names, ctx->sht.names_size,
				meta.name_index))
			return (nm_error(ctx->file->path, "invalid section name"));
		i++;
	}
	return (0);
}

/*
** e_shstrndx == SHN_UNDEF is valid ELF: the sections have no names.
** Otherwise it must be a string table inside the file.
*/
int	load_section_names(t_nmctx *ctx, size_t strndx)
{
	t_section_meta	meta;

	if (strndx == SHN_UNDEF)
		return (0);
	if (read_section_meta(ctx, strndx, &meta) != 0
		|| meta.type != SHT_STRTAB)
		return (nm_error(ctx->file->path, "invalid section name table"));
	ctx->sht.names = offptr(ctx->file, meta.offset, meta.size);
	if (!ctx->sht.names)
		return (nm_error(ctx->file->path, "section name table truncated"));
	ctx->sht.names_size = meta.size;
	return (check_section_names(ctx));
}

const char	*section_name(const t_nmctx *ctx, const t_section_meta *meta)
{
	const char	*name;

	name = safe_cstr_in_table(ctx->sht.names, ctx->sht.names_size,
			meta->name_index);
	if (!name)
		return ("");
	return (name);
}
