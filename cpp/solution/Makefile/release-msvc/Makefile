# Makefile to build `ArborGVT` with MSVC.
#
# This makefile is for MSFT nmake tool only!
#
# This makefile is intended to be used under "shell" pre-configurated by
# MSFT Visual Studio installation, under "Developer Command Prompt for
# VS2015", for example. This makefile always calls necessary tools (i.e.
# it calls native `cl` for x86-64 platfrom when builds x86-64 releases).

SHELL = cmd

.SUFFIXES:
.SUFFIXES: .cpp .obj

arborgvt = arborgvt

all: dtsample_x86-64 dtsample_x86

toolchain = msvc
projectname = $(arborgvt)
projectext = .dll
platform = x86-64
srcdir = ../../../source/$(projectname)/
outdir = ../../../build/release-$(toolchain)-$(platform)/
outdirdos = ..^\..^\..^\build^\release-$(toolchain)-$(platform)^\
objdir = $(outdir)obj/$(projectname)/
objdirdos = $(outdirdos)obj^\$(projectname)^\
objects = \
$(objdir)arbor.obj \
$(objdir)avisimpl.obj \
$(objdir)barnhut.obj \
$(objdir)bhutquad.obj \
$(objdir)dllmain.obj \
$(objdir)graph.obj \
$(objdir)graphwnd.obj \
$(objdir)miscutil.obj \
$(objdir)newdel.obj \
$(objdir)stladdon.obj \
$(objdir)strgutil.obj \
$(objdir)vector.obj \
$(objdir)wi.obj
resources = $(objdir)$(projectname).res
compiler = "$(VCINSTALLDIR)bin/amd64/cl.exe"
compilerflags = \
/Bv \
/guard:cf /sdl \
/Gd \
/GL /Gw /Gy /GR- \
/Gm- /GF /GS /MP \
/fp:fast /favor:INTEL64 \
/WX /W4 /analyze:WX- /analyze /analyze:plugin"$(VCINSTALLDIR)bin/amd64/LocalESPC.dll" \
/EHsc /MD /Zc:wchar_t /Zc:forScope /Zc:inline /Zc:rvalueCast /Zc:inline /Zc:throwingNew \
/I$(srcdir) \
/Fo$(objdir) \
/Oi /O2 /Ob2 /Ot \
/TP \
/DWIN32 \
/DPLATFORM_WIN32 /DNTDDI_VERSION=NTDDI_WINTHRESHOLD /D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD /DWINVER=0x0A00 /DWIN32_LEAN_AND_MEAN \
/DNDEBUG /D_WIN64 /D_M_X64 /D_M_AMD64 /D_AMD64_ /U_M_IX86 /U_M_IA64 /U_X86_ \
/D_WINDOWS \
/DARBORGVT_EXPORTS /D_USRDLL /D_WINDLL \
/DCODE_ANALYSIS \
/D_UNICODE /DUNICODE
linker = "$(VCINSTALLDIR)bin/amd64/link.exe"
linkerflags = \
/DLL \
/LIBPATH:"$(VCINSTALLDIR)LIB/amd64" \
/LIBPATH:"$(VCINSTALLDIR)ATLMFC/LIB/amd64" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)ucrt/x64" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)um/x64" \
kernel32.lib user32.lib shell32.lib \
/DEF:$(srcdir)exports/$(arborgvt).def \
/OPT:REF /OPT:ICF /LTCG:incremental /DYNAMICBASE /NXCOMPAT \
/MACHINE:X64 /SUBSYSTEM:WINDOWS,6.0 \
/INCREMENTAL:NO /RELEASE /MANIFEST:NO \
/OUT:$(outdir)$(projectname)$(projectext)
rc = "$(WINDOWSSDKDIR)bin/x64/rc.exe"
rcflags = /I$(srcdir) /Fo$(resources) /C 65001 /D_WIN64

makeobjdir$(projectname)$(platform):
	@if not exist $(objdirdos) (mkdir $(objdirdos))

arborgvt_x86-64: makeobjdir$(projectname)$(platform) $(objects) $(resources)
	$(linker) $(linkerflags) $(objects) $(resources)
	@echo $(projectname)$(projectext): build completed.

