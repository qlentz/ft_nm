# ft_nm — TODO before evaluation

Requirements are in [SUBJECT.md](SUBJECT.md).

**Current state.** Compared with GNU `nm` 2.46.1 (Fedora 44), ft_nm gives identical stdout on every test file, in both `LANG=en_US.UTF-8` and `LC_ALL=C`:
- 32/64-bit `.o`, `.so` and executables
- C++ objects with `u` symbols, IFUNC objects
- both `libc.so.6` (8,122 symbols in the 64-bit one)
- several files at once

The code passes `norminette` (src, `ft_nm.h`, libft). Errors go to stderr as `ft_nm: <file>: <reason>` with nm's exit codes. Crafted and fuzzed files cause no crash, and valgrind reports no errors or leaks. The Makefile tracks header dependencies, and `make re` works.

Known cosmetic differences from nm, kept on purpose:
- `strerror` wording ("No such file or directory")
- no `Warning:` prefix for directories and special files
- an empty file reports "file format not recognized" (nm is silent)
- corrupt files get a specific reason

---

## P0 — Blocking

- [ ] **1. Performance.** The insertion sort is O(n²), and `strcoll` is slower than `strcmp`. Output is one `write()` per character.
  - 50,000-symbol `.o`: **17 s** (`nm`: 0.05 s). `libc.so.6`: 0.3 s.
  - Printing libc makes **295,815 `write` syscalls** for 8,122 lines.
  - Fix: a stable merge sort, which keeps equal names in symbol-table order (needed to match libc). Buffer the output, for example a 4 KiB buffer flushed when full and at the end of each file. Remember that stdout and stderr interleave, so flush stdout before writing an error. Keep every new file within the Norm.

## P1 — Should fix (likely to be tested or criticized)

- [x] **2. Check every parsed value.**
  - Rejected like nm: bad magic, class or byte order, `EI_VERSION != EV_CURRENT`, bad `e_shentsize`/`e_shnum`/`e_shstrndx`, tables out of bounds, bad symtab `sh_entsize`, `sh_link` not a `SHT_STRTAB`.
  - A symbol table with an undefined binding (3–9) or type (7) is rejected like nm: one `symbol number N uses unsupported …` message, then `no symbols`, exit 0. Symbol 0 is checked too, but it is never listed.
  - A name offset outside the string table, or an unterminated name: warning on stderr, symbol printed as `<corrupt>`.
  - A section index past the table prints `A`/`a`, like nm (commented in [src/symbol_type.c](src/symbol_type.c)).
  - Big-endian files are listed normally.
  - Not checked, like nm: `e_ehsize`, `e_type`, `e_machine`. Nothing we parse depends on them. Any machine is accepted; be ready to explain that at the defense.

- [x] **3. Symbol letters follow binutils' `bfd_decode_symclass`** ([src/symbol_type.c](src/symbol_type.c)).
  - Order: common (`C`, value = `st_size`), undefined (`U`, `w`/`v`), IFUNC (`i`), weak (`W`/`V`), unique (`u`), other OS/processor bindings (`?`), then the section letter, uppercase only for global symbols.
  - Section letters come from the section flags (`t`, `r`, `d`, `b`, `N` for debug sections by name, `n`, `?`) and the PE names `.idata`/`.drectve` (`i`), `.edata` (`e`), `.pdata` (`p`).
  - Symbols with an empty name are listed, like nm.
  - Checked against nm: every binding × type × section-index combination (3,584 cases, both byte orders), an assembled file with 28 kinds of sections, and real `-fcommon` symbols.

- [x] **4. Static archives (`.a`).** Each member is listed under `\n<member>:` like nm, with GNU long names (`//`) and the `/` and `/SYM64/` indexes skipped ([src/archive.c](src/archive.c), [src/archive_name.c](src/archive_name.c)). All 22 system archives give the same stdout, stderr and exit code as nm.
  - Choices to explain at the defense: a malformed archive stops with `malformed archive` and exit 1, and a member that fails also gives exit 1 (nm exits 0 in both cases). Thin archives (`!<thin>`) are refused: their members are outside files.

- [x] **5. Extended section numbering.** When `e_shnum` is 0 or `e_shstrndx` is `SHN_XINDEX`, the real values come from section 0. Symbols with `SHN_XINDEX` use the `SHT_SYMTAB_SHNDX` section, and a missing one is reported like nm. A 70,011-section object (64- and 32-bit) gives the same output as nm.
  - Also like nm: a file without a section table (`e_shoff == 0`) gives `no symbols` (exit 0) for executables and shared objects, and `file format not recognized` for `.o` files.

## P2 — Cleanup and defense prep

- [ ] **6. Dockerfile.** `alpine:latest` + `binutils` has no `gcc`/`make`, so the project cannot be built in it. Either remove the Dockerfile, or switch to a Debian/Ubuntu image with `build-essential gcc-multilib binutils` so 32-bit test files can be built and linked.
- [ ] **7. Add a reproducible test script**, for example `tests/compare.sh`:
  - diff `nm` and `./ft_nm` for stdout, stderr (with `nm:` replaced by `ft_nm:`) and exit code over a set of files, in both locales
  - build 32-bit inputs (`gcc -m32 -c`; linking also needs `gcc-multilib`)
  - include crafted corrupt files: `e_shentsize = 1`, symtab `sh_entsize = 1`, an undefined symbol with `STB_GNU_UNIQUE` binding
  - add a `valgrind` run and a short fuzzing loop

  It is useful to show during the defense.
- [ ] **8. Re-run the comparison on the evaluation machine (the subject's recommended Ubuntu VM).** Its `nm` version may word some errors differently from Fedora's 2.46.
- [ ] **9. Editor setup (optional).** Add a `compile_flags.txt` with `-I.` and `-I./libft` so clangd (Zed) finds `ft_nm.h` and stops flagging everything as undefined.
- [ ] **10. AI transparency.** Chapter II asks you to *"clearly identify what was generated by AI tools"*. Note in a short README which parts were AI-assisted, and make sure you can explain every line, especially the ELF bounds checks, the locale-aware sort and the symbol-letter rules.
- [ ] **11. Bonus (`-a -g -u -r -p`)**, only after everything above is done. Today any argument is treated as a file name, so `./ft_nm -a` tries to open a file called `-a`.

## Final checklist before pushing to the evaluation repo

- [ ] `norminette src ft_nm.h libft` reports no errors.
- [ ] Fresh clone → `make` → `make` (no relink) → `make re` → `make clean` → `make fclean` all succeed.
- [ ] `diff <(nm f) <(./ft_nm f)` is empty for: 64-bit `.o`/`.so`/exe, 32-bit `.o`/`.so`/exe, C++ objects, both locales, and several files at once.
- [ ] Error cases (missing file, no permission, directory, `/dev/null`, empty, non-ELF, stripped, no argument) go to stderr with nm's exit code.
- [ ] The crafted and fuzzed files cause no crash, and `valgrind` reports no errors or leaks.
- [ ] No build artifacts committed. Decide whether `SUBJECT.md`/`TODO.md` stay in the turned-in repo.
