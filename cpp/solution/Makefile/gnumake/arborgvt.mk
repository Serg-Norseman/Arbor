# Makefile for GNU make tool.
#
# Variables to control result:
# * `toolchain`: tool-chainf to use,
# * `platfrom`: target platform,
# * `releasetype`: release type.

SHELL := /bin/bash

.SUFFIXES:
.SUFFIXES: .cpp .obj

include icc.mk
include values.mk

# Define tool-chain to use. Currently supported tools are:
# * icc -- Intel C++ Compiler,
# * msvc -- MSFT VC++ Compiler.
toolchain := $(icc)
# Define target platform. Supported values: `x86-64` and `x86`
platform := $(x86-64)
# Define release type. Supported values: `debug` and `release`
releasetype := $(release)

ifndef toolchain
$(error Tool-chain is not defined. The `toolchain` variable must be set to `$(icc)` or `$(msvc)`)
endif
ifndef platform
$(error Target platform is not defined. The `platform` variable must be set to `$(x86-64)` or `$(x86)`)
endif
ifndef releasetype
$(error Release type is not defined. The `releasetype` variable must be set to `$(release)` or `$(debug)`)
endif

ifneq ($(icc), $(toolchain))
ifneq ($(msvc), $(toolchain))
$(error `$(toolchain)` is an incorrect value for the `toolchain` variable; must be set to `$(icc)` or `$(msvc)`)
endif
endif

ifneq ($(x86-64), $(platform))
ifneq ($(x86), $(platform))
$(error `$(platform)` is an incorrect value for the `platform` variable; must be set to `$(x86-64)` or `$(x86)`)
endif
endif

ifneq ($(debug), $(releasetype))
ifneq ($(release), $(releasetype))
$(error `$(releasetype)` is an incorrect value for the `releasetype` variable; must be set to `$(debug)` or `$(release)`)
endif
endif

compiler := $($(toolchain)compiler$(platform))
linker := $($(toolchain)linker$(platform))
rc := $($(toolchain)rc$(platform))

project := $(arborgvt)
projectext := .dll
srcdir := ./../../../source/$(project)/

vpath %.cpp \
$(srcdir) $(srcdir)barnhut $(srcdir)dlllayer $(srcdir)exports $(srcdir)graph $(srcdir)ns $(srcdir)resource \
$(srcdir)service $(srcdir)service/com $(srcdir)service/winapi $(srcdir)service/winapi/directx $(srcdir)service/winapi/wam \
$(srcdir)ui $(srcdir)ui/nowindow $(srcdir)ui/nowindow/avisimpl $(srcdir)nowindow/graph \
$(srcdir)ui/window $(srcdir)ui/window/child $(srcdir)ui/window/child/onscreen
vpath %.rc $(srcdir)resource

