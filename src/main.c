#include "ft_nm.h"

void	free_file(t_file *file)
{
	if (file->mem)
		munmap(file->mem, file->size);
	if (file->fd > 0)
		close(file->fd);
}
int	open_file(char *filename, t_file *file)
{
	if (!filename)
		return 0;
	file->fd = open(filename, O_RDONLY);
	if (file < 0)
		return 0;
	if (fstat(file->fd, &file->st) < 0)
		return 0;
	file->size = file->st.st_size;
	file->mem = mmap(0, file->size, PROT_READ, MAP_PRIVATE, file->fd, 0);
	if (file->mem == MAP_FAILED)
		return 0;
	return 1;
}



void ft_nm(char *filename) {
	t_file file;

	file.mem = NULL;
	file.fd = -1;

	if (!open_file(filename, &file)) {
		//handle
	}


	
}

int main(int ac, char **av) {
	if (ac == 1) {
		ft_nm(DFLT_FILE);
	} else {
		while (--ac) {
			ft_nm(*++av);
		}
	}
	return 0;
}