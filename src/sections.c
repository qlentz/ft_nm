/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sections.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/16 23:36:47 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 19:01:07 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static const unsigned char	*section_header_view(const t_nmctx *ctx,
		size_t index)
{
	size_t	offset;
	size_t	stride;

	if (!ctx->sht.base || index >= ctx->sht.count)
		return (NULL);
	stride = ctx->sht.entry_size;
	if (mul_overflows_size_t(index, stride, &offset))
		return (NULL);
	if (offset + stride > ctx->sht.size_bytes)
		return (NULL);
	return (ctx->sht.base + offset);
}

static void	fill_meta64(const t_elf64shdr *sh, int le, t_section_meta *meta)
{
	meta->name_index = r_u32(&sh->sh_name, le);
	meta->type = r_u32(&sh->sh_type, le);
	meta->offset = (size_t)r_u64(&sh->sh_offset, le);
	meta->size = (size_t)r_u64(&sh->sh_size, le);
	meta->link = r_u32(&sh->sh_link, le);
	meta->entsize = (size_t)r_u64(&sh->sh_entsize, le);
	meta->flags = r_u64(&sh->sh_flags, le);
}

static void	fill_meta32(const t_elf32shdr *sh, int le, t_section_meta *meta)
{
	meta->name_index = r_u32(&sh->sh_name, le);
	meta->type = r_u32(&sh->sh_type, le);
	meta->offset = (size_t)r_u32(&sh->sh_offset, le);
	meta->size = (size_t)r_u32(&sh->sh_size, le);
	meta->link = r_u32(&sh->sh_link, le);
	meta->entsize = (size_t)r_u32(&sh->sh_entsize, le);
	meta->flags = r_u32(&sh->sh_flags, le);
}

int	read_section_meta(const t_nmctx *ctx, size_t index, t_section_meta *meta)
{
	const unsigned char	*raw;

	raw = section_header_view(ctx, index);
	if (!raw)
		return (1);
	ft_bzero(meta, sizeof(*meta));
	if (ctx->meta.is_64)
		fill_meta64((const t_elf64shdr *)raw, ctx->meta.is_le, meta);
	else
		fill_meta32((const t_elf32shdr *)raw, ctx->meta.is_le, meta);
	return (0);
}
