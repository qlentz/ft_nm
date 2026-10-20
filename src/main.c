#include "ft_nm.h"

static int	run_file(const char *filename, int show_name)
{
	t_file	file;
	int		ret;

	if (map_file(filename, &file) != 0)
		return (1);
	ret = process_elf(&file, show_name);
	free_file(&file);
	return (ret);
}

int	main(int ac, char **av)
{
	int	status;
	int	i;

	setlocale(LC_ALL, "");
	if (ac <= 1)
		return (run_file(DEFAULT_FILE, 0));
	status = 0;
	i = 1;
	while (i < ac)
	{
		if (run_file(av[i], ac > 2) != 0)
			status = 1;
		++i;
	}
	return (status);
}
