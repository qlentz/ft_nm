# One global and one local symbol in each unusual kind of section.
	.section ".idata","aw",@progbits
	.globl idata_g
idata_g: .byte 1
idata_l: .byte 1
	.section ".idata$2","aw",@progbits
	.globl idata2_g
idata2_g: .byte 1
idata2_l: .byte 1
	.section ".edata","aw",@progbits
	.globl edata_g
edata_g: .byte 1
edata_l: .byte 1
	.section ".pdata","aw",@progbits
	.globl pdata_g
pdata_g: .byte 1
pdata_l: .byte 1
	.section ".drectve","aw",@progbits
	.globl drectve_g
drectve_g: .byte 1
drectve_l: .byte 1
	.section ".robss","a",@nobits
	.globl robss_g
robss_g: .zero 1
robss_l: .zero 1
	.section ".nonalloc_w","w",@progbits
	.globl nonalloc_w_g
nonalloc_w_g: .byte 1
nonalloc_w_l: .byte 1
	.section ".nonalloc_nobits","",@nobits
	.globl nonalloc_nobits_g
nonalloc_nobits_g: .zero 1
nonalloc_nobits_l: .zero 1
	.section ".comment.x","",@progbits
	.globl nonalloc_ro_g
nonalloc_ro_g: .byte 1
nonalloc_ro_l: .byte 1
	.section ".nax","x",@progbits
	.globl nonalloc_exec_g
nonalloc_exec_g: .byte 1
nonalloc_exec_l: .byte 1
	.section ".line","",@progbits
	.globl line_g
line_g: .byte 1
line_l: .byte 1
	.section ".gdb_index","",@progbits
	.globl gdbindex_g
gdbindex_g: .byte 1
gdbindex_l: .byte 1
	.section ".gdb_index2","",@progbits
	.globl gdbindex2_g
gdbindex2_g: .byte 1
gdbindex2_l: .byte 1
	.section ".zdebug_x","",@progbits
	.globl zdebug_g
zdebug_g: .byte 1
zdebug_l: .byte 1
	.section ".gnu.linkonce.wi.x","",@progbits
	.globl linkoncewi_g
linkoncewi_g: .byte 1
linkoncewi_l: .byte 1
	.section ".debugfoo","",@progbits
	.globl debugfoo_g
debugfoo_g: .byte 1
debugfoo_l: .byte 1
	.section ".debug_w","w",@progbits
	.globl debugw_g
debugw_g: .byte 1
debugw_l: .byte 1
	.section ".debug_x_exec","x",@progbits
	.globl debugexec_g
debugexec_g: .byte 1
debugexec_l: .byte 1
	.section ".gnu.debuglto_.debug_x","",@progbits
	.globl debuglto_g
debuglto_g: .byte 1
debuglto_l: .byte 1
	.section "debug_nodot","",@progbits
	.globl nodot_g
nodot_g: .byte 1
nodot_l: .byte 1
	.section ".note.foo","a",@note
	.globl note_g
note_g: .byte 1
note_l: .byte 1
	.section ".init_array","aw",@init_array
	.globl initarray_g
initarray_g: .byte 1
initarray_l: .byte 1
	.section ".tdata","awT",@progbits
	.globl tdata_g
tdata_g: .byte 1
tdata_l: .byte 1
	.section ".tbss","awT",@nobits
	.globl tbss_g
tbss_g: .zero 1
tbss_l: .zero 1
	.section ".sdata","aw",@progbits
	.globl sdata_g
sdata_g: .byte 1
sdata_l: .byte 1
	.section ".sbss","aw",@nobits
	.globl sbss_g
sbss_g: .zero 1
sbss_l: .zero 1
	.section ".alloc_exec_w","awx",@progbits
	.globl awx_g
awx_g: .byte 1
awx_l: .byte 1
	.text
	.globl abs_g
	.set abs_g, 0x1234
	.set abs_l, 0x42
