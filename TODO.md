# ft_nm — TODO before evaluation

Requirements are in [SUBJECT.md](SUBJECT.md).

**Current state.** Compared with GNU `nm` 2.46.1 (Fedora 44), ft_nm gives identical stdout on every test file, in both `LANG=en_US.UTF-8` and `LC_ALL=C`:
- 32/64-bit `.o`, `.so` and executables
- C++ objects with `u` symbols, IFUNC objects
- both `libc.so.6` (8,122 symbols in the 64-bit one)
- several files at once

Errors go to stderr as `ft_nm: <file>: <reason>` with nm's exit codes. Crafted and fuzzed files cause no crash, and valgrind reports no errors or leaks. The Makefile tracks header dependencies, and `make re` works.

Known cosmetic differences from nm, kept on purpose:
- `strerror` wording ("No such file or directory")
- no `Warning:` prefix for directories and special files
- an empty file reports "file format not recognized" (nm is silent)
- corrupt files get a specific reason

---

## P0 — Blocking

- [ ] **1. Pass the 42 Norm.** Every file we wrote must pass `norminette` (v3.3.59 is installed at `~/.local/bin/norminette`). Check with `norminette src ft_nm.h libft`. Today there are about 690 errors:

  | File | Errors | Structural problems (beyond tabs, spacing and `return (x)`) |
  |---|---|---|
  | `ft_nm.h` | 434 | anonymous `typedef struct` (needs `s_` names), function-like macros `ELF_ST_BIND`/`ELF_ST_TYPE` are forbidden (use functions), `t_ElfMeta` has an uppercase name, the `enum` and `#define`s need the Norm layout, trailing comments |
  | `readers.c` | 115 | `for` loop, declaration with assignment, several instructions per line, comments inside functions |
  | `safety_utils.c` | 97 | `for` loop, declaration not at the start of the function, comments inside functions |
  | `symbols.c` | 19 | 17 functions (max 5 per file), 2 functions with more than 4 parameters, 2 ternaries |
  | `ctx.c` | 13 | 12 functions, 1 function with more than 4 parameters, 2 ternaries, 1 line too long |
  | `sections.c` | 5 | 7 functions, 1 ternary, 1 line too long |
  | `main.c`, `errors.c`, `elf_ident.c`, `formatting.c` | 1 each | only the missing 42 header |
  | `libft/` | 4 | `ft_memchr.c`, `gnl_utils.c`, `ft_lstadd_front_bonus.c` (indentation) |

  - Every file needs the 42 header (`INVALID_HEADER`).
  - Splitting into ≤ 5 functions per file means new source files. Group them by job, for example `file.c` (open/mmap), `elf_header.c`, `section_table.c`, `symtab.c`, `symbol_type.c`, `sort.c`, `print.c`.
  - Use consistent `#include "ft_nm.h"`. `elf_ident.c`, `readers.c` and `safety_utils.c` use `<ft_nm.h>`.
  - Re-run the full comparison after the refactor. It is easy to break behavior while splitting functions.

- [ ] **2. Performance.** The insertion sort is O(n²), and `strcoll` is slower than `strcmp`. Output is one `write()` per character.
  - 50,000-symbol `.o`: **17 s** (`nm`: 0.05 s). `libc.so.6`: 0.3 s.
  - Printing libc makes **295,815 `write` syscalls** for 8,122 lines.
  - Fix: a stable merge sort, which keeps equal names in symbol-table order (needed to match libc). Buffer the output, for example a 4 KiB buffer flushed when full and at the end of each file. Remember that stdout and stderr interleave, so flush stdout before writing an error.

## P1 — Should fix (likely to be tested or criticized)

- [ ] **3. Check every parsed value.** The subject says: *"Make sure that every value that you parse is correct … flags, architecture, match your result with a reference"*.
  - Already checked: magic, `EI_CLASS`, `EI_DATA`, `e_shentsize`, `e_shnum`, `e_shstrndx < e_shnum`, section table bounds, symtab `sh_entsize` and bounds, `sh_link` → `SHT_STRTAB`, string offsets.
  - Still to check: `EI_VERSION == EV_CURRENT`, `e_ehsize`, and an out-of-range `st_name`. Today ft_nm silently drops that symbol. nm warns `invalid string offset … for section '.strtab'` on stderr and prints the symbol as `<corrupt>`, exit 0.
  - Decide on `e_machine` (EM_386 = 3, EM_X86_64 = 62) and be ready to explain the choice.
  - Big-endian files are refused with "big-endian ELF files are not supported" ([src/elf_ident.c](src/elf_ident.c)), but `r_u16/32/64` already support big-endian. Either drop the refusal or keep it and be ready to explain it.