$(objdir)arbor.obj: $(srcdir)dlllayer/arbor.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)avisimpl.obj: $(srcdir)ui/nowindow/avisimpl/avisimpl.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)barnhut.obj: $(srcdir)barnhut/barnhut.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)bhutquad.obj: $(srcdir)barnhut/bhutquad.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)dllmain.obj: $(srcdir)dlllayer/dllmain.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)graph.obj: $(srcdir)graph/graph.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)graphwnd.obj: $(srcdir)ui/window/child/onscreen/graphwnd.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)miscutil.obj: $(srcdir)service/miscutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)newdel.obj: $(srcdir)service/newdel.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)stladdon.obj: $(srcdir)service/stladdon.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)strgutil.obj: $(srcdir)service/strgutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)vector.obj: $(srcdir)graph/vector.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)wi.obj: $(srcdir)ui/window/wi.cpp
	$(compiler) /c $(compilerflags) $?

$(srcdir)dlllayer/arbor.cpp: $(srcdir)dlllayer/arbor.h $(srcdir)ui/nowindow/avisimpl/avisimpl.h

$(srcdir)ui/nowindow/avisimpl/avisimpl.cpp: $(srcdir)ui/nowindow/avisimpl/avisimpl.h

$(srcdir)barnhut/barnhut.cpp: $(srcdir)barnhut/barnhut.h

$(srcdir)barnhut/bhutquad.cpp: $(srcdir)barnhut/bhutquad.h $(srcdir)graph/vector.h $(srcdir)service/sse.h

$(srcdir)dlllayer/dllmain.cpp: $(srcdir)sdkver.h

$(srcdir)graph/graph.cpp: $(srcdir)barnhut/barnhut.h $(srcdir)graph/graph.h

$(srcdir)ui/window/child/onscreen/graphwnd.cpp: $(srcdir)service/sse.h $(srcdir)ui/window/child/onscreen/graphwnd.h

$(srcdir)service/miscutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)service/newdel.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/stladdon.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/strgutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/strgutil.h

$(srcdir)graph/vector.cpp: $(srcdir)graph/vector.h $(srcdir)service/sse.h

$(srcdir)service/winapi/wam/animatio.h: $(srcdir)ns/wapi.h $(srcdir)service/com/comptr.h

$(srcdir)ui/window/wi.cpp: $(srcdir)service/functype.h $(srcdir)service/strgutil.h $(srcdir)service/winapi/theme.h $(srcdir)ui/window/child/onscreen/graphwnd.h $(srcdir)ui/window/wi.h

$(srcdir)dlllayer/arbor.h: $(srcdir)dlllayer/arborvis.h

$(srcdir)ns/arbor.h:

$(srcdir)dlllayer/arborvis.h: $(srcdir)graph/edge.h $(srcdir)graph/vertex.h $(srcdir)sdkver.h

$(srcdir)ns/atladd.h:

$(srcdir)ui/nowindow/avisimpl/avisimpl.h: $(srcdir)dlllayer/arborvis.h $(srcdir)graph/edge.h $(srcdir)graph/vertex.h $(srcdir)service/com/impl.h $(srcdir)ui/window/child/onscreen/graphwnd.h

$(srcdir)barnhut/barnhut.h: $(srcdir)barnhut/bhutquad.h $(srcdir)graph/vertex.h $(srcdir)ns/barnhut.h

$(srcdir)ns/barnhut.h:

$(srcdir)barnhut/bhutquad.h: $(srcdir)graph/vector.h $(srcdir)graph/vertex.h $(srcdir)ns/barnhut.h $(srcdir)service/stladdon.h

$(srcdir)service/winapi/chkerror.h: $(srcdir)ns/wapi.h

$(srcdir)service/com/comptr.h: $(srcdir)ns/atladd.h

$(srcdir)ui/window/child/cwi.h: $(srcdir)ns/atladd.h $(srcdir)ui/window/wi.h

$(srcdir)ui/nowindow/graph/draw.h: $(srcdir)graph/edge.h $(srcdir)graph/vertex.h $(srcdir)service/com/comptr.h $(srcdir)service/sse.h $(srcdir)service/stladdon.h

$(srcdir)service/winapi/directx/dx.h: $(srcdir)ns/dxu.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/functype.h

$(srcdir)ns/dxu.h:

$(srcdir)graph/edge.h: $(srcdir)graph/vertex.h $(srcdir)ns/arbor.h $(srcdir)service/stladdon.h

$(srcdir)service/functype.h:

