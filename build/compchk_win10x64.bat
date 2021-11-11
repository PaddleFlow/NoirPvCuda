@echo off
set ddkpath=T:\Program Files\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.28.29910
set path=%ddkpath%\bin\Hostx64\x64;T:\Program Files\Windows Kits\10\bin\10.0.22000.0\x64;%path%
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
cl ..\src\hv_stack\windows\pvmisc.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pvmisc.cod" /Fo"%objpath%\pvmisc.obj" /Fd"%objpath%\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\hv_stack\windows\pvcvm.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /I"..\header" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\pvcvm.cod" /Fo"%objpath%\pvcvm.obj" /Fd"%objpath%\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

cl ..\src\hv_stack\windows\main.c /I"%incpath%\shared" /I"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /I"..\header" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\main.cod" /Fo"%objpath%\main.obj" /Fd"%objpath%\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

rc /nologo /i"%incpath%\shared" /i"%incpath%\um" /I"%incpath%\ucrt" /I"%ddkpath%\include" /d"_AMD64_" /fo"%objpath%\version.res" /n ..\src\hv_stack\windows\version.rc

echo Compiling Paravirtualized Kernel...
cl ..\src\pv_kernel\kernel_entry.c /I"%incpath%\shared" /I"%incpath%\km" /I"%incpath%\km\crt" /I"%ddkpath%\include" /Zi /nologo /W3 /WX /Od /Oi /D"_AMD64_" /D"_M_AMD64" /D"_WIN64" /D"_UNICODE" /D"UNICODE" /Zc:wchar_t /std:c17 /FAcs /Fa"%objpath%\kernel_entry.cod" /Fo"%objpath%\kernel_entry.obj" /Fd"%objpath%\vc140.pdb" /GS- /Qspectre /TC /c /errorReport:queue

nasm ..\src\pv_kernel\page_base.asm -o "%binpath%\page_base.dat"

echo ============Start Linking============
link "%objpath%\main.obj" "%objpath%\pvcvm.obj" "%objpath%\pvmisc.obj" "%objpath%\version.res" /LIBPATH:"%libpath%\um\x64" /LIBPATH:"%libpath%\ucrt\x64" /LIBPATH:"%ddkpath%\lib\x64" "..\header\lib\x64\NoirCvmApi.lib" /NOLOGO /DEBUG /INCREMENTAL:NO /PDB:"%binpath%\NoirPvCuda.pdb" /OUT:"%binpath%\NoirPvCuda.exe" /SUBSYSTEM:CONSOLE /Machine:X64 /ERRORREPORT:QUEUE

link "%objpath%\kernel_entry.obj" /NOLOGO /DEBUG /INCREMENTAL:NO /PDB:"%binpath%\pv_kernel.pdb" /OUT:"%binpath%\pv_kernel.exe" /NODEFAULTLIB /SUBSYSTEM:NATIVE /ENTRY:"PvKernelEntry" /Machine:X64 /ERRORREPORT:QUEUE

if "%~1"=="/s" (echo Completed!) else (pause)