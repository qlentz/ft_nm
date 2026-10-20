#!/usr/bin/env bash
# Full test suite for ft_nm.
#
# Usage: tests/run_tests.sh [--quick]
#   --quick   only the comparison and robustness tests
#             (no valgrind, fuzzing or benchmarks)
#
# Sections:
#   norm        every C file and header in the repo passes norminette.
#   compare     valid files, system files and error cases: stdout and exit
#               code must equal nm's, and stderr must be empty exactly when
#               nm's is. Run in a UTF-8 locale and in LC_ALL=C.
#   robustness  malformed files (tests/gen_inputs.py): no crash, no hang,
#               and every exit 1 comes with an error message.
#   valgrind    no memory errors or leaks on valid and malformed files.
#   fuzz        random mutations of valid files (tests/fuzz.py).
#   bench       ft_nm and nm timings on large inputs.
#
# Every run appends its results with a timestamp and the git commit to
# tests/history.csv, so changes in speed or results can be tracked.

set -u

ROOT=$(cd "$(dirname "$0")/.." && pwd)
FT="$ROOT/ft_nm"
BUILD="$ROOT/tests/build"
HISTORY="$ROOT/tests/history.csv"
QUICK=0
[ "${1:-}" = "--quick" ] && QUICK=1

RUN_DATE=$(date -Iseconds)
COMMIT=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)
if [ -n "$(git -C "$ROOT" status --porcelain -- src ft_nm.h 2>/dev/null)" ]; then
	COMMIT="$COMMIT+dirty"
fi
TOTAL_FAILS=0

if [ -t 1 ]; then
	GREEN=$'\e[32m'; RED=$'\e[31m'; BOLD=$'\e[1m'; RESET=$'\e[0m'
else
	GREEN=; RED=; BOLD=; RESET=
fi

now() { date +%s.%N; }
elapsed() { awk -v a="$1" -v b="$(now)" 'BEGIN { printf "%.2f", b - a }'; }
title() { printf '\n%s== %s ==%s  (%s)\n' "$BOLD" "$1" "$RESET" "$(date +%T)"; }
record() { echo "$RUN_DATE,$COMMIT,$1,$2,$3,$4" >> "$HISTORY"; }
fail() { printf '  %sFAIL%s %s\n' "$RED" "$RESET" "$1"; }

# report <section> <passed> <total> <start>
report() {
	local secs color=$GREEN
	secs=$(elapsed "$4")
	[ "$2" -ne "$3" ] && color=$RED
	printf '  %s%s/%s passed%s in %ss\n' "$color" "$2" "$3" "$RESET" "$secs"
	record section "$1" "$2/$3" "$secs"
	TOTAL_FAILS=$((TOTAL_FAILS + $3 - $2))
}

# --------------------------------------------------------------------------
title "build"
START=$(now)
[ -f "$HISTORY" ] || echo "date,commit,kind,name,value1,value2" > "$HISTORY"
if ! make -s -C "$ROOT" >/dev/null; then
	echo "make failed"
	exit 1
fi
mkdir -p "$BUILD"
python3 "$ROOT/tests/gen_inputs.py" "$BUILD" || exit 1
echo "  commit $COMMIT, inputs built in $(elapsed "$START")s"

UTF8_LOCALE=$(locale -a 2>/dev/null | grep -iE '^(en_US|C)\.utf-?8$' | sort -r | head -1)
LOCALES="C"
[ -n "$UTF8_LOCALE" ] && LOCALES="$UTF8_LOCALE C"

# --------------------------------------------------------------------------
title "norm"
START=$(now)
if command -v norminette >/dev/null; then
	pass=0; total=0
	while read -r f; do
		total=$((total + 1))
		if norminette "$f" 2>/dev/null | grep -q '^Error'; then
			fail "$f"
			norminette "$f" 2>/dev/null | grep '^Error' | head -3 | sed 's/^/      /'
		else
			pass=$((pass + 1))
		fi
	done < <(cd "$ROOT" && find . \( -name '*.c' -o -name '*.h' \) \
		-not -path './obj/*' -not -path './libft/obj/*' -not -path './tests/build/*' \
		| sort)
	report norm "$pass" "$total" "$START"
else
	echo "  norminette not installed: skipped"
