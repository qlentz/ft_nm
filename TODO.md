# ft_nm — TODO before evaluation

Audit of commit `f8e8395` (2026-10-20), checked against [SUBJECT.md](SUBJECT.md).

**How it was checked**
- Built with `make`, then compared `ft_nm` and GNU `nm` 2.46.1 (Fedora 44) on 57 files:
  - 11 test files: 32/64-bit `.o`, `.so`, executables, C++ objects, IFUNC and common symbols.
  - 46 system files that still have a `.symtab`, including the 64-bit and 32-bit `libc.so.6`.
  - Every file was run under both `LANG=en_US.UTF-8` and `LC_ALL=C`.
- Error cases: missing file, directory, empty file, non-ELF file, no read permission, truncated file, stripped binary, `.a` archive, several files at once, no arguments.
- Several thousand mutated ELF files (random byte flips, truncations, extreme section-header values), plus 2 hand-made files.
- `valgrind --leak-check=full`.

**What already works:** parsing for 32/64-bit little-endian ELF, `.o`/`.so`/executables, address width and padding, most symbol letters, filtering of section and file symbols, most bounds checks (`offptr`, `safe_cstr_in_table`), no memory leaks, and no crash on random mutations.

**Current result:** 0 of 11 test files match `nm`. 38 of 46 system files match, but those are mostly tiny `crt*.o` files. Fixes P0-2 to P0-4 below were tried in a scratch copy (not committed). With them, **all 53 files that have symbols match `nm` byte for byte in both locales**, including both libcs (8,122 symbols in the 64-bit one) and a 50,000-symbol object. The only differences left were in the error and "no symbols" messages (P0-6).

---

## P0 — Blocking (each of these can fail the defense)

- [x] **1. Out-of-bounds read / segfault on a crafted `e_shentsize` or `sh_entsize`.** The code checks entries against the declared entry size, then reads a full `Elf64_Shdr` (64 bytes) or `Elf64_Sym` (24 bytes) anyway.
  - Test: a page-aligned `.o` with `e_shentsize = 1` and the section table in its last bytes. valgrind reports `Invalid read … r_u64 ← fill_meta64 ← read_section_meta` and the program dies with **SIGSEGV (exit 139)**.
  - Same result for a symtab with `sh_entsize = 1` (`fill_symbol64 ← read_symbol_view`).
  - It only runs clean outside valgrind because the next page happens to be mapped. The subject forbids this: *"In no way can your program quit in an unexpected manner"*.
  - Fix: require `e_shentsize == sizeof(Elf{32,64}_Shdr)` (40 / 64) in `load_section_table` ([src/ctx.c:129](src/ctx.c:129)), and symtab `sh_entsize == sizeof(Elf{32,64}_Sym)` (16 / 24) in `check_symtab_shape` ([src/sections.c:61](src/sections.c:61)). Otherwise report the file as corrupt.

- [x] **2. Sort order is wrong.** `nm` calls `setlocale(LC_ALL, "")` and sorts names with `strcoll`. The home-made key in `build_sort_key` / `compare_sort_key` ([src/symbols.c:142](src/symbols.c:142)) breaks ties with `strcmp`, so:
  - UTF-8 locale: `AB` comes before `ab`, and `_Alpha` before `__alpha`/`alpha`. `nm` gives `a__b, a_b, ab, AB, __alpha, alpha, _Alpha`.
  - `LC_ALL=C`: everything is wrong. `nm` uses plain byte order there (`Zeta` before `_Alpha` before `a__b`), and ft_nm ignores the locale.
  - Names longer than 255 characters (common in C++) get a truncated key.
  - Fix: `setlocale(LC_ALL, "")` in `main` and `strcoll(a->name, b->name)` as the **only** comparison. Both are in the C standard library, which the subject allows.
  - On equal names, keep symbol-table order with a stable sort. Do not compare by value or type: matching libc depends on this.

- [x] **3. Missing type `i` (GNU IFUNC, `STT_GNU_IFUNC` = 10).** ft_nm prints `T`/`t` instead, which accounts for 141 wrong lines in `/usr/lib64/libc.so.6` and also shows up in the IFUNC test objects.
  - Fix: in `classify_symbol` ([src/symbols.c:101](src/symbols.c:101)), return `'i'` for a defined IFUNC symbol. Do it before the weak/unique checks and do not lowercase it, matching `bfd_decode_symclass`.

