/* GNU indirect functions (nm type 'i'). */
static int	impl(void)
{
	return (1);
}

static void	*resolve(void)
{
	return (impl);
}

int			global_ifunc(void) __attribute__((ifunc("resolve")));
static int	local_ifunc(void) __attribute__((ifunc("resolve")));

int	use(void)
{
	return (local_ifunc());
}
