@echo off
IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) ELSE (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
set compilerflags=/Od /Zi /EHsc /MT /GL /std:c++latest
set includedirs=/I../../include 
set linkerflags=/LIBPATH:../../libs/win glfw3.lib assimp-vc142-mt.lib zlib.lib IrrXML.lib gdi32.lib user32.lib Shell32.lib gluit.lib trimesh.lib
cl.exe %compilerflags% %includedirs% ../../include/glad/glad.c ../../include/imgui/*.cpp main.cpp /Fe:myproject.exe /link %linkerflags%
