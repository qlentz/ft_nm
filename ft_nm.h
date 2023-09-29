#ifndef FT_NM_H
#define FT_NM_H

#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define EI_NIDENT 16
#define DFLT_FILE "a.out"

typedef struct {
    int fd;
    void *mem;
    struct stat st;
    size_t size;
} t_file;

// Simplified ELF Header for 32-bit files
typedef struct {
    unsigned char e_ident[16];  // ELF identification (only need up to EI_CLASS to determine 32/64-bit)
    uint32_t e_shoff;           // Section header table offset
    uint16_t e_shnum;           // Number of section header entries
} Elf32Header;

// Simplified ELF Header for 64-bit files
typedef struct {
    unsigned char e_ident[16];  // ELF identification (only need up to EI_CLASS to determine 32/64-bit)
    uint64_t e_shoff;           // Section header table offset
    uint16_t e_shnum;           // Number of section header entries
} Elf64Header;

// Simplified Section Header for 32-bit files
typedef struct {
    uint32_t sh_name;  // Section name (index into string table)
    uint32_t sh_type;  // Section type
    uint32_t sh_offset;// Section offset
    uint32_t sh_size;  // Section size
} Elf32SectionHeader;

// Simplified Section Header for 64-bit files
typedef struct {
    uint32_t sh_name;  // Section name (index into string table)
    uint32_t sh_type;  // Section type
    uint64_t sh_offset;// Section offset
    uint64_t sh_size;  // Section size
} Elf64SectionHeader;

// Simplified Symbol Table Entry for 32-bit files
typedef struct {
    uint32_t st_name;  // Symbol name (index into string table)
    uint32_t st_value; // Symbol value (address)
    uint8_t  st_info;  // Symbol type and binding
} Elf32Symbol;

// Simplified Symbol Table Entry for 64-bit files
typedef struct {
    uint32_t st_name;  // Symbol name (index into string table)
    uint8_t  st_info;  // Symbol type and binding
    uint64_t st_value; // Symbol value (address)
} Elf64Symbol;

#endif

