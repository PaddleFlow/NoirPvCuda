; Paravirtualized System Call Handler

.code

PvSystemCall64 proc

	; Entrance of System Call.
	; Swap GS Base, then switch to kernel stack
	swapgs
	xchg rsp,qword ptr gs: [28h]
	; pushaq

	; popaq
PvSystemReturn64:
	; Exit of System Call.
	xchg rsp,qword ptr gs: [28h]
	swapgs
	sysret

PvSystemCall64 endp

end