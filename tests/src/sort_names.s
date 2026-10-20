# Names that only sort like nm when using the locale (strcoll), and names
# C cannot spell: mixed case, dots, dollar signs, digits, many underscores.
	.data
	.irp name, ab, AB, a_b, a__b, _Alpha, __alpha, alpha, Zeta, b, same_1, same_2, same_10, Same_1, SAME_1, _same_1, __same_1, x9, x10, x_9, X9, x_10, a.b, a$b, A.b, .Ldot, $dollar, a1b, a10b, a2b, ___, _, __, ____x, _x_, X_, x, x.1, x.10, x.2
	.globl \name
\name:	.long 0
	.endr
	.local local_ab, local_AB, _local
local_ab: .long 0
local_AB: .long 0
_local: .long 0
