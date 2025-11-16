#include "ft_nm.h"

static int	run_file(const char *filename)
{
	t_file	file;
	int		ret;

	if (!filename)
		return (1);
	if (map_file(filename, &file) != 0)
	{
		ft_putstr_fd("ft_nm: failed to map ", STDERR_FILENO);
		ft_putendl_fd(filename, STDERR_FILENO);
		return (1);
	}
	ret = process_elf(&file, filename);
	free_file(&file);
	return (ret);
}

int	ft_nm(char *filename)
{
	return (run_file(filename));
}

int	main(int ac, char **av)
{
	int	status;

	if (ac <= 1)
		return (ft_nm(DEFAULT_FILE));
	status = 0;
	while (--ac)
	{
		if (ft_nm(*++av) != 0)
			status = 1;
	}
	return (status);
}