- [ ] **4. Symbol-letter edge cases** in `section_letter` ([src/symbols.c](src/symbols.c)), compared with binutils `decode_section_type`:
  - `SHT_NOBITS` without `SHF_WRITE` should give `b` (today `R`).
  - A section without `SHF_ALLOC` should give `N` only for debug sections (`.debug*`, `.stab*`). This needs the section name: `t_section_meta.name_index` is read but not used yet. Other read-only sections give `n`, and non-alloc executable sections give `t`.
  - A binding other than LOCAL/GLOBAL/WEAK/UNIQUE should give `?`.
  - None of these showed up in the test files, but evaluators like twisted inputs.

- [ ] **5. Decide on static archives (`.a`).** The subject only requires x86_32/x86_64 binaries, object files and `.so`, but `nm` lists every member of an archive (`\n<member>:\n` + symbols) and evaluators often try `libc_nonshared.a`.
  - Today an archive gets "file format not recognized" on stderr, exit 1. That is acceptable if you can explain it.
  - Full support: `!<arch>\n` magic, 60-byte member headers, the GNU `//` long-name table, and skipping the `/` index.

- [ ] **6. Extended section numbering** (`e_shnum == 0`, `e_shstrndx == SHN_XINDEX`, `st_shndx == SHN_XINDEX`). Today this gives "invalid section header table". It is rare, but either support it or be ready to explain why not.

## P2 — Cleanup and defense prep

- [ ] **7. Dockerfile.** `alpine:latest` + `binutils` has no `gcc`/`make`, so the project cannot be built in it. Either remove the Dockerfile, or switch to a Debian/Ubuntu image with `build-essential gcc-multilib binutils` so 32-bit test files can be built and linked.
- [ ] **8. Add a reproducible test script**, for example `tests/compare.sh`:
  - diff `nm` and `./ft_nm` for stdout, stderr (with `nm:` replaced by `ft_nm:`) and exit code over a set of files, in both locales
  - build 32-bit inputs (`gcc -m32 -c`; linking also needs `gcc-multilib`)
  - include crafted corrupt files: `e_shentsize = 1`, symtab `sh_entsize = 1`, an undefined symbol with `STB_GNU_UNIQUE` binding
  - add a `valgrind` run and a short fuzzing loop

  It is useful to show during the defense.
- [ ] **9. Re-run the comparison on the evaluation machine (the subject's recommended Ubuntu VM).** Its `nm` version may word some errors differently from Fedora's 2.46.
- [ ] **10. Editor setup (optional).** Add a `compile_flags.txt` with `-I.` and `-I./libft` so clangd (Zed) finds `ft_nm.h` and stops flagging everything as undefined.
- [ ] **11. AI transparency.** Chapter II asks you to *"clearly identify what was generated by AI tools"*. Note in a short README which parts were AI-assisted, and make sure you can explain every line, especially the ELF bounds checks, the locale-aware sort and the symbol-letter rules.
- [ ] **12. Bonus (`-a -g -u -r -p`)**, only after everything above is done. Today any argument is treated as a file name, so `./ft_nm -a` tries to open a file called `-a`.

## Final checklist before pushing to the evaluation repo

- [ ] `norminette src ft_nm.h libft` reports no errors.
- [ ] Fresh clone → `make` → `make` (no relink) → `make re` → `make clean` → `make fclean` all succeed.
- [ ] `diff <(nm f) <(./ft_nm f)` is empty for: 64-bit `.o`/`.so`/exe, 32-bit `.o`/`.so`/exe, C++ objects, both locales, and several files at once.
- [ ] Error cases (missing file, no permission, directory, `/dev/null`, empty, non-ELF, stripped, no argument) go to stderr with nm's exit code.
- [ ] The crafted and fuzzed files cause no crash, and `valgrind` reports no errors or leaks.
- [ ] No build artifacts committed. Decide whether `SUBJECT.md`/`TODO.md` stay in the turned-in repo.
