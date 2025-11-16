#include <ft_nm.h>

uint16_t r_u16(const void *p, int is_le) {
    const uint8_t *b = p;
    uint16_t v = (uint16_t)b[0] | ((uint16_t)b[1] << 8);
    if (is_le) return v;               // file is little-endian → already correct
    return (uint16_t)((v >> 8) | (v << 8)); // swap bytes for big-endian
}

uint32_t r_u32(const void *p, int is_le) {
    const uint8_t *b = p;
    uint32_t v = (uint32_t)b[0]
               | ((uint32_t)b[1] << 8)
               | ((uint32_t)b[2] << 16)
               | ((uint32_t)b[3] << 24);
    if (is_le) return v;
    return ((v & 0x000000FFU) << 24)
         | ((v & 0x0000FF00U) << 8)
         | ((v & 0x00FF0000U) >> 8)
         | ((v & 0xFF000000U) >> 24);
}

uint64_t r_u64(const void *p, int is_le) {
    const uint8_t *b = p;
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= ((uint64_t)b[i]) << (8 * i);
    if (is_le) return v;
    // reverse byte order
    v = ((v & 0x00000000000000FFULL) << 56) |
        ((v & 0x000000000000FF00ULL) << 40) |
        ((v & 0x0000000000FF0000ULL) << 24) |
        ((v & 0x00000000FF000000ULL) << 8)  |
        ((v & 0x000000FF00000000ULL) >> 8)  |
        ((v & 0x0000FF0000000000ULL) >> 24) |
        ((v & 0x00FF000000000000ULL) >> 40) |
        ((v & 0xFF00000000000000ULL) >> 56);
    return v;
}
