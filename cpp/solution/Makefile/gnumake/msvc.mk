# MSFT VC++ Compiler settings.

# Path to compiler root directory (analog of $(VCINSTALLDIR))
ifndef msvcroot
msvcroot := /c/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio\ 14.0/VC/
endif
ifndef msvcrootdos
msvcrootdos := C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\
endif

# MSVC paths
msvcbinrootx86-64 := $(msvcroot)bin/amd64/
msvccompilerx86-64 := $(msvcbinrootx86-64)cl.exe
msvclinkerx86-64 := $(msvcbinrootx86-64)link.exe
msvcrcx86-64 := $(windowssdkdir)bin/x64/rc.exe
prefastplugindosx86-64 := "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\LocalESPC.dll"
msvcbinrootx86 := $(msvcroot)bin/
msvccompilerx86 := $(msvcbinrootx86)cl.exe
msvclinkerx86 := $(msvcbinrootx86)link.exe
msvcrcx86 := $(windowssdkdir)bin/x86/rc.exe
prefastplugindosx86 := "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\LocalESPC.dll"
# Includes directories
msvcincludex86-64 := \
-I$(msvcroot)include \
-I$(msvcroot)atlmfc/include \
-I$(windowssdkdir)include/$(windowssdklibversion)ucrt \
-I$(windowssdkdir)include/$(windowssdklibversion)shared \
-I$(windowssdkdir)include/$(windowssdklibversion)um \
-I$(windowssdkdir)include/$(windowssdklibversion)winrt
msvcincludex86 := $(msvcincludex86-64)
# Linker's LIBPATHs
msvclibpathx86-64 :=\
-LIBPATH:$(msvcroot)lib/amd64 \
-LIBPATH:$(msvcroot)atlmfc/lib/amd64 \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)ucrt/x64 \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)um/x64
msvclibpathx86 :=\
-LIBPATH:$(msvcroot)lib \
-LIBPATH:$(msvcroot)atlmfc/lib \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)ucrt/x86 \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)um/x86