$(srcdir)graph/graph.h: $(srcdir)ns/arbor.h $(srcdir)graph/edge.h $(srcdir)graph/vector.h $(srcdir)graph/vertex.h $(srcdir)service/sse.h $(srcdir)service/stladdon.h $(srcdir)service/winapi/srwlock.h

$(srcdir)ui/window/child/onscreen/graphwnd.h: $(srcdir)graph/graph.h $(srcdir)service/com/comptr.h $(srcdir)service/stladdon.h $(srcdir)ui/nowindow/graph/draw.h $(srcdir)ui/window/child/cwi.h

$(srcdir)service/winapi/heap.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/com/impl.h: $(srcdir)ns/atladd.h

$(srcdir)ns/miscutil.h:

$(srcdir)service/miscutil.h: $(srcdir)ns/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)sdkver.h:

$(srcdir)service/winapi/srwlock.h: $(srcdir)ns/wapi.h $(srcdir)service/functype.h

$(srcdir)service/sse.h:

$(srcdir)ns/stladd.h:

$(srcdir)service/stladdon.h: $(srcdir)ns/arbor.h $(srcdir)ns/stladd.h $(srcdir)service/winapi/heap.h

$(srcdir)service/strgutil.h: $(srcdir)service/stladdon.h

$(srcdir)service/winapi/theme.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/uh.h: $(srcdir)ns/wapi.h

$(srcdir)graph/vector.h: $(srcdir)ns/arbor.h

$(srcdir)graph/vertex.h: $(srcdir)graph/vector.h $(srcdir)ns/arbor.h $(srcdir)service/sse.h $(srcdir)service/stladdon.h

$(srcdir)ns/wapi.h:

$(srcdir)ui/window/wi.h: $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/directx/dx.h $(srcdir)service/winapi/wam/animatio.h

$(resources): $(srcdir)resource/$(projectname).rc
	$(rc) $(rcflags) $**

platform = x86
outdir = ../../../build/release-$(toolchain)-$(platform)/
outdirdos = ..^\..^\..^\build^\release-$(toolchain)-$(platform)^\
objdir = $(outdir)obj/$(projectname)/
objdirdos = $(outdirdos)obj^\$(projectname)^\
objects = \
$(objdir)arbor.obj \
$(objdir)avisimpl.obj \
$(objdir)barnhut.obj \
$(objdir)bhutquad.obj \
$(objdir)dllmain.obj \
$(objdir)graph.obj \
$(objdir)graphwnd.obj \
$(objdir)miscutil.obj \
$(objdir)newdel.obj \
$(objdir)stladdon.obj \
$(objdir)strgutil.obj \
$(objdir)vector.obj \
$(objdir)wi.obj
resources = $(objdir)$(projectname).res
compiler = "$(VCINSTALLDIR)bin/cl.exe"
compilerflags = \
/Bv \
/guard:cf /sdl \
/Gd \
/GL /Gw /Gy /GR- \
/Gm- /GF /GS /MP \
/fp:fast /arch:SSE2 \
/WX /W4 /analyze:WX- /analyze /analyze:plugin"$(VCINSTALLDIR)bin/LocalESPC.dll" \
/EHsc /MD /Zc:wchar_t /Zc:forScope /Zc:inline /Zc:rvalueCast /Zc:inline /Zc:throwingNew \
/I$(srcdir) \
/Fo$(objdir) \
/Oi /O2 /Ob2 /Ot \
/TP \
/DWIN32 \
/DPLATFORM_WIN32 /DNTDDI_VERSION=NTDDI_WINTHRESHOLD /D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD /DWINVER=0x0A00 /DWIN32_LEAN_AND_MEAN \
/DNDEBUG /D_X86_ \
/D_WINDOWS \
/DARBORGVT_EXPORTS /D_USRDLL /D_WINDLL \
/DCODE_ANALYSIS \
/D_UNICODE /DUNICODE
linker = "$(VCINSTALLDIR)bin/link.exe"
linkerflags = \
/DLL \
/LIBPATH:"$(VCINSTALLDIR)LIB" \
/LIBPATH:"$(VCINSTALLDIR)ATLMFC/LIB" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)ucrt/x86" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)um/x86" \
kernel32.lib user32.lib shell32.lib \
/DEF:$(srcdir)exports/$(arborgvt).def \
/OPT:REF /OPT:ICF /LTCG:incremental /DYNAMICBASE /NXCOMPAT \
/MACHINE:X86 /SAFESEH /SUBSYSTEM:WINDOWS,6.0 \
/INCREMENTAL:NO /RELEASE /MANIFEST:NO \
/OUT:$(outdir)$(projectname)$(projectext)
rc = "$(WINDOWSSDKDIR)bin/x86/rc.exe"
rcflags = /I$(srcdir) /Fo$(resources) /C 65001 /D_WIN64