outdir := ./../../../build/$(releasetype)-$(toolchain)-$(platform)/
objdir := $(outdir)obj/$(project)/
objects := \
$(addprefix $(objdir), \
arbor.obj \
avisimpl.obj \
barnhut.obj \
bhutquad.obj \
dllmain.obj \
graph.obj \
graphwnd.obj \
miscutil.obj \
newdel.obj \
stladdon.obj \
strgutil.obj \
vector.obj \
wi.obj)
resources := $(addprefix $(objdir), $(project).res)
ifeq ($(icc), $(toolchain))
# Settings for ICC tool-chain.
ifeq ($(x86-64), $(platform))
# Settings for x86-64 by icc.
ifeq ($(release), $(releasetype))
# Settings for x86-64 release by icc.
compilerflags := \
-Qm64 -Qstd=c++14 -Qms0 \
-GR- -Gm- -GF -GS -MP \
-fp:fast -QxSSE3 -QaxSSE3 \
-WX -W4 \
-EHsc -MD -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Qipo -Qftz -Oi -O2 -Ob2 -Ot \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
-Qipo \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-OPT:REF -OPT:ICF -DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:NO
else
# Settings for x86-64 debug by icc.
compilerflags := \
-Qm64 -Qstd=c++14 -Qms0 \
-GR- -Gm- -GF -GS -MP \
-fp:fast -QxSSE3 -QaxSSE3 \
-WX -W4 \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Qftz -Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:NO
endif
else
# Settings for x86 by icc.
ifeq ($(release), $(releasetype))
# Settings for x86 release by icc.
compilerflags := \
-Qm32 -Qstd=c++14 -Qms0 \
-Gd -GR- -Gm- -GF -GS -MP \
-fp:fast -QxSSE3 -QaxSSE3 \
-WX -W4 \
-EHsc -MD -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Qipo -Qftz -Oi -O2 -Ob2 -Ot \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -D_X86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
-Qipo \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-OPT:REF -OPT:ICF -DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:NO
else
# Settings for x86 debug by icc.
compilerflags := \
-Qm32 -Qstd=c++14 -Qms0 \
-Gd -GR- -Gm- -GF -GS -MP \
-fp:fast -QxSSE3 -QaxSSE3 \
-WX -W4 \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Qftz -Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_X86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:NO
endif
endif
else
# Settings for MSVC tool-chain.
ifeq ($(x86-64), $(platform))
# Settings for x86-64 by msvc.
ifeq ($(release), $(releasetype))
# Settings for x86-64 release by msvc.
compilerflags := \
-Bv \
-guard:cf -sdl \
-GL -Gw -Gy -GR- -Gm- -GF -GS -MP \
-fp:fast -favor:INTEL64 \
-WX -W4 -analyze:WX- -analyze -analyze:plugin$(prefastplugindos$(platform)) \
-EHsc -MD -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Oi -O2 -Ob2 -Ot \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-OPT:REF -OPT:ICF -LTCG:incremental -DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:NO
else
# Settings for x86-64 debug by msvc.
compilerflags := \
-Bv \
-guard:cf -sdl \
-Gw- -Gy- -GR- -Gm- -GF -GS -MP \
-fp:fast -favor:INTEL64 \
-WX -W4 -analyze- \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:NO
endif
else
# Settings for x86 by msvc.
ifeq ($(release), $(releasetype))
# Settings for x86 release by msvc.
compilerflags := \
-Bv \
-guard:cf -sdl \
-Gd -GL -Gw -Gy -GR- -Gm- -GF -GS -MP \
-fp:fast -arch:SSE2 \
-WX -W4 -analyze:WX- -analyze -analyze:plugin$(prefastplugindos$(platform)) \
-EHsc -MD -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Oi -O2 -Ob2 -Ot \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -DX86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-OPT:REF -OPT:ICF -LTCG:incremental -DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:NO
else
# Settings for x86 debug by msvc.
compilerflags := \
-Bv \
-guard:cf -sdl \
-Gd -Gw- -Gy- -GR- -Gm- -GF -GS -MP \
-fp:fast -arch:SSE2 \
-WX -W4 -analyze- \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) $($(toolchain)include$(platform)) \
-Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_X86_ \
-D_WINDOWS \
-DARBORGVT_EXPORTS -D_USRDLL -D_WINDLL \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
-DLL \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-DEF:$(srcdir)exports/$(arborgvt).def \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:NO
endif
endif
endif
rcflags := \
-I$(srcdir) \
-I$(windowssdkdir)include/$(windowssdklibversion)um \
-I$(windowssdkdir)include/$(windowssdklibversion)shared \
-C 65001 -D_WIN64

.PHONY: all
all: $(outdir)$(project)$(projectext)

.PHONY: clean
clean:
	@rm -rf -- $(outdir)
	@echo "$(outdir) removed"

$(objdir):
	@mkdir -p $(objdir)

$(objects): | $(objdir)

$(outdir)$(project)$(projectext): $(objects) $(resources)
# Modify the $PATH, so 'xilink' will use correct MSVC linker.
ifeq ($(icc), $(toolchain))
	@PATH=$(msvcbinroot$(platform)):$$PATH && set -x && $(linker) $(linkerflags) -OUT:$(outdir)$(project)$(projectext) $^
else
	$(linker) $(linkerflags) -OUT:$(outdir)$(project)$(projectext) $^
endif
	@echo "**** $(project)$(projectext): build completed ($(releasetype) for $(platform) by $(toolchain))."

# ICL requires CL be on the $PATH
$(objdir)%.obj: %.cpp
ifeq ($(icc), $(toolchain))
	@PATH=$(msvcbinroot$(platform)):$$PATH && set -x && $(compiler) -c $(compilerflags) $< -Fo$@
else
	$(compiler) -c $(compilerflags) $< -Fo$@
endif

$(objdir)%.res: %.rc
	$(rc) $(rcflags) -Fo$@ $<

