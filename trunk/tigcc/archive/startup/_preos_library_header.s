| Copyright (C) 2002-2003 Sebastian Reichelt.
| Copyright (C) 2003 Kevin Kofler.
| See License.txt for licensing conditions.

| PreOs ("Kernel v6") Library Header

	.xdef __kernel_library_header_AND___preos_headers,__kernel_entry_point,__kernel_fixed_header_end,__bss_handle

| Fixed kernel program header.
.section _stl1, "d"
__kernel_library_header:
| This is the kernel program entry point.
__kernel_entry_point:
| This is not actually a real bsr: The library stub will clear the return
| address from the stack. This may also be a bra.w, but maybe some kernels
| rely on the bsr.w.
	bsr.w __kernel_library_stub | same dummy stub as for the old format
__kernel_library_signature:
	.ascii "68kS"
__kernel_format_version:
	.byte 1 | "Kernel v6"
__kernel_reloc_count_pos:
	.byte 0
__kernel_comment_pos:
	.word _comment-__kernel_entry_point
__kernel_main_pos:
	.word 0
__kernel_exit_pos:
	.word _exit-__kernel_entry_point
__kernel_flags_pos:
	.byte __ld_file_version
	.byte __ld_kernel_flags
__kernel_bss_handle_pos:
__bss_handle:
	.word 0
__kernel_reloc_table_pos: | compressed import and relocation tables
	.word __ld_insert_preos_compressed_tables-__kernel_entry_point
__kernel_export_table_pos:
	.word __ld_kernel_export_table-__kernel_entry_point
__kernel_extra_pos:
	.word _extraram-__kernel_entry_point
__kernel_fixed_header_end:

| The format of the compressed import and relocation tables (autogenerated by
| __ld_append) is as follows (see below for the explanation of "Index" and
| "Compressed relocation table):
|
| 1. Kernel library import table.
| The format for the library table is as follows:
| 1 byte: number of imported libraries
| For each library...
|   8 bytes: name
|   1 byte: 0
|   1 byte: minimum version
| For each library...
|   Index: function import count - 1
|   For each imported function...
|     Index: function index in the library (relative to the previous one)
|     Compressed relocation table
|
| 2. ROM call import table.
| The format for the ROM call table is as follows:
| Index: ROM call count
| For each ROM call used...
|   Index: ROM call number (relative to the previous one)
|   Compressed relocation table
|
| 3. RAM call import table.
| The format for the RAM call table is as follows:
| Index: RAM call count
| For each RAM call used...
|   Index: RAM call number (relative to the previous one)
|     Bit 14: extra RAM address
|     Bit 15: 2-byte references (instead of 4)
|   Compressed relocation table
|
| 4. Relocation table.
| The format for the reloc table is as follows:
| Compressed relocation table
|
| 5. BSS relocation table.
| The format for the BSS table is as follows:
| 2 bytes: (BSS size)/4
| Compressed relocation table
|
| where "Index" means a number "index" represented as follows ("last_index" is
| either the previous one or -1 depending on the context):
| 0 <= index - (last_index + 1) <= 253:
|   1 byte: index - (last_index + 1)
| 254+0 <= index - (last_index + 1) <= 254+255:
|   1 byte: 254
|   1 byte: (index - (last_index + 1)) - 254
| index - (last_index + 1) > 254+255:
|   1 byte: 255
|   2 bytes: index 
|
| and "Compressed relocation table" means a relocation table compressed as
| follows:
/*
(NOTE: all numbers below are hexadecimal!):
The offset starts at 24-4 (where 4 is the smallest encodable difference)
00: end of list
01-7F: add 004-100 (2(value+1)) to the previous offset to get the new one
80-BF: add 04-22 (2(nibble_value+2)). (Note that 01-7F can be used for
       that purpose as well if you are a lazy linker author. :-) But the
       nibble-encoding saves some space, so you'd better use it.)
       The bytes have the following values (the most significant nibble is
       the first one):
       Byte 1: n+6 (2+6=8 .. 5+6=B), first data nibble
       Byte 2: 2 data nibbles
       ...
       Byte n: 2 data nibbles
       Note that n = ([total number of data nibbles] + 1) / 2. It follows
       that the total number of data nibbles must be odd and >=3. Use
       byte-encoding (01-7F) otherwise (for all data if the number is <=2,
       for the last data nibble only if it is even and >=4). It also must
       not exceed 9 (5*2-1), in which case you need to split it up.
C0-FF: the byte starts a full word: 0xC000+difference
       add 2(difference+0x81) to the previous offset to get the new one
       FFFF is special: it adds 0x80FC to the offset, but the computation
                        does not stop here, instead another delta is added

(NOTE: numbers below are decimal unless prefixed by "0x"!)
In pseudo-code, this means:
offset=36-4; // 0x24 - (smallest encodable offset)
for each (byte) {
  switch (byte) {
    case 0: stop;
    case 0x01..0x7f:
      offset+=2*(byte+1);
      dump(offset);
      break;
    case 0x80..0xbf:
      n=(byte>>4)-6;
      offset+=2*((byte&7)+2);
      dump(offset);
      for(i=1;i<n;i++) {
        offset+=2*((byte>>4)+2);
        dump(offset);
        offset+=2*((byte&7)+2);
        dump(offset);
      }
    case 0xc0..0xff:
      complete the word;
      if (word==0xFFFF) {
        offset+=0x80FC;
      } else {
        offset+=2*(word-(0xC000-0x81));
        dump(offset);
      }
   }
}
*/
