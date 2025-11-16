#ifndef FT_NM_H
#define FT_NM_H

#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libft.h>
#include <stdio.h>

#define DEFAULT_FILE "a.out"

typedef struct {
    int is_64; //is 64 bit
    int is_le; //is little endian
}   t_ElfMeta;

typedef struct {
    int fd;
    void *mem;
    struct stat st;
    size_t size;
    int elf_class;
} t_file;

typedef struct s_sh_table {
    size_t offset;
    uint16_t entry_size;
    uint16_t count;
    uint16_t str_index;
    const unsigned char *base;
}   t_sh_table;

enum { EI_MAG0=0, EI_MAG1, EI_MAG2, EI_MAG3, EI_CLASS=4, EI_DATA=5, EI_NIDENT=16 };
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFCLASS32 1
#define ELFCLASS64 2
#define ELFDATA2LSB 1   // little-endian
#define ELFDATA2MSB 2   // big-endian
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11

typedef struct { // 32-bit ELF header (subset of fields we need)
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} t_elf32header;

typedef struct { // 64-bit ELF header (subset of fields we need)
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} t_elf64header;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} t_elf32shdr;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} t_elf64shdr;

typedef struct s_symtab_slice {
    const unsigned char *sym_base;
    size_t sym_size;
    size_t entry_size;
    size_t count;
    const char *str_base;
    size_t str_size;
    int present;
}   t_symtab_slice;

typedef struct s_nmctx {
    t_file *file;
    t_ElfMeta meta;
    const t_elf32header *hdr32;
    const t_elf64header *hdr64;
    t_sh_table sht;
    t_symtab_slice symtab;
    t_symtab_slice dynsym;
}   t_nmctx;

//peu d'impact car toujours little endian sur x86 pour ce projet
uint16_t r_u16(const void *p, int is_le);
uint32_t r_u32(const void *p, int is_le);
uint64_t r_u64(const void *p, int is_le);

int elf_ident(const t_file *f, t_ElfMeta *meta);

const void *offptr(const t_file *f, size_t off, size_t len);
int mul_overflows_size_t(size_t a, size_t b, size_t *out);
const char *safe_cstr_in_table(const char *base, size_t sz, size_t off);

#endif
