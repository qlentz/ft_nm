#include <ft_nm.h>

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

	if (ctx->meta.is_64)
		width = 16;
	else
		width = 8;
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

void	print_symbols(const t_nmctx *ctx, t_symbol *symbols, size_t count)
{
	size_t	i;

	i = 0;
	while (i < count)
	{
		print_symbol_row(ctx, &symbols[i]);
		++i;
	}
}