- [x] **4. Address is blank for `u` (GNU unique) symbols.** `u` symbols are *defined* (for example C++ inline static locals, `_ZZ7countervE1c`), and `nm` prints their address.
  - The cause is [src/formatting.c:47](src/formatting.c:47), which treats `'u'` as undefined.
  - Fix: decide "blank address" from `shndx == SHN_UNDEF` (store a flag in `t_symbol`) rather than from the letter.

- [x] **5. No `file:` header when several files are given.** With more than one argument, `nm` prints `\n<file>:\n` before each file's output ([src/main.c:33](src/main.c:33)).
  - The header is still printed when that file then reports "no symbols".
  - It is not printed for open errors (missing file, directory).

- [x] **6. Error handling differs from `nm`.** The subject says *"behave like the system nm on every other aspect"*. A different program-name prefix is acceptable; the stream, the message meaning and the exit code are not.

  | Case | `nm` (stderr, exit code) | `ft_nm` today |
  |---|---|---|
  | missing file | `nm: 'x': No such file`, 1 | `ft_nm: failed to map x`, 1 |
  | no read permission | `nm: x: Permission denied`, 1 | `failed to map`, 1 |
  | directory | `nm: Warning: 'x' is a directory`, 1 | `failed to map`, 1 |
  | non-ELF file ≥ 16 bytes | `nm: x: file format not recognized`, 1 | `ft_nm: file is not an ELF file.` on **stdout** ([src/elf_ident.c:10](src/elf_ident.c:10)) |
  | non-ELF file < 16 bytes | `… file format not recognized`, 1 | **silent**, 1 ([src/elf_ident.c:52](src/elf_ident.c:52)) |
  | bad `EI_CLASS` | `… file format not recognized`, 1 | `uknown elf class flag.` |
  | no `.symtab` (stripped) | `nm: x: no symbols`, **0** | message on **stdout**, two different formats ([src/symbols.c:284](src/symbols.c:284) and [:292](src/symbols.c:292)) |
  | `.symtab` with only section/file symbols (`crtn.o`) | prints **nothing**, 0 | `x: no symbols` on stdout |
  | empty file | nothing, 1 | `failed to map`, 1 |
  | no argument and no `a.out` | `nm: 'a.out': No such file`, 1 | `failed to map a.out`, 1 |

  - Fix: send every diagnostic to stderr and use `strerror(errno)` for open errors. Check `S_ISDIR` after `fstat` ([src/ctx.c:17](src/ctx.c:17)). Use one "file format not recognized" path for every invalid header. Fix the typo "uknown".
  - Done: every diagnostic goes through `nm_error` ([src/errors.c](src/errors.c)) as `ft_nm: <file>: <reason>` on stderr. Intentional cosmetic differences from nm: `strerror` wording ("No such file or directory"), no `Warning:` prefix for directories and special files, an empty file reports "file format not recognized" instead of being silent, and corrupt files get a specific reason. Big-endian files still get their own refusal message (see item 10).

## P1 — Should fix (likely to be tested or criticized)

- [ ] **7. Performance.** The insertion sort is O(n²) and rebuilds two sort keys on every comparison, and output is one `write()` per character.
  - 50,000-symbol `.o`: **89 s** (`nm`: 0.05 s). `libc.so.6`: 2.6 s (`nm`: 0.02 s).
  - Printing libc makes **295,815 `write` syscalls** for 8,122 lines.
  - Fix together with P0-2: use a stable merge sort, or `qsort` that breaks ties on the original index. Buffer the output, for example a 4 KiB buffer flushed when full and at the end of each file.

- [ ] **8. Makefile.**
  - `make re` fails with `cannot find -lft`: `re: fclean $(NAME)` deletes `libft.a` and never rebuilds it ([Makefile:36](Makefile:36)). Use `re: fclean all`.
  - `$(NAME)` should depend on `libft/libft.a` ([Makefile:21](Makefile:21)).
  - Editing `ft_nm.h` triggers no rebuild. Add the header as a prerequisite, or use `-MMD -MP` and `-include $(DEPS)`.
  - Optional: `libft/Makefile` `clean` uses an undefined `$(BONUS_OBJS)`.

- [x] **9. Add a `.gitignore`** for `*.o`, `*.a` and `ft_nm`. After a build, `git status` lists about 60 untracked object files, and committing them by accident looks bad at the defense.

- [ ] **10. Check every parsed value.** The subject says: *"Make sure that every value that you parse is correct … flags, architecture, match your result with a reference"*.
  - Check `EI_VERSION == EV_CURRENT`, `e_ehsize`, the `e_shentsize` check from P0-1, `e_shoff`, and `sh_link`/`sh_type` (the last two are already partly done).
  - Decide on `e_machine` (EM_386 = 3, EM_X86_64 = 62) and be ready to explain the choice.
  - Big-endian files are refused on purpose ([src/elf_ident.c:37](src/elf_ident.c:37)), but `r_u16/32/64` already support big-endian. Either drop the refusal or keep it with an nm-style "file format not recognized" message.

