/* Text, data and bss symbols, global and local. */
int			g_data = 1;
static int	s_bss;
const int	g_rodata = 42;

int	f(void)
{
	return (s_bss + g_data);
}
