#!/usr/bin/env python3
"""Build every test input for tests/run_tests.sh.

Usage: gen_inputs.py <build_dir>

Creates:
  <build_dir>/valid/      files ft_nm must list exactly like nm
  <build_dir>/malformed/  corrupt or hostile files: ft_nm must not crash,
                          hang or leak, and must explain every failure
  <build_dir>/bench/      large files for the timing benchmarks
"""
import os
import shutil
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "src")

# ---------------------------------------------------------------------------
# ELF layout (offsets of the fields we patch)
# ---------------------------------------------------------------------------
EHDR = {
    64: {"type": (16, "H"), "machine": (18, "H"), "version": (20, "I"),
         "shoff": (40, "Q"), "ehsize": (52, "H"), "shentsize": (58, "H"),
         "shnum": (60, "H"), "shstrndx": (62, "H")},
    32: {"type": (16, "H"), "machine": (18, "H"), "version": (20, "I"),
         "shoff": (32, "I"), "ehsize": (40, "H"), "shentsize": (46, "H"),
         "shnum": (48, "H"), "shstrndx": (50, "H")},
}
SHDR = {
    64: {"name": (0, "I"), "type": (4, "I"), "flags": (8, "Q"),
         "offset": (24, "Q"), "size": (32, "Q"), "link": (40, "I"),
         "info": (44, "I"), "entsize": (56, "Q")},
    32: {"name": (0, "I"), "type": (4, "I"), "flags": (8, "I"),
         "offset": (16, "I"), "size": (20, "I"), "link": (24, "I"),
         "info": (28, "I"), "entsize": (36, "I")},
}
SYM = {
    64: {"name": (0, "I"), "info": (4, "B"), "shndx": (6, "H"),
         "value": (8, "Q"), "size": (16, "Q")},
    32: {"name": (0, "I"), "value": (4, "I"), "size": (8, "I"),
         "info": (12, "B"), "shndx": (14, "H")},
}
SHDR_SIZE = {64: 64, 32: 40}
SYM_SIZE = {64: 24, 32: 16}
SHT_PROGBITS, SHT_SYMTAB, SHT_STRTAB, SHT_NOBITS = 1, 2, 3, 8
SHF_WRITE, SHF_ALLOC, SHF_EXEC = 1, 2, 4


class Elf:
    """A mutable ELF image with helpers to read and patch its fields."""

    def __init__(self, data):
        self.d = bytearray(data)
        self.cls = 64 if self.d[4] == 2 else 32
        self.e = "<" if self.d[5] == 1 else ">"

    def _get(self, base, spec):
        off, fmt = spec
        return struct.unpack_from(self.e + fmt, self.d, base + off)[0]

    def _set(self, base, spec, value):
        off, fmt = spec
        mask = (1 << (8 * struct.calcsize(fmt))) - 1
        struct.pack_into(self.e + fmt, self.d, base + off, value & mask)

    def eh(self, field, value=None):
        if value is None:
            return self._get(0, EHDR[self.cls][field])
        self._set(0, EHDR[self.cls][field], value)
        return self

    def sh_base(self, index):
        return self.eh("shoff") + index * SHDR_SIZE[self.cls]

    def sh(self, index, field, value=None):
        if value is None:
            return self._get(self.sh_base(index), SHDR[self.cls][field])
        self._set(self.sh_base(index), SHDR[self.cls][field], value)
        return self

    def find(self, sh_type):
        for i in range(self.eh("shnum")):
            if self.sh(i, "type") == sh_type:
                return i
        raise KeyError(sh_type)

    def sym(self, index, field, value=None):
        tab = self.find(SHT_SYMTAB)
        base = self.sh(tab, "offset") + index * SYM_SIZE[self.cls]
        if value is None:
            return self._get(base, SYM[self.cls][field])
        self._set(base, SYM[self.cls][field], value)
        return self

    def nsyms(self):
        tab = self.find(SHT_SYMTAB)
        return self.sh(tab, "size") // SYM_SIZE[self.cls]

    def bytes(self):
        return bytes(self.d)


