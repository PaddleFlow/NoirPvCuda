// Paravirtualized Cuda Paging

/*
	Paravirtualized Cuda Virtual Memory Layout
  
	00000000`00000000	+---------------------------------------+
						| User-Mode Virtual Memory Space		|
	00007FFF`FFFFF000	+---------------------------------------+
						| User-Kernel Shared Region				|
	00007FFF`FFFFFFFF	+---------------------------------------+

	FFFF8000`00000000	+---------------------------------------+
						| Booting & First-Party Modules			|
	FFFFF680`00000000	+---------------------------------------+
						| Paging Structures						|
	FFFFF7FF`FFFFF000	+---------------------------------------+
						| User-Kernel Shared Region				|
	FFFFF800`00000000	+---------------------------------------+
						| System Memory Pool					|
	FFFFFA80`00000000	+---------------------------------------+
						| Swappable Memory Pool					|
	FFFFFFFF`80000000	+---------------------------------------+
						| Hardware Abstract Level				|
	FFFFFFFF`FFFFFFFF	+---------------------------------------+
*/

/*
	Paravirtualized Cuda Physical Memory Layout

	00000000`00000000	+---------------------------------------+
						| System Critical Range					|
						| 2 MiB Size							|
	00000000`00200000	+---------------------------------------+
						| Paging Structures						|
						| 126 MiB Size							|
	00000000`08000000	+---------------------------------------+
						| Free Memory							|
						+---------------------------------------+
*/