#include <ft_nm.h>

int	check_elf_magic(const unsigned char *mem)
{
	static const unsigned char	elf_magic[4] = {ELFMAG0, ELFMAG1, ELFMAG2,
		ELFMAG3};

	if (ft_memcmp(mem, elf_magic, 4) != 0)
	{
		ft_printf("ft_nm: file is not an ELF file.\n");
		return (0);
	}
	return (1);
}

int	parse_elf_class(const unsigned char *id, t_ElfMeta *meta)
{
	if (id[EI_CLASS] == ELFCLASS64)
		meta->is_64 = 1;
	else if (id[EI_CLASS] == ELFCLASS32)
		meta->is_64 = 0;
	else
	{
		ft_putendl_fd("ft_nm: uknown elf class flag.", 2);
		return (-1);
	}
	return (0);
}

int	parse_elf_endianness(const unsigned char *id, t_ElfMeta *meta)
{
	if (id[EI_DATA] == ELFDATA2LSB)
		meta->is_le = 1;
	else if (id[EI_DATA] == ELFDATA2MSB)
	{
		meta->is_le = 0;
		ft_putendl_fd("ft_nm: Big endian files unsupported, not x86 ?", 2);
		return (-1);
	}
	else
	{
		ft_putendl_fd("ft_nm: uknown file endianness flag.", 2);
		return (-1);
	}
	return (0);
}

int	elf_ident(const t_file *f, t_ElfMeta *meta)
{
	const unsigned char	*id = (const unsigned char *)f->mem;

	if (f->size < EI_NIDENT || !check_elf_magic(id))
		return (-1);
	if (parse_elf_class(id, meta))
		return (-1);
	if (parse_elf_endianness(id, meta))
		return (-1);
	return (0);
}
