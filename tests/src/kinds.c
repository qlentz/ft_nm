/* Weak, undefined, TLS, local and constant symbols. */
extern int			ext_var;
extern int			ext_func(void);
__attribute__((weak)) extern int	weak_undef_var;
__attribute__((weak)) extern int	weak_undef_func(void);
__attribute__((weak)) int		weak_def_var = 3;
__thread int		tls_data = 1;
__thread int		tls_bss;
static const char	local_ro[] = "ro";
static int			local_data = 5;

__attribute__((weak)) int	weak_def_func(void)
{
	return (1);
}

static int	local_func(void)
{
	return (local_data + local_ro[0]);
}

int	use_all(void)
{
	return (ext_var + ext_func() + local_func() + tls_data + tls_bss
		+ (&weak_undef_var != 0) + (weak_undef_func != 0) + weak_def_var);
}