fi

# --------------------------------------------------------------------------
title "compare with nm (locales: $LOCALES)"
START=$(now)
CASES=()
for f in "$BUILD"/valid/*; do CASES+=("$f"); done
# system files: libc, a few shared libraries and archives, a stripped binary
for d in /usr/lib64 /usr/lib/x86_64-linux-gnu /lib/x86_64-linux-gnu /usr/lib \
		/usr/lib32 /usr/lib/i386-linux-gnu; do
	[ -f "$d/libc.so.6" ] && CASES+=("$d/libc.so.6")
	[ -f "$d/libc_nonshared.a" ] && CASES+=("$d/libc_nonshared.a")
done
while read -r f; do CASES+=("$f"); done < <(ls /usr/lib64/*.so.* \
	/usr/lib/x86_64-linux-gnu/*.so.* 2>/dev/null | sort | head -40)
CASES+=("$(command -v ls)")
# several files at once, missing files and other errors
# (not the empty file: ft_nm reports it on purpose, nm is silent)
V="$BUILD/valid"
CASES+=("$V/basic.o $V/lib.a $V/stripped.so $V/basic32.o"
	"missing_file $V/basic.o" "$V/basic.o missing_file" "missing_file"
	"$ROOT/tests" "/dev/null" "$BUILD/malformed/text_file"
	"$V/empty.a $V/basic.o")
mkdir -p "$BUILD/paths"
rm -f "$BUILD/paths/"*
mkfifo "$BUILD/paths/fifo"
ln -s "$V/basic.o" "$BUILD/paths/link.o"
ln -s "$BUILD/paths/nowhere" "$BUILD/paths/broken.o"
ln -s "$BUILD/paths/loop_b" "$BUILD/paths/loop_a"
ln -s "$BUILD/paths/loop_a" "$BUILD/paths/loop_b"
cp "$V/basic.o" "$BUILD/paths/with space.o"
cp "$V/basic.o" "$BUILD/paths/no_read.o"
chmod 000 "$BUILD/paths/no_read.o"
for f in fifo link.o broken.o loop_a "with space.o"; do
	CASES+=("$BUILD/paths/$f")
done
[ "$(id -u)" -ne 0 ] && CASES+=("$BUILD/paths/no_read.o")
CASES+=("$BUILD/paths/$(printf 'x%.0s' $(seq 300))")
CASES+=("")

pass=0; total=0
for loc in $LOCALES; do
	for c in "${CASES[@]}"; do
		total=$((total + 1))
		if [ -z "$c" ]; then args=("") ; else read -r -a args <<< "$c"; fi
		[ -e "$c" ] && args=("$c")
		ft_out=$(LC_ALL=$loc timeout 120 "$FT" "${args[@]}" 2>"$BUILD/ft_err"; echo "exit=$?")
		nm_out=$(LC_ALL=$loc timeout 120 nm "${args[@]}" 2>"$BUILD/nm_err"; echo "exit=$?")
		grep -v '^bfd plugin' "$BUILD/nm_err" > "$BUILD/nm_err2"
		ft_has_err=$([ -s "$BUILD/ft_err" ] && echo 1 || echo 0)
		nm_has_err=$([ -s "$BUILD/nm_err2" ] && echo 1 || echo 0)
		if [ "$ft_out" = "$nm_out" ] && [ "$ft_has_err" = "$nm_has_err" ]; then
			pass=$((pass + 1))
		else
			fail "[$loc] ${c:-<empty argument>}"
			diff <(echo "$ft_out"; cat "$BUILD/ft_err") \
				<(echo "$nm_out"; cat "$BUILD/nm_err2") | head -6 | sed 's/^/      /'
		fi
	done
done
chmod 644 "$BUILD/paths/no_read.o"
report compare "$pass" "$total" "$START"

# --------------------------------------------------------------------------
title "robustness (malformed files)"
START=$(now)
pass=0; total=0; same_as_nm=0
for f in "$BUILD"/malformed/*; do
	total=$((total + 1))
	timeout 10 "$FT" "$f" >"$BUILD/ft_out" 2>"$BUILD/ft_err"
	code=$?
	if [ "$code" -gt 1 ]; then
		fail "$(basename "$f"): exit code $code (crash or hang)"
	elif [ "$code" -eq 1 ] && [ ! -s "$BUILD/ft_err" ]; then
		fail "$(basename "$f"): exit 1 without an error message"
	else
		pass=$((pass + 1))
	fi
	nm "$f" >"$BUILD/nm_out" 2>/dev/null
	[ $? -eq "$code" ] && cmp -s "$BUILD/ft_out" "$BUILD/nm_out" \
		&& same_as_nm=$((same_as_nm + 1))
done
echo "  (info: $same_as_nm/$total give the same stdout and exit code as nm)"
report robustness "$pass" "$total" "$START"

if [ "$QUICK" -eq 1 ]; then
	title "done (quick)"
	[ "$TOTAL_FAILS" -eq 0 ] && echo "  ${GREEN}all tests passed${RESET}" \
		|| echo "  ${RED}$TOTAL_FAILS failure(s)${RESET}"
	exit $((TOTAL_FAILS > 0))
fi

# --------------------------------------------------------------------------
title "valgrind"
START=$(now)
if command -v valgrind >/dev/null; then
	pass=0; total=0
	for f in "$BUILD"/valid/* "$BUILD"/malformed/*; do
		total=$((total + 1))
		valgrind -q --error-exitcode=99 --leak-check=full \
			--errors-for-leak-kinds=all "$FT" "$f" >/dev/null 2>"$BUILD/vg_err"
		if [ $? -eq 99 ] || grep -q '^==' "$BUILD/vg_err"; then
			fail "$(basename "$f")"
			grep '^==' "$BUILD/vg_err" | head -5 | sed 's/^/      /'
		else
			pass=$((pass + 1))
		fi
	done
	report valgrind "$pass" "$total" "$START"
else
	echo "  valgrind not installed: skipped"
fi

# --------------------------------------------------------------------------
title "fuzz"
START=$(now)
FUZZ_SEED=${FUZZ_SEED:-$(date +%s)}
FUZZ_RUNS=${FUZZ_RUNS:-1000}
FUZZ_BASES=()
for f in basic.o basic32.o lib.a big_endian64.o sections.o libtest.so many_sections.o; do
	[ -f "$V/$f" ] && FUZZ_BASES+=("$V/$f")
done
read -r runs crashes hangs silent < <(python3 "$ROOT/tests/fuzz.py" "$FT" \
	"$BUILD/fuzz_failures" "$FUZZ_RUNS" "$FUZZ_SEED" "${FUZZ_BASES[@]}")
echo "  seed $FUZZ_SEED: $runs runs, $crashes crashes, $hangs hangs, $silent silent failures"
[ $((crashes + hangs + silent)) -gt 0 ] && fail "inputs saved in $BUILD/fuzz_failures"
report fuzz $((runs - crashes - hangs - silent)) "$runs" "$START"

# --------------------------------------------------------------------------
title "bench (seconds, one run each)"
BENCH=("$BUILD/bench/symbols_10k.o" "$BUILD/bench/symbols_50k.o"
	"$V/many_sections.o" "$V/lib.a")
for d in /usr/lib64 /usr/lib/x86_64-linux-gnu; do
	[ -f "$d/libc.so.6" ] && BENCH+=("$d/libc.so.6") && break
done
printf '  %-28s %10s %10s\n' file ft_nm nm
for f in "${BENCH[@]}"; do
	[ -f "$f" ] || continue
	s=$(now); timeout 600 "$FT" "$f" >/dev/null 2>&1; ft_t=$(elapsed "$s")
	s=$(now); nm "$f" >/dev/null 2>&1; nm_t=$(elapsed "$s")
	printf '  %-28s %10s %10s\n' "$(basename "$f")" "$ft_t" "$nm_t"
	record bench "$(basename "$f")" "$ft_t" "$nm_t"
done

# --------------------------------------------------------------------------
title "done"
echo "  results appended to tests/history.csv ($RUN_DATE, $COMMIT)"
if [ "$TOTAL_FAILS" -eq 0 ]; then
	echo "  ${GREEN}all tests passed${RESET}"
else
	echo "  ${RED}$TOTAL_FAILS failure(s)${RESET}"
fi
exit $((TOTAL_FAILS > 0))
