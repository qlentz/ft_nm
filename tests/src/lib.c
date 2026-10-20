/* Shared library and executable test code. */
int			lib_data = 4;
static int	lib_local;

int	lib_func(int x)
{
	return (x + lib_data + lib_local);
}