- [ ] **11. Symbol-letter edge cases** in `section_letter` ([src/symbols.c:81](src/symbols.c:81)), compared with binutils `decode_section_type`:
  - `SHT_NOBITS` without `SHF_WRITE` should give `b` (today `R`).
  - A section without `SHF_ALLOC` should give `N` only for debug sections (`.debug*`, `.stab*`). Other read-only sections give `n`, and non-alloc executable sections give `t`.
  - A binding other than LOCAL/GLOBAL/WEAK/UNIQUE should give `?`.
  - None of these showed up in the test files, but evaluators like twisted inputs.

- [ ] **12. Decide on static archives (`.a`).** The subject only requires x86_32/x86_64 binaries, object files and `.so`, but `nm` lists every member of an archive (`\n<member>:\n` + symbols) and evaluators often try `libc_nonshared.a`.
  - Minimum: give nm's `file format not recognized` on stderr. Today it prints the "not an ELF file" message on stdout.
  - Full support: `!<arch>\n` magic, 60-byte member headers, the GNU `//` long-name table, and skipping the `/` index.

- [ ] **13. Extended section numbering** (`e_shnum == 0`, `e_shstrndx == SHN_XINDEX`, `st_shndx == SHN_XINDEX`). Today this gives "invalid section header table". It is rare, but either support it or be ready to explain why not.

## P2 — Cleanup and defense prep

- [ ] **14. Remove dead code:**
  - the `dynsym` slice and the commented-out code that reads it ([ft_nm.h:193](ft_nm.h:193), [src/ctx.c:163](src/ctx.c:163), [src/ctx.c:179](src/ctx.c:179), [src/symbols.c:10](src/symbols.c:10))
  - the unused `t_file.elf_class` ([ft_nm.h:23](ft_nm.h:23))
  - the commented `ft_putstr_fd` ([src/symbols.c:283](src/symbols.c:283))
  - the `ft_nm()` wrapper, which only forwards to `run_file`
- [ ] **15. Consistent style.** The code mixes 42-Norm-style files with K&R-style files (`readers.c`, `safety_utils.c`, `for` loops, C++ comments, a French comment in `ft_nm.h`). The Norm is **not** required by this subject ("corrected by humans only"), but one consistent style is easier to defend.
- [ ] **16. Dockerfile.** `alpine:latest` + `binutils` has no `gcc`/`make`, so the project cannot be built in it. Either remove the Dockerfile, or switch to a Debian/Ubuntu image with `build-essential gcc-multilib binutils` so 32-bit test files can be built and linked.
- [ ] **17. Add a reproducible test script**, for example `tests/compare.sh`:
  - diff `nm` and `./ft_nm` for stdout, stderr and exit code over a set of files, in both locales
  - build 32-bit inputs (`gcc -m32 -c`; linking also needs `gcc-multilib`)
  - include the two crafted corrupt files from P0-1
  - add a `valgrind` run

  It is useful to show during the defense.
- [ ] **18. Re-run the comparison on the evaluation machine (the subject's recommended Ubuntu VM).** Its `nm` version may word some errors differently from Fedora's 2.46, which for example adds an LTO-plugin line on truncated files.
- [ ] **19. AI transparency.** Chapter II asks you to *"clearly identify what was generated by AI tools"*. The sort fix came in through PR #1 from a `codex/…` branch. Note in a short README which parts were AI-assisted, and make sure you can explain every line, especially the ELF bounds checks and the sort.
- [ ] **20. Bonus (`-a -g -u -r -p`)**, only after everything above is done. Today any argument is treated as a file name, so `./ft_nm -a` tries to open a file called `-a`.

## Final checklist before pushing to the evaluation repo

- [ ] Fresh clone → `make` → `make` (no relink) → `make re` → `make clean` → `make fclean` all succeed.
- [ ] `diff <(nm f) <(./ft_nm f)` is empty for: 64-bit `.o`/`.so`/exe, 32-bit `.o`/`.so`/exe, C++ objects, both locales, and several files at once.
- [ ] Every error case in the P0-6 table matches `nm` (stream + exit code).
- [ ] The crafted and fuzzed files cause no crash, and `valgrind` reports no errors or leaks.
- [ ] No build artifacts committed. Decide whether `SUBJECT.md`/`TODO.md` stay in the turned-in repo.
