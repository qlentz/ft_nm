/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   symbol_check.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 21:19:19 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:12:02 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static void	put_size_fd(size_t n, int fd)
{
	if (n >= 10)
		put_size_fd(n / 10, fd);
	ft_putchar_fd('0' + n % 10, fd);
}

/* Prints "ft_nm: <file>: symbol number <index> <msg>[<value>]". */
static int	report_symbol(const t_nmctx *ctx, size_t index, const char *msg,
		long value)
{
	ft_putstr_fd("ft_nm: ", STDERR_FILENO);
	ft_putstr_fd(ctx->file->path, STDERR_FILENO);
	ft_putstr_fd(": symbol number ", STDERR_FILENO);
	put_size_fd(index, STDERR_FILENO);
	ft_putstr_fd(msg, STDERR_FILENO);
	if (value >= 0)
		put_size_fd((size_t)value, STDERR_FILENO);
	ft_putchar_fd('\n', STDERR_FILENO);
	return (1);
}

/*
** Like nm, rejects the whole table if any symbol (including the null
** entry 0) has a binding or type that ELF does not define, or needs an
** SHT_SYMTAB_SHNDX section that does not exist.
*/
int	check_symbol_kinds(const t_nmctx *ctx)
{
	t_symbol_view	view;
	unsigned char	bind;
	size_t			i;
	int				ret;

	i = 0;
	while (i < ctx->symtab.count)
	{
		ret = read_symbol_view(ctx, i, &view);
		if (ret == 2)
			return (report_symbol(ctx, i, " references nonexistent "
					"SHT_SYMTAB_SHNDX section", -1));
		if (ret != 0)
			return (1);
		bind = elf_st_bind(view.info);
		if (bind >= STB_NUM && bind < STB_LOOS)
			return (report_symbol(ctx, i, " uses unsupported binding of ",
					bind));
		if (elf_st_type(view.info) == STT_NUM)
			return (report_symbol(ctx, i, " uses unsupported type of ",
					STT_NUM));
		i++;
	}
	return (0);
}

/*
** Returns the symbol's name, or "<corrupt>" with a warning (like nm) if
** the offset is outside the string table or the string is not terminated.
*/
const char	*symbol_name(const t_nmctx *ctx, uint32_t offset)
{
	const char	*name;

	name = safe_cstr_in_table(ctx->symtab.str_base, ctx->symtab.str_size,
			offset);
	if (name)
		return (name);
	ft_putstr_fd("ft_nm: ", STDERR_FILENO);
	ft_putstr_fd(ctx->file->path, STDERR_FILENO);
	ft_putstr_fd(": invalid string offset ", STDERR_FILENO);
	put_size_fd(offset, STDERR_FILENO);
	ft_putstr_fd(" in the symbol string table\n", STDERR_FILENO);
	return ("<corrupt>");
}
