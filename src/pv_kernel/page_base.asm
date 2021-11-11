bits 64
org 0x200000
; This file will be compiled into a binary file that includes initial paging structure state.

; Paravirtualized Kernel Virtual Memory Layout
; FFFF8000`00000000 +---------------------------+
;					| Booting Modules			|
; FFFFF680`00000000 +---------------------------+
; 					| Paging Structures			|
; FFFFF7FF`FFFFF000 +---------------------------+
;					| User-Kernel Shared Region	|
; FFFFF800`00000000 +---------------------------+
;					| System Memory Pool		|
; FFFFFA80`00000000 +---------------------------+
;					| Swappable Memory Pool		|
; FFFFFFFF`80000000 +---------------------------+
;					| Hardware Abstract Level	|
; FFFFFFFF`FFFFFFFF +---------------------------+

; Initial Paravirtualized Kernel Physical Memory Layout
; 00000000`00000000 +---------------------------+
;					| System Critical Range		|
; 00000000`00200000	+---------------------------+
;					| Paging Structures			|
; 00000000`08000000 +---------------------------+
; 					| Free Memory				|
;					+---------------------------+

; The first page will starts with the PML4E.
pml4e_base:
times 256 dq 0		; User-Mode Virtual Memory Space
; Booting modules
dq booting_module_pdpte_base+3	; 1GiB should suffice for initial state.
times 236 dq 0
dq page_structure_pdpte_base+3	; 1GiB should suffice for initial state.
times 18 dq 0

booting_module_pdpte_base:
dq booting_module_pde_base+3
times 511 dq 0

page_structure_pdpte_base:
dq page_structure_pde_base+3
times 511 dq 0

booting_module_pde_base:
dq 0x00000083,0x08000083
times 510 dq 0

page_structure_pde_base:		; Map as 2MiB Large Pages.
%assign i 0x200083
%rep 63
dq i
%assign i i+0x200000
%endrep
; dq 0x00200087,0x00400087,0x00600087,0x00800087,0x00A00087,0x00C00087,0x00E00087,0x01000087
; dq 0x01200087,0x01400087,0x01600087,0x01800087,0x01A00087,0x01C00087,0x01E00087,0x02000087
; dq 0x02200087,0x02400087,0x02600087,0x02800087,0x02A00087,0x02C00087,0x02E00087,0x03000087
; dq 0x03200087,0x03400087,0x03600087,0x03800087,0x03A00087,0x03C00087,0x03E00087,0x04000087
; dq 0x04200087,0x04400087,0x04600087,0x04800087,0x04A00087,0x04C00087,0x04E00087,0x05000087
; dq 0x05200087,0x05400087,0x05600087,0x05800087,0x05A00087,0x05C00087,0x05E00087,0x06000087
; dq 0x06200087,0x06400087,0x06600087,0x06800087,0x06A00087,0x06C00087,0x06E00087,0x07000087
; dq 0x07200087,0x07400087,0x07600087,0x07800087,0x07A00087,0x07C00087,0x07E00087
times 449 dq 0