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
; 					| Basic Free Memory			|
; 00000000`10000000 +---------------------------+
;					| Extended Free Memory		|
;					+---------------------------+

; The first page will starts with the PML4E.
pml4e_base:
times 256 dq 0		; User-Mode Virtual Memory Space
; Booting modules
dq booting_module_pdpte_base+3		; 1GiB should suffice for initial state.
times 235 dq 0
dq alloc_bitmap_pdpte_base+3		; 1GiB should suffice for initial state.
dq page_structure_pdpte_base+3		; 1GiB should suffice for initial state.
times 2 dq 0
dq init_system_pool_pdpte_base+3	; 1GiB should suffice for initial state.
times 15 dq 0

booting_module_pdpte_base:
dq booting_module_pde_base+3
times 511 dq 0

page_structure_pdpte_base:
dq page_structure_pde_base+3
times 511 dq 0

alloc_bitmap_pdpte_base:
times 511 dq 0
dq alloc_bitmap_pde_base+3

init_system_pool_pdpte_base:
dq init_system_pool_pde_base+3
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
times 449 dq 0

alloc_bitmap_pde_base:
dq alloc_bitmap_pte_base+3
times 511 dq 0

init_system_pool_pde_base:
dq init_system_pool_pte_base+3
times 511 dq 0

alloc_bitmap_pte_base:
dq 0x08200003
times 511 dq 0

init_system_pool_pte_base:
dq 0x08201003
times 511 dq 0