#include "ft_nm.h"

int	nm_error(const char *path, const char *msg)
{
	ft_putstr_fd("ft_nm: ", STDERR_FILENO);
	if (path)
	{
		ft_putstr_fd(path, STDERR_FILENO);
		ft_putstr_fd(": ", STDERR_FILENO);
	}
	ft_putendl_fd(msg, STDERR_FILENO);
	return (1);
}
