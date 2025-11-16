#include <stdlib.h>
#include "ft_nm.h"

typedef struct s_symbol_view
{
	uint32_t		name;
	uint64_t		value;
	uint64_t		size;
	unsigned char	info;
	unsigned char	other;
	uint16_t		shndx;
}	t_symbol_view;

static t_symtab_slice	*active_symtab(t_nmctx *ctx)
{
	if (ctx->symtab.present)
		return (&ctx->symtab);
	if (ctx->dynsym.present)
		return (&ctx->dynsym);
	return (NULL);
}

static void	fill_symbol64(const t_nmctx *ctx,
		const t_elf64sym *sym, t_symbol_view *view)
{
	view->name = r_u32(&sym->st_name, ctx->meta.is_le);
	view->info = sym->st_info;
	view->other = sym->st_other;
	view->shndx = r_u16(&sym->st_shndx, ctx->meta.is_le);
	view->value = r_u64(&sym->st_value, ctx->meta.is_le);
	view->size = r_u64(&sym->st_size, ctx->meta.is_le);
}

static void	fill_symbol32(const t_nmctx *ctx,
		const t_elf32sym *sym, t_symbol_view *view)
{
	view->name = r_u32(&sym->st_name, ctx->meta.is_le);
	view->info = sym->st_info;
	view->other = sym->st_other;
	view->shndx = r_u16(&sym->st_shndx, ctx->meta.is_le);
	view->value = r_u32(&sym->st_value, ctx->meta.is_le);
	view->size = r_u32(&sym->st_size, ctx->meta.is_le);
}

static int	read_symbol_view(const t_nmctx *ctx,
		const t_symtab_slice *slice, size_t index, t_symbol_view *view)
{
	size_t				offset;
	size_t				stride;
	const unsigned char	*raw;

	stride = slice->entry_size;
	if (mul_overflows_size_t(index, stride, &offset))
		return (1);
	if (offset + stride > slice->sym_size)
		return (1);
	raw = slice->sym_base + offset;
	if (ctx->meta.is_64)
		fill_symbol64(ctx, (const t_elf64sym *)raw, view);
	else
		fill_symbol32(ctx, (const t_elf32sym *)raw, view);
	return (0);
}

static char	lower_if_local(char c, unsigned char info)
{
	if (c >= 'A' && c <= 'Z' && ELF_ST_BIND(info) == STB_LOCAL)
		return (c + 32);
	return (c);
}

static char	weak_letter(unsigned char type, const t_symbol_view *sym)
{
	if (type == STT_OBJECT)
		return (sym->shndx == SHN_UNDEF ? 'v' : 'V');
	return (sym->shndx == SHN_UNDEF ? 'w' : 'W');
}

static char	handle_bindings(const t_symbol_view *sym,
		unsigned char bind, unsigned char type)
{
	if (bind == STB_GNU_UNIQUE)
		return ('u');
	if (bind == STB_WEAK)
		return (weak_letter(type, sym));
	return (0);
}

static char	section_letter(const t_nmctx *ctx, const t_symbol_view *sym)
{
	t_section_meta	sect;

	if (sym->shndx >= ctx->sht.count)
		return ('?');
	if (read_section_meta(ctx, sym->shndx, &sect) != 0)
		return ('?');
	if (sect.type == SHT_NOBITS && (sect.flags & SHF_ALLOC)
		&& (sect.flags & SHF_WRITE))
		return ('B');
	if ((sect.flags & SHF_ALLOC) && (sect.flags & SHF_EXECINSTR))
		return ('T');
	if ((sect.flags & SHF_ALLOC) && (sect.flags & SHF_WRITE))
		return ('D');
	if (sect.flags & SHF_ALLOC)
		return ('R');
	return ('N');
}

static char	classify_symbol(const t_nmctx *ctx, const t_symbol_view *sym)
{
	unsigned char	bind;
	unsigned char	type;
	char			c;

	bind = ELF_ST_BIND(sym->info);
	type = ELF_ST_TYPE(sym->info);
	c = handle_bindings(sym, bind, type);
	if (c)
		return (lower_if_local(c, sym->info));
	if (sym->shndx == SHN_UNDEF)
		return (lower_if_local('U', sym->info));
	if (sym->shndx == SHN_ABS)
		return (lower_if_local('A', sym->info));
	if (sym->shndx == SHN_COMMON)
		return (lower_if_local('C', sym->info));
	return (lower_if_local(section_letter(ctx, sym), sym->info));
}

static int	should_skip_symbol(const t_symbol_view *view, const char *name)
{
	unsigned char	type;

	if (!name || !name[0])
		return (1);
	type = ELF_ST_TYPE(view->info);
	if (type == STT_FILE || type == STT_SECTION)
		return (1);
	return (0);
}

