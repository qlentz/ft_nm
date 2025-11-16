#include <ft_nm.h>

// Already have this, keep using it everywhere:
const void *offptr(const t_file *f, size_t off, size_t len) {
    if (!f || !f->mem) return NULL;
    if (off > f->size) return NULL;
    if (len > f->size - off) return NULL;  // prevents overflow + OOB
    return (const unsigned char*)f->mem + off;
}

int mul_overflows_size_t(size_t a, size_t b, size_t *out) {
    if (!out)
        return 1;
    if (a == 0 || b == 0) {
        *out = 0;
        return 0;
    }
    if (a > SIZE_MAX / b)
        return 1;
    *out = a * b;
    return 0;
}
// Validate that a NUL-terminated string exists fully inside [base, base+sz).
// Returns NULL if invalid.
const char *safe_cstr_in_table(const char *base, size_t sz, size_t off) {
    if (!base || off >= sz) return NULL;
    const char *s = base + off;
    const char *end = base + sz;
    for (const char *p = s; p < end; ++p) {
        if (*p == '\0') return s;
    }
    return NULL; // no NUL before end => corrupted
}