makeobjdir$(projectname)$(platform):
	@if not exist $(objdirdos) (mkdir $(objdirdos))

arborgvt_x86: makeobjdir$(projectname)$(platform) $(objects) $(resources)
	$(linker) $(linkerflags) $(objects) $(resources)
	@echo $(projectname)$(projectext): build completed.

$(objdir)arbor.obj: $(srcdir)dlllayer/arbor.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)avisimpl.obj: $(srcdir)ui/nowindow/avisimpl/avisimpl.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)barnhut.obj: $(srcdir)barnhut/barnhut.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)bhutquad.obj: $(srcdir)barnhut/bhutquad.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)dllmain.obj: $(srcdir)dlllayer/dllmain.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)graph.obj: $(srcdir)graph/graph.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)graphwnd.obj: $(srcdir)ui/window/child/onscreen/graphwnd.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)miscutil.obj: $(srcdir)service/miscutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)newdel.obj: $(srcdir)service/newdel.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)stladdon.obj: $(srcdir)service/stladdon.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)strgutil.obj: $(srcdir)service/strgutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)vector.obj: $(srcdir)graph/vector.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)wi.obj: $(srcdir)ui/window/wi.cpp
	$(compiler) /c $(compilerflags) $?

$(srcdir)dlllayer/arbor.cpp: $(srcdir)dlllayer/arbor.h $(srcdir)ui/nowindow/avisimpl/avisimpl.h

$(srcdir)ui/nowindow/avisimpl/avisimpl.cpp: $(srcdir)ui/nowindow/avisimpl/avisimpl.h

$(srcdir)barnhut/barnhut.cpp: $(srcdir)barnhut/barnhut.h

$(srcdir)barnhut/bhutquad.cpp: $(srcdir)barnhut/bhutquad.h $(srcdir)graph/vector.h $(srcdir)service/sse.h

$(srcdir)dlllayer/dllmain.cpp: $(srcdir)sdkver.h

$(srcdir)graph/graph.cpp: $(srcdir)barnhut/barnhut.h $(srcdir)graph/graph.h

$(srcdir)ui/window/child/onscreen/graphwnd.cpp: $(srcdir)service/sse.h $(srcdir)ui/window/child/onscreen/graphwnd.h

$(srcdir)service/miscutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)service/newdel.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/stladdon.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/strgutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/strgutil.h

$(srcdir)graph/vector.cpp: $(srcdir)graph/vector.h $(srcdir)service/sse.h

$(srcdir)service/winapi/wam/animatio.h: $(srcdir)ns/wapi.h $(srcdir)service/com/comptr.h

$(srcdir)dlllayer/arbor.h: $(srcdir)dlllayer/arborvis.h

$(srcdir)ns/arbor.h:

$(srcdir)dlllayer/arborvis.h: $(srcdir)graph/edge.h $(srcdir)graph/vertex.h $(srcdir)sdkver.h

$(srcdir)ns/atladd.h:

$(srcdir)ui/nowindow/avisimpl/avisimpl.h: $(srcdir)dlllayer/arborvis.h $(srcdir)graph/edge.h $(srcdir)graph/vertex.h $(srcdir)service/com/impl.h $(srcdir)ui/window/child/onscreen/graphwnd.h

$(srcdir)barnhut/barnhut.h: $(srcdir)barnhut/bhutquad.h $(srcdir)graph/vertex.h $(srcdir)ns/barnhut.h

$(srcdir)ns/barnhut.h:

$(srcdir)barnhut/bhutquad.h: $(srcdir)graph/vector.h $(srcdir)graph/vertex.h $(srcdir)ns/barnhut.h $(srcdir)service/stladdon.h

$(srcdir)service/winapi/chkerror.h: $(srcdir)ns/wapi.h

$(srcdir)service/com/comptr.h: $(srcdir)ns/atladd.h

$(srcdir)ui/window/child/cwi.h: $(srcdir)ns/atladd.h $(srcdir)ui/window/wi.h

