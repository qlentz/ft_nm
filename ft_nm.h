/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_nm.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qlentz <qlentz@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/29 14:22:50 by qlentz            #+#    #+#             */
/*   Updated: 2026/10/20 21:21:59 by qlentz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_NM_H
# define FT_NM_H

# include <stdint.h>
# include <stdlib.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <unistd.h>
# include <errno.h>
# include <locale.h>
# include <string.h>
# include "libft.h"

# define DEFAULT_FILE "a.out"

/* e_ident */
# define EI_CLASS 4
# define EI_DATA 5
# define EI_VERSION 6
# define EI_NIDENT 16
# define ELFMAG0 0x7f
# define ELFMAG1 'E'
# define ELFMAG2 'L'
# define ELFMAG3 'F'
# define ELFCLASS32 1
# define ELFCLASS64 2
# define ELFDATA2LSB 1
# define ELFDATA2MSB 2
# define EV_CURRENT 1

/* section types, special indexes and flags */
# define SHT_SYMTAB 2
# define SHT_STRTAB 3
# define SHT_NOBITS 8
# define SHN_UNDEF 0
# define SHN_ABS 0xfff1
# define SHN_COMMON 0xfff2
# define SHF_WRITE 0x1
# define SHF_ALLOC 0x2
# define SHF_EXECINSTR 0x4

/* symbol bindings and types */
# define STB_LOCAL 0
# define STB_GLOBAL 1
# define STB_WEAK 2
# define STB_NUM 3
# define STB_LOOS 10
# define STB_GNU_UNIQUE 10
# define STT_OBJECT 1
# define STT_SECTION 3
# define STT_FILE 4
# define STT_NUM 7
# define STT_GNU_IFUNC 10

typedef struct s_elfmeta
{
	int	is_64;
	int	is_le;
}	t_elfmeta;

typedef struct s_file
{
	const char	*path;
	int			fd;
	void		*mem;
	struct stat	st;
	size_t		size;
}	t_file;

typedef struct s_sht_fields
{
	size_t		offset;
	uint16_t	entsize;
	uint16_t	count;
	uint16_t	strndx;
}	t_sht_fields;

typedef struct s_sh_table
{
	size_t				size_bytes;
	uint16_t			entry_size;
	uint16_t			count;
	const unsigned char	*base;
}	t_sh_table;

typedef struct s_section_meta
{
	uint32_t	type;
	uint32_t	name_index;
	size_t		offset;
	size_t		size;
	uint32_t	link;
	size_t		entsize;
	uint64_t	flags;
}	t_section_meta;

typedef struct s_elf32header
{
	unsigned char	e_ident[EI_NIDENT];
	uint16_t		e_type;
	uint16_t		e_machine;
	uint32_t		e_version;
	uint32_t		e_entry;
	uint32_t		e_phoff;
	uint32_t		e_shoff;
	uint32_t		e_flags;
	uint16_t		e_ehsize;
	uint16_t		e_phentsize;
	uint16_t		e_phnum;
	uint16_t		e_shentsize;
	uint16_t		e_shnum;
	uint16_t		e_shstrndx;
}	t_elf32header;

typedef struct s_elf64header
{
	unsigned char	e_ident[EI_NIDENT];
	uint16_t		e_type;
	uint16_t		e_machine;
	uint32_t		e_version;
	uint64_t		e_entry;
	uint64_t		e_phoff;
	uint64_t		e_shoff;
	uint32_t		e_flags;
	uint16_t		e_ehsize;
	uint16_t		e_phentsize;
	uint16_t		e_phnum;
	uint16_t		e_shentsize;
	uint16_t		e_shnum;
	uint16_t		e_shstrndx;
}	t_elf64header;

typedef struct s_elf32shdr
{
	uint32_t	sh_name;
	uint32_t	sh_type;
	uint32_t	sh_flags;
	uint32_t	sh_addr;
	uint32_t	sh_offset;
	uint32_t	sh_size;
	uint32_t	sh_link;
	uint32_t	sh_info;
	uint32_t	sh_addralign;
	uint32_t	sh_entsize;
}	t_elf32shdr;