def make_obj(cls=64, endian="<", syms=None, etype=1, machine=None,
             extra=()):
    """Build a small, valid relocatable object.

    syms: (name, info, section_name_or_shndx, value, size) tuples.
    extra: (name, type, flags, data) sections added before .symtab.
    """
    if machine is None:
        machine = 62 if cls == 64 else 3
    if syms is None:
        syms = [("func", 0x12, ".text", 0x10, 4), ("data", 0x11, ".data", 4, 4),
                ("local", 0x01, ".bss", 8, 4), ("ext", 0x10, 0, 0, 0)]
    secs = [("", 0, 0, b""),
            (".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXEC, b"\x90" * 32),
            (".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE, b"\x01" * 16),
            (".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE, b"\x00" * 16),
            (".rodata", SHT_PROGBITS, SHF_ALLOC, b"\x02" * 8)]
    secs += list(extra)
    index = {s[0]: i for i, s in enumerate(secs)}
    symtab_i, strtab_i, shstr_i = len(secs), len(secs) + 1, len(secs) + 2
    strtab = b"\0"
    symfmt = "IBBHQQ" if cls == 64 else "IIIBBH"
    sym_bytes = b"\0" * SYM_SIZE[cls]
    for name, info, sec, value, size in syms:
        shndx = index[sec] if isinstance(sec, str) else sec
        noff = len(strtab)
        strtab += name.encode() + b"\0"
        if cls == 64:
            sym_bytes += struct.pack(endian + symfmt, noff, info, 0, shndx,
                                     value, size)
        else:
            sym_bytes += struct.pack(endian + symfmt, noff, value, size, info,
                                     0, shndx)
    names = [s[0] for s in secs] + [".symtab", ".strtab", ".shstrtab"]
    shstr = b"\0"
    name_off = {}
    for n in names:
        if n and n not in name_off:
            name_off[n] = len(shstr)
            shstr += n.encode() + b"\0"
    blobs = [s[3] for s in secs] + [sym_bytes, strtab, shstr]
    types = [s[1] for s in secs] + [SHT_SYMTAB, SHT_STRTAB, SHT_STRTAB]
    flags = [s[2] for s in secs] + [0, 0, 0]
    ehsize = 64 if cls == 64 else 52
    out = bytearray(ehsize)
    offs = []
    for i, blob in enumerate(blobs):
        offs.append(len(out))
        if types[i] != SHT_NOBITS:
            out += blob
    while len(out) % 8:
        out += b"\0"
    shoff = len(out)
    for i, n in enumerate(names):
        link = strtab_i if i == symtab_i else 0
        info = 1 if i == symtab_i else 0
        ent = SYM_SIZE[cls] if i == symtab_i else 0
        size = len(blobs[i])
        fields = (name_off.get(n, 0), types[i], flags[i], 0, offs[i] if i else 0,
                  size if i else 0, link, info, 1 if i else 0, ent)
        if cls == 64:
            out += struct.pack(endian + "IIQQQQIIQQ", *fields)
        else:
            out += struct.pack(endian + "IIIIIIIIII", *fields)
    ident = b"\x7fELF" + bytes([2 if cls == 64 else 1,
                                1 if endian == "<" else 2, 1, 0]) + b"\0" * 8
    if cls == 64:
        hdr = struct.pack(endian + "HHIQQQIHHHHHH", etype, machine, 1, 0, 0,
                          shoff, 0, 64, 0, 0, 64, len(names), shstr_i)
    else:
        hdr = struct.pack(endian + "HHIIIIIHHHHHH", etype, machine, 1, 0, 0,
                          shoff, 0, 52, 0, 0, 40, len(names), shstr_i)
    out[0:ehsize] = ident + hdr
    return bytes(out)


def ar_member(name, data, size_field=None, fmag=b"`\n"):
    size = str(len(data)).encode() if size_field is None else size_field
    hdr = (name.ljust(16).encode()[:16] + b"0".ljust(12) + b"0".ljust(6)
           + b"0".ljust(6) + b"644".ljust(8) + size.ljust(10)[:10] + fmag)
    body = hdr + data
    if len(data) % 2:
        body += b"\n"
    return body


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------
def run(cmd, cwd=None, quiet=False):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if r.returncode != 0 and not quiet:
        print("  skipped (%s): %s" % (" ".join(cmd), r.stderr.strip().splitlines()[-1][:120] if r.stderr.strip() else "failed"),
              file=sys.stderr)
    return r.returncode == 0


def write(path, data):
    with open(path, "wb") as f:
        f.write(data)


def gen_valid(out):
    """Real files built by the toolchain, plus valid hand-made ELF files."""
    cc = shutil.which("gcc") or shutil.which("cc")
    cxx = shutil.which("g++")
    builds = [
        ("basic.o", [cc, "-c", "basic.c"]),
        ("sort_names.o", ["as", "sort_names.s"]),
        ("sort_names32.o", ["as", "--32", "sort_names.s"]),
        ("tls.o", [cc, "-c", "tls.c"]),
        ("tls32.o", [cc, "-m32", "-c", "tls.c"]),
        ("kinds.o", [cc, "-c", "-fPIC", "kinds.c"]),
        ("ifunc.o", [cc, "-c", "ifunc.c"]),
        ("common.o", [cc, "-c", "-fcommon", "common.c"]),
        ("lib.o", [cc, "-c", "-fPIC", "lib.c"]),
        ("libtest.so", [cc, "-shared", "-fPIC", "lib.c"]),
        ("exe", [cc, "main.c", "lib.c"]),
        ("exe_pie", [cc, "-pie", "-fPIE", "main.c", "lib.c"]),
        ("exe_static", [cc, "-static", "main.c", "lib.c"]),
        ("basic_debug.o", [cc, "-c", "-g", "basic.c"]),
        ("sections.o", ["as", "sections.s"]),
        ("basic32.o", [cc, "-m32", "-c", "basic.c"]),
        ("kinds32.o", [cc, "-m32", "-c", "-fPIC", "kinds.c"]),
        ("ifunc32.o", [cc, "-m32", "-c", "ifunc.c"]),
        ("common32.o", [cc, "-m32", "-c", "-fcommon", "common.c"]),
        ("libtest32.so", [cc, "-m32", "-shared", "-fPIC", "lib.c"]),
        ("exe32", [cc, "-m32", "main.c", "lib.c"]),
        ("sections32.o", ["as", "--32", "sections.s"]),
    ]
    if cxx:
        builds += [("unique.o", [cxx, "-c", "unique.cpp"]),
                   ("unique.so", [cxx, "-shared", "-fPIC", "unique.cpp"]),
                   ("unique32.o", [cxx, "-m32", "-c", "unique.cpp"])]
    for name, cmd in builds:
        run(cmd + ["-o", os.path.join(out, name)], cwd=SRC)
    if os.path.exists(os.path.join(out, "libtest.so")):
        run(["strip", "-o", os.path.join(out, "stripped.so"),
             os.path.join(out, "libtest.so")])
    if os.path.exists(os.path.join(out, "exe")):
        run(["strip", "-o", os.path.join(out, "stripped_exe"),
             os.path.join(out, "exe")])
    # archives
    objs = [o for o in ("basic.o", "sort_names.o", "kinds.o", "common.o")
            if os.path.exists(os.path.join(out, o))]
    tmp = os.path.join(out, "a_really_long_member_name_for_the_gnu_table.o")
    shutil.copy(os.path.join(out, "basic.o"), tmp)
    run(["ar", "rcs", "lib.a"] + objs, cwd=out)
    run(["ar", "rcs", "long_names.a", "basic.o", os.path.basename(tmp)], cwd=out)
    run(["ar", "rcS", "no_index.a", "basic.o", "kinds.o"], cwd=out)
    os.remove(tmp)
    write(os.path.join(out, "empty.a"), b"!<arch>\n")
    # many sections: extended numbering (e_shnum == 0, SHN_XINDEX)
    with open(os.path.join(out, "many_sections.s"), "w") as f:
        for i in range(70000):
            f.write('\t.section .text.f%d,"ax",@progbits\n' % i)
            if i % 7000 == 0 or i > 69995:
                f.write("\t.globl f%d\nf%d: ret\nl%d: ret\n" % (i, i, i))
            else:
                f.write("\tret\n")
    run(["as", "many_sections.s", "-o", "many_sections.o"], cwd=out)
    run(["as", "--32", "many_sections.s", "-o", "many_sections32.o"], cwd=out)
    os.remove(os.path.join(out, "many_sections.s"))
    # hand-made but valid ELF files
    write(os.path.join(out, "big_endian64.o"), make_obj(64, ">", machine=21))
    write(os.path.join(out, "big_endian32.o"), make_obj(32, ">", machine=20))
    write(os.path.join(out, "machine_arm.o"), make_obj(64, "<", machine=183))
    write(os.path.join(out, "machine_none.o"), make_obj(32, "<", machine=0))
    write(os.path.join(out, "empty_names.o"), make_obj(syms=[
        ("", 0x12, ".text", 0x10, 0), ("", 0x10, 0, 0, 0), ("x", 0x12, ".text", 4, 0)]))
    write(os.path.join(out, "same_names.o"), make_obj(syms=[
        ("dup", 0x12, ".text", 0x30, 0), ("dup", 0x11, ".data", 0x10, 0),
        ("dup", 0x01, ".bss", 0x20, 0), ("dup", 0x10, 0, 0, 0)]))
    write(os.path.join(out, "max_values.o"), make_obj(syms=[
        ("max", 0x10, 0xfff1, 0xffffffffffffffff, 0xffffffffffffffff),
        ("zero", 0x10, 0xfff1, 0, 0)]))
    write(os.path.join(out, "only_null_symbol.o"), make_obj(syms=[]))
    write(os.path.join(out, "only_file_symbols.o"), make_obj(syms=[
        ("a.c", 0x04, 0xfff1, 0, 0), ("", 0x03, ".text", 0, 0)]))
    # every binding x type x section-index combination, 64- and 32-bit
    for cls in (64, 32):
        syms = []
        for b in (0, 1, 2, 10, 11, 12, 13, 14, 15):
            for t in (0, 1, 2, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15):
                for sh in (".text", ".data", ".bss", ".rodata", 0, 0xfff1,
                           0xfff2, 200, 0xff05):
                    tag = sh if isinstance(sh, str) else "%x" % sh
                    syms.append(("b%d_t%d_%s" % (b, t, tag.strip(".")),
                                 (b << 4) | t, sh, 8, 16))
        write(os.path.join(out, "symbol_grid%d.o" % cls), make_obj(cls, syms=syms))
    # nm behaviors we reproduce on purpose
    no_sections = Elf(make_obj(etype=3)).eh("shoff", 0).eh("shnum", 0).eh("shstrndx", 0)
    write(os.path.join(out, "dyn_without_sections.so"), no_sections.bytes())
    no_sections = Elf(make_obj(etype=2)).eh("shoff", 0).eh("shnum", 0).eh("shstrndx", 0)
    write(os.path.join(out, "exec_without_sections"), no_sections.bytes())


def gen_malformed(out, valid):
    """Corrupt and hostile inputs. Each one is a separate file."""
    def put(name, data):
        write(os.path.join(out, name), data)

    base64, base32 = make_obj(64), make_obj(32)
    real = os.path.join(valid, "basic.o")
    real = open(real, "rb").read() if os.path.exists(real) else base64

    # --- sizes and identification ---------------------------------------
    put("empty", b"")
    put("one_byte", b"\x7f")
    put("magic_only", b"\x7fELF")
    put("ident_only", base64[:16])
    put("header_truncated64", base64[:63])
    put("header_truncated32", base32[:51])
    put("text_file", b"this is not an ELF file\n" * 4)
    put("zeros", b"\0" * 4096)
    put("random_bytes", bytes((i * 131 + 7) % 256 for i in range(4096)))
    for name, idx, val in (("bad_magic", 1, ord("X")), ("class_0", 4, 0),
                           ("class_3", 4, 3), ("class_255", 4, 255),
                           ("data_0", 5, 0), ("data_3", 5, 3),
                           ("version_0", 6, 0), ("version_2", 6, 2)):
        d = bytearray(base64)
        d[idx] = val
        put(name, bytes(d))
    put("class32_with_64bit_layout", bytes(base64[:4]) + b"\x01" + base64[5:])
    put("class64_with_32bit_layout", bytes(base32[:4]) + b"\x02" + base32[5:])
    put("rel_without_sections", Elf(base64).eh("shoff", 0).eh("shnum", 0).bytes())

    # --- section header table -------------------------------------------
    size = len(base64)
    cases = [
        ("shoff_past_eof", "shoff", size + 100),
        ("shoff_at_eof", "shoff", size),
        ("shoff_last_byte", "shoff", size - 1),
        ("shoff_overflow", "shoff", (1 << 64) - 16),
        ("shoff_one", "shoff", 1),
        ("shoff_odd", "shoff", Elf(base64).eh("shoff") + 1),
        ("shentsize_0", "shentsize", 0),
        ("shentsize_1", "shentsize", 1),
        ("shentsize_63", "shentsize", 63),
        ("shentsize_65", "shentsize", 65),
        ("shentsize_max", "shentsize", 0xffff),
        ("shnum_max", "shnum", 0xfeff),
        ("shnum_one_too_many", "shnum", Elf(base64).eh("shnum") + 1),
        ("shstrndx_out_of_range", "shstrndx", 0xfe00),
        ("shstrndx_equals_shnum", "shstrndx", Elf(base64).eh("shnum")),
        ("shstrndx_points_to_text", "shstrndx", 1),
        ("shstrndx_zero", "shstrndx", 0),
    ]
    for name, field, value in cases:
        put(name, Elf(base64).eh(field, value).bytes())
        if not name.startswith("shoff_overflow"):
            put(name + "_32", Elf(base32).eh(field, value).bytes())
    # extended numbering with bad values in section 0
    put("xnum_huge", Elf(base64).eh("shnum", 0).sh(0, "size", 1 << 40).bytes())
    put("xnum_zero", Elf(base64).eh("shnum", 0).sh(0, "size", 0).bytes())
    put("xnum_wraps", Elf(base64).eh("shnum", 0).sh(0, "size", (1 << 64) - 1).bytes())
    put("xstrndx_huge", Elf(base64).eh("shstrndx", 0xffff).sh(0, "link", 0xffffffff).bytes())
    put("xnum_section0_past_eof", Elf(base64).eh("shnum", 0).eh("shoff", size + 8).bytes())

    # --- symbol table and string tables ---------------------------------
    for cls, base in ((64, base64), (32, base32)):
        sfx = "" if cls == 64 else "_32"
        e = Elf(base)
        st = e.find(SHT_SYMTAB)
        strt = e.sh(st, "link")
        shstr = e.eh("shstrndx")
        sec = [
            ("symtab_offset_past_eof", st, "offset", size * 4),
            ("symtab_offset_overflow", st, "offset", (1 << 64) - 8),
            ("symtab_size_huge", st, "size", (1 << 62)),
            ("symtab_size_overflow", st, "size", (1 << 64) - 8),
            ("symtab_size_not_multiple", st, "size", e.sh(st, "size") - 1),
            ("symtab_size_zero", st, "size", 0),
            ("symtab_entsize_0", st, "entsize", 0),
            ("symtab_entsize_1", st, "entsize", 1),
            ("symtab_entsize_25", st, "entsize", 25),
            ("symtab_link_self", st, "link", st),
            ("symtab_link_out_of_range", st, "link", 0xffff),
            ("symtab_link_to_text", st, "link", 1),
            ("symtab_link_to_null", st, "link", 0),
            ("strtab_offset_past_eof", strt, "offset", size * 4),
            ("strtab_size_huge", strt, "size", (1 << 62)),
            ("strtab_size_zero", strt, "size", 0),
            ("strtab_not_terminated", strt, "size", e.sh(strt, "size") - 1),
            ("strtab_is_symtab", strt, "offset", e.sh(st, "offset")),
            ("shstrtab_past_eof", shstr, "offset", size * 4),
            ("shstrtab_not_terminated", shstr, "size", e.sh(shstr, "size") - 1),
            ("shstrtab_wrong_type", shstr, "type", SHT_PROGBITS),
            ("section_name_out_of_range", 1, "name", 0xffffff),
            ("section_offset_past_eof", 1, "offset", size * 4),
            ("section_type_symtab_twice", 2, "type", SHT_SYMTAB),
        ]
        for name, idx, field, value in sec:
            put(name + sfx, Elf(base).sh(idx, field, value).bytes())
        n = Elf(base).nsyms()
        symc = [
            ("sym_name_out_of_range", 1, "name", 0xffffff),
            ("sym_name_last_byte", 1, "name", e.sh(strt, "size") - 1),
            ("sym_shndx_out_of_range", 1, "shndx", 300),
            ("sym_shndx_xindex", 1, "shndx", 0xffff),
            ("sym_shndx_loproc", 1, "shndx", 0xff00),
            ("sym_binding_5", 2, "info", 0x52),
            ("sym_binding_9", n - 1, "info", 0x92),
            ("sym_type_7", 1, "info", 0x17),
            ("sym0_binding_5", 0, "info", 0x52),
            ("sym0_named", 0, "name", 1),
        ]
        for name, idx, field, value in symc:
            put(name + sfx, Elf(base).sym(idx, field, value).bytes())
    # every symbol name invalid, and a symbol table pointing into itself
    e = Elf(base64)
    for i in range(1, e.nsyms()):
        e.sym(i, "name", 0x7fffffff)
    put("all_names_corrupt", e.bytes())
    e = Elf(base64)
    st = e.find(SHT_SYMTAB)
    put("symtab_overlaps_headers", e.sh(st, "offset", e.eh("shoff")).bytes())
    put("symtab_overlaps_ehdr", Elf(base64).sh(st, "offset", 0).bytes())

    # --- truncations of a real object -----------------------------------
    for cut in range(0, len(real), max(1, len(real) // 24)):
        put("truncated_at_%05d" % cut, real[:cut])
    put("truncated_last_byte", real[:-1])

    # --- archives ---------------------------------------------------------
    elf = real
    put("ar_magic_only_no_newline", b"!<arch>")
    put("ar_truncated_header", b"!<arch>\n" + ar_member("a.o/", elf)[:30])
    put("ar_bad_fmag", b"!<arch>\n" + ar_member("a.o/", elf, fmag=b"XX"))
    put("ar_size_not_number", b"!<arch>\n" + ar_member("a.o/", elf, b"abc"))
    put("ar_size_negative", b"!<arch>\n" + ar_member("a.o/", elf, b"-12"))
    put("ar_size_empty", b"!<arch>\n" + ar_member("a.o/", elf, b""))
    put("ar_size_spaces_first", b"!<arch>\n" + ar_member("a.o/", elf, b"  12"))
    put("ar_size_past_eof", b"!<arch>\n" + ar_member("a.o/", elf, b"999999"))
    put("ar_size_overflow", b"!<arch>\n" + ar_member("a.o/", elf, b"9999999999"))
    put("ar_missing_padding", (b"!<arch>\n" + ar_member("a.o/", elf + b"\0"))[:-1])
    put("ar_member_not_elf", b"!<arch>\n" + ar_member("a.o/", elf)
        + ar_member("notes.txt/", b"hello\n") + ar_member("b.o/", elf))
    put("ar_member_corrupt_elf", b"!<arch>\n" + ar_member(
        "bad.o/", Elf(elf).eh("shentsize", 1).bytes()) + ar_member("ok.o/", elf))
    put("ar_member_empty", b"!<arch>\n" + ar_member("zero.o/", b""))
    put("ar_long_name_no_table", b"!<arch>\n" + ar_member("/0", elf))
    put("ar_long_name_out_of_range", b"!<arch>\n" + ar_member("//", b"x.o/\n")
        + ar_member("/999", elf))
    put("ar_long_name_not_number", b"!<arch>\n" + ar_member("//", b"x.o/\n")
        + ar_member("/1x", elf))
    put("ar_long_name_table_after", b"!<arch>\n" + ar_member("/0", elf)
        + ar_member("//", b"x.o/\n"))
    put("ar_long_name_unterminated", b"!<arch>\n" + ar_member("//", b"abcdef")
        + ar_member("/0", elf))
    put("ar_name_without_slash", b"!<arch>\n" + ar_member("plainname", elf))
    put("ar_name_empty", b"!<arch>\n" + ar_member("/", b"") + ar_member("", elf))
    put("ar_bsd_name", b"!<arch>\n" + ar_member("#1/8", b"bsd.o\0\0\0" + elf))
    put("ar_nested_archive", b"!<arch>\n" + ar_member(
        "inner.a/", b"!<arch>\n" + ar_member("a.o/", elf)))
    put("ar_thin", b"!<thin>\n" + ar_member("a.o/", b"", b"100"))
    put("ar_garbage_after_member", b"!<arch>\n" + ar_member("a.o/", elf) + b"garbage")
    put("ar_many_small_members", b"!<arch>\n" + b"".join(
        ar_member("m%d.o/" % i, b"x") for i in range(500)))


def gen_bench(out):
    with open(os.path.join(out, "symbols_50k.c"), "w") as f:
        for i in range(50000):
            f.write("int sym_%08x_%d;\n" % ((i * 2654435761) % (1 << 32), i))
    run(["gcc", "-c", "symbols_50k.c", "-o", "symbols_50k.o"], cwd=out)
    with open(os.path.join(out, "symbols_10k.c"), "w") as f:
        for i in range(10000):
            f.write("int sym_%08x_%d;\n" % ((i * 2654435761) % (1 << 32), i))
    run(["gcc", "-c", "symbols_10k.c", "-o", "symbols_10k.o"], cwd=out)
    for name in ("symbols_50k.c", "symbols_10k.c"):
        os.remove(os.path.join(out, name))


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: gen_inputs.py <build_dir>")
    root = sys.argv[1]
    dirs = {k: os.path.join(root, k) for k in ("valid", "malformed", "bench")}
    for d in dirs.values():
        shutil.rmtree(d, ignore_errors=True)
        os.makedirs(d)
    gen_valid(dirs["valid"])
    gen_malformed(dirs["malformed"], dirs["valid"])
    gen_bench(dirs["bench"])
    for k, d in dirs.items():
        print("  %-9s %4d files" % (k, len(os.listdir(d))))


if __name__ == "__main__":
    main()