$(srcdir)ui/nowindow/graph/draw.h: $(srcdir)graph/edge.h $(srcdir)graph/vertex.h $(srcdir)service/com/comptr.h $(srcdir)service/sse.h $(srcdir)service/stladdon.h

$(srcdir)service/winapi/directx/dx.h: $(srcdir)ns/dxu.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/functype.h

$(srcdir)ns/dxu.h:

$(srcdir)graph/edge.h: $(srcdir)graph/vertex.h $(srcdir)ns/arbor.h $(srcdir)service/stladdon.h

$(srcdir)service/functype.h:

$(srcdir)graph/graph.h: $(srcdir)ns/arbor.h $(srcdir)graph/edge.h $(srcdir)graph/vector.h $(srcdir)graph/vertex.h $(srcdir)service/sse.h $(srcdir)service/stladdon.h $(srcdir)service/winapi/srwlock.h

$(srcdir)ui/window/child/onscreen/graphwnd.h: $(srcdir)graph/graph.h $(srcdir)service/com/comptr.h $(srcdir)service/stladdon.h $(srcdir)ui/nowindow/graph/draw.h $(srcdir)ui/window/child/cwi.h

$(srcdir)service/winapi/heap.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/com/impl.h: $(srcdir)ns/atladd.h

$(srcdir)ns/miscutil.h:

$(srcdir)service/miscutil.h: $(srcdir)ns/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)sdkver.h:

$(srcdir)service/winapi/srwlock.h: $(srcdir)ns/wapi.h $(srcdir)service/functype.h

$(srcdir)service/sse.h:

$(srcdir)ns/stladd.h:

$(srcdir)service/stladdon.h: $(srcdir)ns/arbor.h $(srcdir)ns/stladd.h $(srcdir)service/winapi/heap.h

$(srcdir)service/strgutil.h: $(srcdir)service/stladdon.h

$(srcdir)service/winapi/theme.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/uh.h: $(srcdir)ns/wapi.h

$(srcdir)graph/vector.h: $(srcdir)ns/arbor.h

$(srcdir)graph/vertex.h: $(srcdir)graph/vector.h $(srcdir)ns/arbor.h $(srcdir)service/sse.h $(srcdir)service/stladdon.h

$(srcdir)ns/wapi.h:

$(srcdir)ui/window/wi.h: $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/directx/dx.h $(srcdir)service/winapi/wam/animatio.h

$(resources): $(srcdir)resource/$(projectname).rc
	$(rc) $(rcflags) $**

projectname = dtsample
projectext = .exe
platform = x86-64
srcdir = ../../../source/$(projectname)/
outdir = ../../../build/release-$(toolchain)-$(platform)/
outdirdos = ..^\..^\..^\build^\release-$(toolchain)-$(platform)^\
objdir = $(outdir)obj/$(projectname)/
objdirdos = $(outdirdos)obj^\$(projectname)^\
objects = \
$(objdir)dtsample.obj \
$(objdir)dtsmpwnd.obj \
$(objdir)miscutil.obj \
$(objdir)newdel.obj \
$(objdir)stladdon.obj \
$(objdir)strgutil.obj \
$(objdir)twinimpl.obj \
$(objdir)wi.obj
resources = $(objdir)$(projectname).res
compiler = "$(VCINSTALLDIR)bin/amd64/cl.exe"
compilerflags = \
/Bv \
/guard:cf /sdl \
/Gd \
/GL /Gw /Gy /GR- \
/Gm- /GF /GS /MP \
/fp:fast /favor:INTEL64 \
/WX /W4 /analyze:WX- /analyze /analyze:plugin"$(VCINSTALLDIR)bin/amd64/LocalESPC.dll" \
/EHsc /MD /Zc:wchar_t /Zc:forScope /Zc:inline /Zc:rvalueCast /Zc:inline /Zc:throwingNew \
/I$(srcdir) /I$(srcdir)../$(arborgvt) \
/Fo$(objdir) \
/Oi /O2 /Ob2 /Ot \
/TP \
/DWIN32 \
/DPLATFORM_WIN32 /DNTDDI_VERSION=NTDDI_WINTHRESHOLD /D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD /DWINVER=0x0A00 /DWIN32_LEAN_AND_MEAN \
/DNDEBUG /D_WIN64 /D_M_X64 /D_M_AMD64 /D_AMD64_ /U_M_IX86 /U_M_IA64 /U_X86_ \
/D_WINDOWS \
/DCODE_ANALYSIS \
/D_UNICODE /DUNICODE
linker = "$(VCINSTALLDIR)bin/amd64/link.exe"
linkerflags = \
/LIBPATH:"$(VCINSTALLDIR)LIB/amd64" \
/LIBPATH:"$(VCINSTALLDIR)ATLMFC/LIB/amd64" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)ucrt/x64" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)um/x64" \
kernel32.lib user32.lib shell32.lib \
/LIBPATH:$(outdir) \
/OPT:REF /OPT:ICF /LTCG:incremental /DYNAMICBASE /NXCOMPAT \
/MACHINE:X64 /SUBSYSTEM:WINDOWS,6.0 \
/INCREMENTAL:NO /RELEASE /MANIFEST:EMBED /MANIFESTINPUT:$(srcdir)resource/$(projectname)$(projectext).amd64.manifest \
/OUT:$(outdir)$(projectname)$(projectext)
rc = "$(WINDOWSSDKDIR)bin/x64/rc.exe"
rcflags = /I$(srcdir) /Fo$(resources) /C 65001 /D_WIN64

