#!/usr/bin/env python3
"""Mutation fuzzer for ft_nm.

Usage: fuzz.py <ft_nm> <failures_dir> <iterations_per_file> <seed> <file>...

Each base file is mutated many times (byte flips, interesting values in
random places, truncations, block copies). A run fails if ft_nm crashes
(signal or exit code other than 0/1), hangs, or exits 1 without printing
an error. Failing inputs are saved to <failures_dir>.
Prints one summary line: "runs crashes hangs silent".
"""
import os
import random
import subprocess
import sys

INTERESTING = [0, 1, 2, 0x7f, 0x80, 0xff, 0xfe]
INTERESTING_WIDE = [b"\xff\xff", b"\x00\x00", b"\xff\xff\xff\xff",
                    b"\x00\x00\x00\x80", b"\xff" * 8, b"\x00" * 8,
                    b"\xf1\xff", b"\xf2\xff", b"\x00\xff"]


def mutate(data, rng):
    d = bytearray(data)
    if not d:
        return bytes([rng.randrange(256)])
    for _ in range(rng.randint(1, 8)):
        choice = rng.random()
        # headers and tables live at the start: bias mutations there
        limit = len(d) if rng.random() < 0.3 else min(len(d), 512)
        pos = rng.randrange(limit)
        if choice < 0.4:
            d[pos] = rng.randrange(256)
        elif choice < 0.6:
            d[pos] = rng.choice(INTERESTING)
        elif choice < 0.8:
            val = rng.choice(INTERESTING_WIDE)
            d[pos:pos + len(val)] = val
        elif choice < 0.9 and len(d) > 1:
            d = d[:rng.randrange(1, len(d))]
        else:
            src = rng.randrange(len(d))
            size = rng.randint(1, 64)
            d[pos:pos + size] = d[src:src + size]
    return bytes(d)


def main():
    ft_nm, fail_dir, iterations, seed = sys.argv[1:5]
    files = sys.argv[5:]
    iterations, rng = int(iterations), random.Random(int(seed))
    os.makedirs(fail_dir, exist_ok=True)
    target = os.path.join(fail_dir, "current_input")
    stats = {"runs": 0, "crashes": 0, "hangs": 0, "silent": 0}
    for base in files:
        data = open(base, "rb").read()
        for i in range(iterations):
            sample = mutate(data, rng)
            with open(target, "wb") as f:
                f.write(sample)
            stats["runs"] += 1
            kind = None
            try:
                r = subprocess.run([ft_nm, target], capture_output=True,
                                   timeout=10)
                if r.returncode not in (0, 1):
                    kind = "crashes"
                elif r.returncode == 1 and not r.stderr:
                    kind = "silent"
            except subprocess.TimeoutExpired:
                kind = "hangs"
            if kind:
                stats[kind] += 1
                name = "%s_%s_%d" % (kind, os.path.basename(base), i)
                with open(os.path.join(fail_dir, name), "wb") as f:
                    f.write(sample)
    os.remove(target)
    print("%(runs)d %(crashes)d %(hangs)d %(silent)d" % stats)


if __name__ == "__main__":
    main()
