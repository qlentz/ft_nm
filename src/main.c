/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/29 14:22:50 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 22:12:02 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_nm.h"

static int	run_file(const char *filename, int show_name)
{
	t_file	file;
	int		ret;

	if (map_file(filename, &file) != 0)
		return (1);
	if (is_archive(&file))
		ret = process_archive(&file, show_name);
	else
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
