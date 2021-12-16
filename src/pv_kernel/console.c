#include <pvdef.h>
#include <stdarg.h>
#include <pvbdk.h>

ULONG PvPrintStdOut(IN PSTR Format,...)
{
	va_list ArgList;
	va_start(ArgList,Format);
	char Buffer[512];
	ULONG Length=RtlVPrintStringA(Buffer,sizeof(Buffer),Format,ArgList);
	va_end(ArgList);
	__outbytestring(ConsoleOutputPort,Buffer,Length);
	return Length;
}