makeobjdir$(projectname)$(platform):
	@if not exist $(objdirdos) (mkdir $(objdirdos))

dtsample_x86-64: arborgvt_x86-64 makeobjdir$(projectname)$(platform) $(objects) $(resources)
	$(linker) $(linkerflags) $(objects) $(resources)
	@echo $(projectname)$(projectext): build completed.

$(objdir)dtsample.obj: $(srcdir)applayer/dtsample.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)dtsmpwnd.obj: $(srcdir)ui/window/top/onscreen/dtsmpwnd.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)miscutil.obj: $(srcdir)service/miscutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)newdel.obj: $(srcdir)service/newdel.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)stladdon.obj: $(srcdir)service/stladdon.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)strgutil.obj: $(srcdir)service/strgutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)twinimpl.obj: $(srcdir)ui/window/top/twinimpl.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)wi.obj: $(srcdir)ui/window/wi.cpp
	$(compiler) /c $(compilerflags) $?

$(srcdir)applayer/dtsample.cpp: $(srcdir)resource.h $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/strgutil.h $(srcdir)ui/window/top/onscreen/dtsmpwnd.h

$(srcdir)ui/window/top/onscreen/dtsmpwnd.cpp: $(srcdir)resource.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/winapi/directx/dx.h $(srcdir)ui/window/appmsg.h $(srcdir)ui/window/top/onscreen/dtsmpwnd.h

$(srcdir)service/miscutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)service/newdel.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/stladdon.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/strgutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/strgutil.h

$(srcdir)ui/window/top/twinimpl.cpp: $(srcdir)resource.h $(srcdir)ui/window/appmsg.h $(srcdir)ui/window/top/twinimpl.h

$(srcdir)ui/window/wi.cpp: $(srcdir)service/functype.h $(srcdir)service/strgutil.h $(srcdir)service/winapi/theme.h $(srcdir)ui/window/top/twinimpl.h $(srcdir)ui/window/wi.h

$(srcdir)service/winapi/wam/animatio.h: $(srcdir)ns/wapi.h $(srcdir)service/com/comptr.h

$(srcdir)ui/window/appmsg.h:

$(srcdir)ns/atladd.h:

$(srcdir)service/winapi/chkerror.h: $(srcdir)ns/wapi.h

$(srcdir)service/com/comptr.h: $(srcdir)ns/atladd.h

$(srcdir)ui/window/top/onscreen/dtsmpwnd.h: $(srcdir)../$(arborgvt)/dlllayer/arbor.h $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/winapi/handle.h $(srcdir)ui/window/top/twinimpl.h

$(srcdir)service/winapi/directx/dx.h: $(srcdir)ns/dxu.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/functype.h

$(srcdir)ns/dxu.h:

$(srcdir)service/functype.h:

$(srcdir)service/winapi/handle.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/heap.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/icon.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/menu.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)ns/miscutil.h:

$(srcdir)service/miscutil.h: $(srcdir)ns/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)resource.h:

$(srcdir)sdkver.h:

$(srcdir)service/winapi/srwlock.h: $(srcdir)ns/wapi.h $(srcdir)service/functype.h

$(srcdir)ns/stladd.h:

