@echo off
set ddkpath=T:\Program Files\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.28.29910
set path=%ddkpath%\bin\Hostx64\x64;T:\Program Files\Windows Kits\10\bin\10.0.22000.0\x64;%path%
set dbgpath=T:\Program Files\Windows Kits\10\Debuggers
set incpath=T:\Program Files\Windows Kits\10\Include\10.0.22000.0
set libpath=T:\Program Files\Windows Kits\10\Lib\10.0.22000.0
set binpath=..\bin\compchk_win10x64
set objpath=..\bin\compchk_win10x64\Intermediate

title Compiling NoirPvCuda, Checked Build, 64-Bit Windows (AMD64 Architecture)
echo Project: NoirPvCuda
echo Platform: 64-Bit Windows
echo Preset: Debug/Checked Build
echo Powered by zero.tangptr@gmail.com
echo Copyright (c) 2021, zero.tangptr@gmail.com. All Rights Reserved.
if "%~1"=="/s" (echo DO-NOT-PAUSE is activated!) else (pause)

echo ============Start Compiling============
echo Compiling User Hypervisor for Paravirtualization....
cl ..\src\hv_stack\windows\pvmisc.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\hv_stack\pvmisc.cod" /Fo"%objpath%\hv_stack\pvmisc.obj" /Fd"%objpath%\hv_stack\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\hv_stack\windows\debug_agent.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\hv_stack\debug_agent.cod" /Fo"%objpath%\hv_stack\debug_agent.obj" /Fd"%objpath%\hv_stack\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\hv_stack\windows\timer.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /I"..\header" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\hv_stack\timer.cod" /Fo"%objpath%\hv_stack\timer.obj" /Fd"%objpath%\hv_stack\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\hv_stack\windows\virt_fileio.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /I"..\header" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\hv_stack\virt_fileio.cod" /Fo"%objpath%\hv_stack\virt_fileio.obj" /Fd"%objpath%\hv_stack\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\hv_stack\windows\pvcvm.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /I"..\header" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\hv_stack\pvcvm.cod" /Fo"%objpath%\hv_stack\pvcvm.obj" /Fd"%objpath%\hv_stack\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\hv_stack\windows\main.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /I"..\header" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\hv_stack\main.cod" /Fo"%objpath%\hv_stack\main.obj" /Fd"%objpath%\hv_stack\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

rc /nologo /i"%incpath%\shared" /i"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /d"_AMD64_" /fo"%objpath%\hv_stack\version.res" /n ..\src\hv_stack\windows\version.rc

echo Compiling Paravirtualized Kernel...
cl ..\src\pv_kernel\kernel_entry.c /I"..\src\pv_kernel\inc" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_KERNEL_INIT" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_kernel\kernel_entry.cod" /Fo"%objpath%\pv_kernel\kernel_entry.obj" /Fd"%objpath%\pv_kernel\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

ml64 /W3 /WX /Zf /Zd /Fo"%objpath%\pv_kernel\interrupt.obj" /c /nologo ..\src\pv_kernel\interrupt.asm

ml64 /W3 /WX /Zf /Zd /Fo"%objpath%\pv_kernel\syscall.obj" /c /nologo ..\src\pv_kernel\syscall.asm

cl ..\src\pv_kernel\io\pv_fileio.c /I"..\src\pv_kernel\inc" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_BITMAP" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_kernel\io\pv_fileio.cod" /Fo"%objpath%\pv_kernel\io\pv_fileio.obj" /Fd"%objpath%\pv_kernel\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\pv_kernel\ps\psmgr.c /I"..\src\pv_kernel\inc" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_BITMAP" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_kernel\ps\psmgr.cod" /Fo"%objpath%\pv_kernel\ps\psmgr.obj" /Fd"%objpath%\pv_kernel\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\pv_kernel\rtl\bitmap.c /I"..\src\pv_kernel\inc" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_BITMAP" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_kernel\rtl\bitmap.cod" /Fo"%objpath%\pv_kernel\rtl\bitmap.obj" /Fd"%objpath%\pv_kernel\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

