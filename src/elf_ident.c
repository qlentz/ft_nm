#include <ft_nm.h>

static int	check_elf_magic(const unsigned char *mem)
{
	static const unsigned char	elf_magic[4] = {ELFMAG0, ELFMAG1, ELFMAG2,
		ELFMAG3};

	return (ft_memcmp(mem, elf_magic, 4) == 0);
}

static int	parse_elf_class(const unsigned char *id, t_ElfMeta *meta)
{
	if (id[EI_CLASS] == ELFCLASS64)
		meta->is_64 = 1;
	else if (id[EI_CLASS] == ELFCLASS32)
		meta->is_64 = 0;
	else
		return (-1);
	return (0);
}

static int	parse_elf_endianness(const unsigned char *id, t_ElfMeta *meta)
{
	if (id[EI_DATA] == ELFDATA2LSB)
		meta->is_le = 1;
	else if (id[EI_DATA] == ELFDATA2MSB)
		meta->is_le = 0;
	else
		return (-1);
	return (0);
}

int	elf_ident(const t_file *f, t_ElfMeta *meta)
{
	const unsigned char	*id = (const unsigned char *)f->mem;

	if (f->size < EI_NIDENT || !check_elf_magic(id)
		|| parse_elf_class(id, meta) || parse_elf_endianness(id, meta))
	{
		nm_error(f->path, "file format not recognized");
		return (-1);
	}
	if (!meta->is_le)
	{
		nm_error(f->path, "big-endian ELF files are not supported");
		return (-1);
	}
	return (0);
}