$(srcdir)service/stladdon.h: $(srcdir)ns/stladd.h $(srcdir)service/winapi/heap.h

$(srcdir)service/strgutil.h: $(srcdir)service/stladdon.h

$(srcdir)service/winapi/theme.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)ui/window/top/twinimpl.h: $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/icon.h $(srcdir)service/winapi/menu.h $(srcdir)ui/window/wi.h

$(srcdir)service/winapi/uh.h: $(srcdir)ns/wapi.h

$(srcdir)ns/wapi.h:

$(srcdir)ui/window/wi.h: $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/directx/dx.h $(srcdir)service/winapi/wam/animatio.h

$(srcdir)../$(arborgvt)/dlllayer/arbor.h:

$(resources): $(srcdir)resource/$(projectname).rc
	$(rc) $(rcflags) $**

platform = x86
outdir = ../../../build/release-$(toolchain)-$(platform)/
outdirdos = ..^\..^\..^\build^\release-$(toolchain)-$(platform)^\
objdir = $(outdir)obj/$(projectname)/
objdirdos = $(outdirdos)obj^\$(projectname)^\
objects = \
$(objdir)dtsample.obj \
$(objdir)dtsmpwnd.obj \
$(objdir)miscutil.obj \
$(objdir)newdel.obj \
$(objdir)stladdon.obj \
$(objdir)strgutil.obj \
$(objdir)twinimpl.obj \
$(objdir)wi.obj
resources = $(objdir)$(projectname).res
compiler = "$(VCINSTALLDIR)bin/cl.exe"
compilerflags = \
/Bv \
/guard:cf /sdl \
/Gd \
/GL /Gw /Gy /GR- \
/Gm- /GF /GS /MP \
/fp:fast /arch:SSE2 \
/WX /W4 /analyze:WX- /analyze /analyze:plugin"$(VCINSTALLDIR)bin/LocalESPC.dll" \
/EHsc /MD /Zc:wchar_t /Zc:forScope /Zc:inline /Zc:rvalueCast /Zc:inline /Zc:throwingNew \
/I$(srcdir) /I$(srcdir)../$(arborgvt) \
/Fo$(objdir) \
/Oi /O2 /Ob2 /Ot \
/TP \
/DWIN32 \
/DPLATFORM_WIN32 /DNTDDI_VERSION=NTDDI_WINTHRESHOLD /D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD /DWINVER=0x0A00 /DWIN32_LEAN_AND_MEAN \
/DNDEBUG /D_X86_ \
/D_WINDOWS \
/DCODE_ANALYSIS \
/D_UNICODE /DUNICODE
linker = "$(VCINSTALLDIR)bin/link.exe"
linkerflags = \
/LIBPATH:"$(VCINSTALLDIR)LIB" \
/LIBPATH:"$(VCINSTALLDIR)ATLMFC/LIB" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)ucrt/x86" \
/LIBPATH:"$(WINDOWSSDKDIR)lib/$(WINDOWSSDKLIBVERSION)um/x86" \
kernel32.lib user32.lib shell32.lib \
/LIBPATH:$(outdir) \
/OPT:REF /OPT:ICF /LTCG:incremental /DYNAMICBASE /NXCOMPAT \
/MACHINE:X86 /SAFESEH /SUBSYSTEM:WINDOWS,6.0 \
/INCREMENTAL:NO /RELEASE /MANIFEST:EMBED /MANIFESTINPUT:$(srcdir)resource/$(projectname)$(projectext).manifest \
/OUT:$(outdir)$(projectname)$(projectext)
rc = "$(WINDOWSSDKDIR)bin/x86/rc.exe"
rcflags = /I$(srcdir) /Fo$(resources) /C 65001 /D_WIN64

makeobjdir$(projectname)$(platform):
	@if not exist $(objdirdos) (mkdir $(objdirdos))

dtsample_x86: arborgvt_x86 makeobjdir$(projectname)$(platform) $(objects) $(resources)
	$(linker) $(linkerflags) $(objects) $(resources)
	@echo $(projectname)$(projectext): build completed.

$(objdir)dtsample.obj: $(srcdir)applayer/dtsample.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)dtsmpwnd.obj: $(srcdir)ui/window/top/onscreen/dtsmpwnd.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)miscutil.obj: $(srcdir)service/miscutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)newdel.obj: $(srcdir)service/newdel.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)stladdon.obj: $(srcdir)service/stladdon.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)strgutil.obj: $(srcdir)service/strgutil.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)twinimpl.obj: $(srcdir)ui/window/top/twinimpl.cpp
	$(compiler) /c $(compilerflags) $?