ml64 /W3 /WX /Zf /Zd /Fo"%objpath%\pv_kernel\rtl\context.obj" /c /nologo ..\src\pv_kernel\rtl\context.asm

cl ..\src\pv_kernel\rtl\list.c /I"..\src\pv_kernel\inc" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_BITMAP" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_kernel\rtl\list.cod" /Fo"%objpath%\pv_kernel\rtl\list.obj" /Fd"%objpath%\pv_kernel\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\pv_kernel\rtl\memop.c /I"..\src\pv_kernel\inc" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_BITMAP" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_kernel\rtl\memop.cod" /Fo"%objpath%\pv_kernel\rtl\memop.obj" /Fd"%objpath%\pv_kernel\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\pv_kernel\mm\memmgr.c /I"..\src\pv_kernel\inc" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_MEMMGR" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_kernel\mm\memmgr.cod" /Fo"%objpath%\pv_kernel\mm\memmgr.obj" /Fd"%objpath%\pv_kernel\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

nasm ..\src\pv_kernel\page_base.asm -o "%binpath%\page_base.dat"

echo Compiling Paravirtualized Console for Guest...

cl ..\src\pv_console\windows\main.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pv_console\main.cod" /Fo"%objpath%\pv_console\main.obj" /Fd"%objpath%\pv_console\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

echo ============Start Linking============
link "%objpath%\hv_stack\main.obj" "%objpath%\hv_stack\pvcvm.obj" "%objpath%\hv_stack\pvmisc.obj" "%objpath%\hv_stack\debug_agent.obj" "%objpath%\hv_stack\timer.obj" "%objpath%\hv_stack\virt_fileio.obj" "%objpath%\hv_stack\version.res" /LIBPATH:"%libpath%\um\x64" /LIBPATH:"%libpath%\ucrt\x64" /LIBPATH:"%ddkpath%\lib\x64" /LIBPATH:"%dbgpath%\lib\x64" "ntdll.lib" "dbghelp.lib" "..\header\lib\x64\NoirCvmApi.lib" /NOLOGO /DEBUG /INCREMENTAL:NO /PDB:"%binpath%\NoirPvCuda.pdb" /OUT:"%binpath%\NoirPvCuda.exe" /SUBSYSTEM:CONSOLE /Machine:X64 /ERRORREPORT:QUEUE

link "%objpath%\pv_kernel\kernel_entry.obj" "%objpath%\pv_kernel\interrupt.obj" "%objpath%\pv_kernel\io\pv_fileio.obj" "%objpath%\pv_kernel\ps\psmgr.obj" "%objpath%\pv_kernel\mm\memmgr.obj" "%objpath%\pv_kernel\rtl\bitmap.obj" "%objpath%\pv_kernel\rtl\list.obj" "%objpath%\pv_kernel\rtl\memop.obj" "%objpath%\pv_kernel\syscall.obj" /BASE:0xFFFF800000200000 /NOLOGO /DEBUG /INCREMENTAL:NO /PDB:"%binpath%\pv_kernel.pdb" /OUT:"%binpath%\pv_kernel.exe" /NODEFAULTLIB /SUBSYSTEM:NATIVE /ENTRY:"PvKernelEntry" /Machine:X64 /ERRORREPORT:QUEUE

link "%objpath%\pv_console\main.obj" /LIBPATH:"%libpath%\um\x64" /LIBPATH:"%libpath%\ucrt\x64" /LIBPATH:"%ddkpath%\lib\x64" /NOLOGO /DEBUG /INCREMENTAL:NO /PDB:"%binpath%\pv_console.pdb" /OUT:"%binpath%\pv_console.exe" /SUBSYSTEM:CONSOLE /Machine:X64 /ERRORREPORT:QUEUE

if "%~1"=="/s" (echo Completed!) else (pause)