# Names of include files can be duplicated, therefore I have to use full paths.
$(objdir)arbor.obj: \
$(srcdir)dlllayer/arbor.h \
$(srcdir)dlllayer/arborvis.h \
$(srcdir)graph/edge.h \
$(srcdir)graph/graph.h \
$(srcdir)graph/vector.h \
$(srcdir)graph/vertex.h \
$(srcdir)ns/arbor.h \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)sdkver.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/com/impl.h \
$(srcdir)service/functype.h \
$(srcdir)service/sse.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/srwlock.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/nowindow/avisimpl/avisimpl.h \
$(srcdir)ui/nowindow/graph/draw.h \
$(srcdir)ui/window/child/cwi.h \
$(srcdir)ui/window/child/onscreen/graphwnd.h \
$(srcdir)ui/window/wi.h

$(objdir)avisimpl.obj: \
$(srcdir)dlllayer/arborvis.h \
$(srcdir)graph/edge.h \
$(srcdir)graph/graph.h \
$(srcdir)graph/vector.h \
$(srcdir)graph/vertex.h \
$(srcdir)ns/arbor.h \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)sdkver.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/com/impl.h \
$(srcdir)service/functype.h \
$(srcdir)service/sse.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/srwlock.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/nowindow/avisimpl/avisimpl.h \
$(srcdir)ui/nowindow/graph/draw.h \
$(srcdir)ui/window/child/cwi.h \
$(srcdir)ui/window/child/onscreen/graphwnd.h \
$(srcdir)ui/window/wi.h

$(objdir)barnhut.obj: \
$(srcdir)barnhut/barnhut.h \
$(srcdir)barnhut/bhutquad.h \
$(srcdir)graph/vector.h \
$(srcdir)graph/vertex.h \
$(srcdir)ns/arbor.h \
$(srcdir)ns/barnhut.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/sse.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)bhutquad.obj: \
$(srcdir)barnhut/bhutquad.h \
$(srcdir)graph/vector.h \
$(srcdir)graph/vertex.h \
$(srcdir)ns/arbor.h \
$(srcdir)ns/barnhut.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/sse.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)dllmain.obj: $(srcdir)sdkver.h

$(objdir)graph.obj: \
$(srcdir)barnhut/barnhut.h \
$(srcdir)barnhut/bhutquad.h \
$(srcdir)graph/edge.h \
$(srcdir)graph/graph.h \
$(srcdir)graph/vector.h \
$(srcdir)graph/vertex.h \
$(srcdir)ns/arbor.h \
$(srcdir)ns/barnhut.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/functype.h \
$(srcdir)service/sse.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/srwlock.h \
$(srcdir)service/winapi/uh.h

$(objdir)graphwnd.obj: \
$(srcdir)graph/edge.h \
$(srcdir)graph/graph.h \
$(srcdir)graph/vector.h \
$(srcdir)graph/vertex.h \
$(srcdir)ns/arbor.h \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/functype.h \
$(srcdir)service/sse.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/srwlock.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/nowindow/graph/draw.h \
$(srcdir)ui/window/child/cwi.h \
$(srcdir)ui/window/child/onscreen/graphwnd.h \
$(srcdir)ui/window/wi.h

$(objdir)miscutil.obj: \
$(srcdir)ns/arbor.h \
$(srcdir)ns/miscutil.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/functype.h \
$(srcdir)service/miscutil.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)newdel.obj: \
$(srcdir)ns/arbor.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)stladdon.obj: \
$(srcdir)ns/arbor.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)strgutil.obj: \
$(srcdir)ns/arbor.h \
$(srcdir)ns/miscutil.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/functype.h \
$(srcdir)service/miscutil.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/strgutil.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)vector.obj: \
$(srcdir)graph/vector.h \
$(srcdir)ns/arbor.h \
$(srcdir)service/sse.h

$(objdir)wi.obj: \
$(srcdir)graph/edge.h \
$(srcdir)graph/graph.h \
$(srcdir)graph/vector.h \
$(srcdir)graph/vertex.h \
$(srcdir)ns/arbor.h \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/functype.h \
$(srcdir)service/sse.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/strgutil.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/srwlock.h \
$(srcdir)service/winapi/theme.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/nowindow/graph/draw.h \
$(srcdir)ui/window/child/cwi.h \
$(srcdir)ui/window/child/onscreen/graphwnd.h \
$(srcdir)ui/window/wi.h