$(objdir)wi.obj: $(srcdir)ui/window/wi.cpp
	$(compiler) /c $(compilerflags) $?

$(srcdir)applayer/dtsample.cpp: $(srcdir)resource.h $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/strgutil.h $(srcdir)ui/window/top/onscreen/dtsmpwnd.h

$(srcdir)ui/window/top/onscreen/dtsmpwnd.cpp: $(srcdir)resource.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/winapi/directx/dx.h $(srcdir)ui/window/appmsg.h $(srcdir)ui/window/top/onscreen/dtsmpwnd.h

$(srcdir)service/miscutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)service/newdel.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/stladdon.cpp: $(srcdir)service/stladdon.h

$(srcdir)service/strgutil.cpp: $(srcdir)service/functype.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/strgutil.h

$(srcdir)ui/window/top/twinimpl.cpp: $(srcdir)resource.h $(srcdir)ui/window/appmsg.h $(srcdir)ui/window/top/twinimpl.h

$(srcdir)ui/window/wi.cpp: $(srcdir)service/functype.h $(srcdir)service/strgutil.h $(srcdir)service/winapi/theme.h $(srcdir)ui/window/top/twinimpl.h $(srcdir)ui/window/wi.h

$(srcdir)service/winapi/wam/animatio.h: $(srcdir)ns/wapi.h $(srcdir)service/com/comptr.h

$(srcdir)ui/window/appmsg.h:

$(srcdir)ns/atladd.h:

$(srcdir)service/winapi/chkerror.h: $(srcdir)ns/wapi.h

$(srcdir)service/com/comptr.h: $(srcdir)ns/atladd.h

$(srcdir)ui/window/top/onscreen/dtsmpwnd.h: $(srcdir)../$(arborgvt)/dlllayer/arbor.h $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/miscutil.h $(srcdir)service/stladdon.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/winapi/handle.h $(srcdir)ui/window/top/twinimpl.h

$(srcdir)service/winapi/directx/dx.h: $(srcdir)ns/dxu.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/chkerror.h $(srcdir)service/functype.h

$(srcdir)ns/dxu.h:

$(srcdir)service/functype.h:

$(srcdir)service/winapi/handle.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/heap.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/icon.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)service/winapi/menu.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)ns/miscutil.h:

$(srcdir)service/miscutil.h: $(srcdir)ns/miscutil.h $(srcdir)service/stladdon.h

$(srcdir)resource.h:

$(srcdir)sdkver.h:

$(srcdir)service/winapi/srwlock.h: $(srcdir)ns/wapi.h $(srcdir)service/functype.h

$(srcdir)ns/stladd.h:

$(srcdir)service/stladdon.h: $(srcdir)ns/stladd.h $(srcdir)service/winapi/heap.h

$(srcdir)service/strgutil.h: $(srcdir)service/stladdon.h

$(srcdir)service/winapi/theme.h: $(srcdir)ns/wapi.h $(srcdir)service/winapi/uh.h

$(srcdir)ui/window/top/twinimpl.h: $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/icon.h $(srcdir)service/winapi/menu.h $(srcdir)ui/window/wi.h

$(srcdir)service/winapi/uh.h: $(srcdir)ns/wapi.h

$(srcdir)ns/wapi.h:

$(srcdir)ui/window/wi.h: $(srcdir)ns/atladd.h $(srcdir)service/com/comptr.h $(srcdir)service/winapi/directx/dx.h $(srcdir)service/winapi/wam/animatio.h

$(srcdir)../$(arborgvt)/dlllayer/arbor.h:

$(resources): $(srcdir)resource/$(projectname).rc
	$(rc) $(rcflags) $**

clean: clean_x86-64 clean_x86

platform = x86-64
outdirdos = ..^\..^\..^\build^\release-$(toolchain)-$(platform)^\

clean_x86-64:
	@if exist $(outdirdos) (rd /S /Q $(outdirdos))

platform = x86
outdirdos = ..^\..^\..^\build^\release-$(toolchain)-$(platform)^\

clean_x86:
	@if exist $(outdirdos) (rd /S /Q $(outdirdos))