typedef struct s_elf64shdr
{
	uint32_t	sh_name;
	uint32_t	sh_type;
	uint64_t	sh_flags;
	uint64_t	sh_addr;
	uint64_t	sh_offset;
	uint64_t	sh_size;
	uint32_t	sh_link;
	uint32_t	sh_info;
	uint64_t	sh_addralign;
	uint64_t	sh_entsize;
}	t_elf64shdr;

typedef struct s_elf32sym
{
	uint32_t		st_name;
	uint32_t		st_value;
	uint32_t		st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	uint16_t		st_shndx;
}	t_elf32sym;

typedef struct s_elf64sym
{
	uint32_t		st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	uint16_t		st_shndx;
	uint64_t		st_value;
	uint64_t		st_size;
}	t_elf64sym;

typedef struct s_symtab_slice
{
	const unsigned char	*sym_base;
	size_t				sym_size;
	size_t				entry_size;
	size_t				count;
	const char			*str_base;
	size_t				str_size;
	int					present;
}	t_symtab_slice;

typedef struct s_symbol
{
	const char	*name;
	uint64_t	value;
	char		type_char;
	int			is_undef;
}	t_symbol;

typedef struct s_nmctx
{
	t_file				*file;
	t_elfmeta			meta;
	const t_elf32header	*hdr32;
	const t_elf64header	*hdr64;
	t_sh_table			sht;
	t_symtab_slice		symtab;
}	t_nmctx;

typedef struct s_symbol_view
{
	uint32_t		name;
	uint64_t		value;
	unsigned char	info;
	uint16_t		shndx;
}	t_symbol_view;

/* file.c */
int				map_file(const char *path, t_file *file);
void			free_file(t_file *file);

/* errors.c */
int				nm_error(const char *path, const char *msg);

/* readers.c */
uint16_t		r_u16(const void *p, int is_le);
uint32_t		r_u32(const void *p, int is_le);
uint64_t		r_u64(const void *p, int is_le);

/* safety_utils.c */
const void		*offptr(const t_file *f, size_t off, size_t len);
int				mul_overflows_size_t(size_t a, size_t b, size_t *out);
const char		*safe_cstr_in_table(const char *base, size_t sz, size_t off);

/* elf_ident.c */
int				elf_ident(const t_file *f, t_elfmeta *meta);

/* elf_utils.c */
size_t			elf_header_size(const t_elfmeta *meta);
size_t			elf_shdr_size(const t_elfmeta *meta);
size_t			elf_sym_size(const t_elfmeta *meta);
unsigned char	elf_st_bind(unsigned char info);
unsigned char	elf_st_type(unsigned char info);

/* elf_header.c */
int				process_elf(t_file *file, int show_name);

/* section_table.c */
int				load_section_table(t_nmctx *ctx);
int				load_symbol_tables(t_nmctx *ctx);

/* sections.c */
int				read_section_meta(const t_nmctx *ctx, size_t index,
					t_section_meta *meta);

/* symtab.c */
int				populate_symtab_slice(t_nmctx *ctx, const t_section_meta *meta,
					t_symtab_slice *dst);

/* symbol_read.c */
int				read_symbol_view(const t_nmctx *ctx, size_t index,
					t_symbol_view *view);
int				should_skip_symbol(const t_symbol_view *view, const char *name);

/* symbol_check.c */
int				check_symbol_kinds(const t_nmctx *ctx);
const char		*symbol_name(const t_nmctx *ctx, uint32_t offset);

/* symbol_type.c */
char			classify_symbol(const t_nmctx *ctx, const t_symbol_view *sym);

/* sort.c */
void			sort_symbols(t_symbol *symbols, size_t count);

/* render.c */
int				render_symbols(t_nmctx *ctx, int show_name);

/* formatting.c */
void			print_symbols(const t_nmctx *ctx, t_symbol *symbols,
					size_t count);

#endif