static void	save_symbol(t_symbol *dst, size_t *index,
		const char *name, const t_symbol_view *view, char type_char)
{
	dst[*index].name = name;
	dst[*index].value = view->value;
	dst[*index].type_char = type_char;
	(*index)++;
}

static int	compare_symbols(const t_symbol *a, const t_symbol *b)
{
	int	cmp;

	cmp = ft_strcmp(a->name, b->name);
	if (cmp != 0)
		return (cmp);
	if (a->value < b->value)
		return (-1);
	if (a->value > b->value)
		return (1);
	if (a->type_char < b->type_char)
		return (-1);
	if (a->type_char > b->type_char)
		return (1);
	return (0);
}

static void	sort_symbols(t_symbol *symbols, size_t count)
{
	size_t	i;

	i = 1;
	while (symbols && i < count)
	{
		size_t		j;
		t_symbol	tmp;

		tmp = symbols[i];
		j = i;
		while (j > 0 && compare_symbols(&tmp, &symbols[j - 1]) < 0)
		{
			symbols[j] = symbols[j - 1];
			--j;
		}
		symbols[j] = tmp;
		++i;
	}
}

static int	append_symbol(t_nmctx *ctx, const t_symtab_slice *slice,
		size_t index, t_symbol *dst, size_t *written)
{
	t_symbol_view	view;
	const char		*name;

	if (read_symbol_view(ctx, slice, index, &view) != 0)
		return (0);
	name = safe_cstr_in_table(slice->str_base, slice->str_size, view.name);
	if (!name || should_skip_symbol(&view, name))
		return (0);
	save_symbol(dst, written, name, &view, classify_symbol(ctx, &view));
	return (0);
}

static void	populate_symbol_array(t_nmctx *ctx, const t_symtab_slice *slice,
		t_symbol *buf, size_t *written)
{
	size_t	i;

	i = 0;
	while (i < slice->count)
	{
		append_symbol(ctx, slice, i, buf, written);
		++i;
	}
}

static int	collect_symbols(t_nmctx *ctx, const t_symtab_slice *slice,
		t_symbol **out_syms, size_t *out_count)
{
	size_t		written;
	t_symbol	*buf;

	*out_syms = NULL;
	*out_count = 0;
	if (!slice->present || !slice->sym_base)
		return (0);
	buf = malloc(sizeof(*buf) * slice->count);
	if (!buf)
		return (1);
	written = 0;
	populate_symbol_array(ctx, slice, buf, &written);
	if (!written)
	{
		free(buf);
		return (0);
	}
	*out_syms = buf;
	*out_count = written;
	return (0);
}

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
	if (digits < width)
	{
		while (digits < width)
		{
			ft_putchar_fd('0', STDOUT_FILENO);
			digits++;
		}
	}
	ft_printhex((unsigned long)value);
}

static void	print_symbol_row(const t_nmctx *ctx, const t_symbol *sym)
{
	int	width;
	int	is_undef;

	width = ctx->meta.is_64 ? 16 : 8;
	is_undef = (sym->type_char == 'U' || sym->type_char == 'u'
			|| sym->type_char == 'w' || sym->type_char == 'v');
	if (is_undef)
		print_padding(width);
	else
		print_hex_value(sym->value, width);
	ft_putchar_fd(' ', STDOUT_FILENO);
	ft_putchar_fd(sym->type_char, STDOUT_FILENO);
	ft_putchar_fd(' ', STDOUT_FILENO);
	ft_putendl_fd(sym->name, STDOUT_FILENO);
}

static void	print_symbols(const t_nmctx *ctx, t_symbol *symbols, size_t count)
{
	size_t	i;

	i = 0;
	while (i < count)
	{
		print_symbol_row(ctx, &symbols[i]);
		++i;
	}
}

int	render_symbols(t_nmctx *ctx, const char *filename)
{
	t_symtab_slice	*slice;
	t_symbol		*symbols;
	size_t			count;

	slice = active_symtab(ctx);
	if (!slice)
	{
		ft_putstr_fd(filename, STDOUT_FILENO);
		ft_putendl_fd(": no symbols", STDOUT_FILENO);
		return (0);
	}
	if (collect_symbols(ctx, slice, &symbols, &count) != 0)
		return (1);
	if (!symbols || !count)
	{
		ft_putstr_fd(filename, STDOUT_FILENO);
		ft_putendl_fd(": no symbols", STDOUT_FILENO);
		return (0);
	}
	sort_symbols(symbols, count);
	print_symbols(ctx, symbols, count);
	free(symbols);
	return (0);
}
