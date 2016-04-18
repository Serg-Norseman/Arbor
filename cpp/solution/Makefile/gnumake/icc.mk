# Intel C++ Compiler settings.

include winsdk.mk
# ICC on Windows (ICL) requires VC++ tool-chain
include msvc.mk

ifndef msvcroot
$(error Intel C++ Compiler requires VC++; check settings for MSVC tool-chain)
endif

# Path to compiler root directory (analog of $(ICPP_COMPILER16))
iccroot := /c/Program\ Files\ \(x86\)/IntelSWTools/compilers_and_libraries_2016.2.180/windows/
iccrootdos := C:\\Program Files (x86)\\IntelSWTools\\compilers_and_libraries_2016.2.180\\windows\\
# ICC paths
icccompilerx86-64 := $(iccroot)bin/intel64/icl.exe
icclinkerx86-64 := $(iccroot)bin/intel64/xilink.exe
iccrcx86-64 := $(windowssdkdir)bin/x64/rc.exe
icccompilerx86 := $(iccroot)bin/ia32/icl.exe
icclinkerx86 := $(iccroot)bin/ia32/xilink.exe
iccrcx86 := $(windowssdkdir)bin/x86/rc.exe
# Includes directories
#
# Because I have to use `-Qoption,cpp,--sys_include,{dir}` option,
# I have to use ms-dos like path (paths with back slashes as
# delimiters and double quotes to allow spaces in paths).
#
# For more information about `-Qoption` see this topic:
# https://software.intel.com/en-us/forums/intel-c-compiler/topic/624904.
iccincludex86-64 := \
-Qoption,cpp,--sys_include,"$(iccrootdos)compiler/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)compiler/include\intel64" \
-Qoption,cpp,--sys_include,"$(msvcrootdos)include" \
-Qoption,cpp,--sys_include,"$(msvcrootdos)atlmfc/include" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)ucrt" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)shared" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)um" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)winrt" \
-Qoption,cpp,--sys_include,"$(iccrootdos)ipp/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)mkl/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)tbb/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)daal/include"
iccincludex86 := \
-Qoption,cpp,--sys_include,"$(iccrootdos)compiler/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)compiler/include\ia32" \
-Qoption,cpp,--sys_include,"$(msvcrootdos)include" \
-Qoption,cpp,--sys_include,"$(msvcrootdos)atlmfc/include" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)ucrt" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)shared" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)um" \
-Qoption,cpp,--sys_include,"$(windowssdkdirdos)include/$(windowssdklibversion)winrt" \
-Qoption,cpp,--sys_include,"$(iccrootdos)ipp/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)mkl/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)tbb/include" \
-Qoption,cpp,--sys_include,"$(iccrootdos)daal/include"
# Linker's LIBPATHs
icclibpathx86-64 :=\
-LIBPATH:$(msvcroot)lib/amd64 \
-LIBPATH:$(msvcroot)atlmfc/lib/amd64 \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)ucrt/x64 \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)um/x64 \
-LIBPATH:$(iccroot)compiler/lib/intel64 \
-LIBPATH:$(iccroot)compiler/lib/intel64_win
icclibpathx86 :=\
-LIBPATH:$(msvcroot)lib \
-LIBPATH:$(msvcroot)atlmfc/lib \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)ucrt/x86 \
-LIBPATH:$(windowssdkdir)lib/$(windowssdklibversion)um/x86 \
-LIBPATH:$(iccroot)compiler/lib/ia32 \
-LIBPATH:$(iccroot)compiler/lib/ia32_win \
