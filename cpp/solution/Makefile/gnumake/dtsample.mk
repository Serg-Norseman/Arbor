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

project := $(dtsample)
projectext := .exe
srcdir := ./../../../source/$(project)/
arborsrcdir := ./../../../source/$(arborgvt)/

vpath %.cpp \
$(srcdir) $(srcdir)applayer $(srcdir)ns $(srcdir)resource \
$(srcdir)service $(srcdir)service/com $(srcdir)service/winapi $(srcdir)service/winapi/directx $(srcdir)service/winapi/wam \
$(srcdir)ui $(srcdir)ui/window $(srcdir)ui/window/top $(srcdir)ui/window/top/onscreen
vpath %.rc $(srcdir)resource

outdir := ./../../../build/$(releasetype)-$(toolchain)-$(platform)/
objdir := $(outdir)obj/$(project)/
objects := \
$(addprefix $(objdir), \
dtsample.obj \
dtsmpwnd.obj \
miscutil.obj \
newdel.obj \
stladdon.obj \
strgutil.obj \
twinimpl.obj \
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
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Qipo -Qftz -Oi -O2 -Ob2 -Ot \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
-Qipo \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-OPT:REF -OPT:ICF -DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).amd64.manifest
else
# Settings for x86-64 debug by icc.
compilerflags := \
-Qm64 -Qstd=c++14 -Qms0 \
-GR- -Gm- -GF -GS -MP \
-fp:fast -QxSSE3 -QaxSSE3 \
-WX -W4 \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Qftz -Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).amd64.manifest
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
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Qipo -Qftz -Oi -O2 -Ob2 -Ot \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -D_X86_ \
-D_WINDOWS \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
-Qipo \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-OPT:REF -OPT:ICF -DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).manifest
else
# Settings for x86 debug by icc.
compilerflags := \
-Qm32 -Qstd=c++14 -Qms0 \
-Gd -GR- -Gm- -GF -GS -MP \
-fp:fast -QxSSE3 -QaxSSE3 \
-WX -W4 \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Qftz -Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-D__INTEL_COMPILER=1600 -DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_X86_ \
-D_WINDOWS \
-D_UNICODE -DUNICODE \
-D__is_assignable=__is_trivially_assignable
linkerflags := \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).manifest
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
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Oi -O2 -Ob2 -Ot \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-OPT:REF -OPT:ICF -LTCG:incremental -DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).amd64.manifest
else
# Settings for x86-64 debug by msvc.
compilerflags := \
-Bv \
-guard:cf -sdl \
-Gw- -Gy- -GR- -Gm- -GF -GS -MP \
-fp:fast -favor:INTEL64 \
-WX -W4 -analyze- \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_WIN64 -D_M_X64 -D_M_AMD64 -D_AMD64_ -U_M_IX86 -U_M_IA64 -U_X86_ \
-D_WINDOWS \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X64 -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).amd64.manifest
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
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Oi -O2 -Ob2 -Ot \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DNDEBUG -DX86_ \
-D_WINDOWS \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-OPT:REF -OPT:ICF -LTCG:incremental -DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL:NO -RELEASE -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).manifest
else
# Settings for x86 debug by msvc.
compilerflags := \
-Bv \
-guard:cf -sdl \
-Gd -Gw- -Gy- -GR- -Gm- -GF -GS -MP \
-fp:fast -arch:SSE2 \
-WX -W4 -analyze- \
-EHsc -MDd -Zc:wchar_t -Zc:forScope -Zc:inline -Zc:rvalueCast -Zc:inline -Zc:throwingNew \
-I$(srcdir) -I$(arborsrcdir) $($(toolchain)include$(platform)) \
-Od -Oi -Zi -RTCsu \
-Fd$(objdir) \
-TP \
-DWIN32 \
-DPLATFORM_WIN32 -DNTDDI_VERSION=NTDDI_WINTHRESHOLD -D_WIN32_WINNT=_WIN32_WINNT_WINTHRESHOLD -DWINVER=0x0A00 -DWIN32_LEAN_AND_MEAN \
-DDEBUG -D_X86_ \
-D_WINDOWS \
-DCODE_ANALYSIS \
-D_UNICODE -DUNICODE
linkerflags := \
$($(toolchain)libpath$(platform)) \
kernel32.lib user32.lib shell32.lib \
-LIBPATH:$(outdir) \
-PDB:$(outdir)$(project).pdb \
-DEBUG \
-DYNAMICBASE -NXCOMPAT \
-MACHINE:X86 -SAFESEH -SUBSYSTEM:WINDOWS,6.0 \
-INCREMENTAL -MANIFEST:EMBED -MANIFESTINPUT:$(srcdir)resource/$(project)$(projectext).manifest
endif
endif
endif
rcflags := \
-I$(srcdir) \
-I$(windowssdkdir)include/$(windowssdklibversion)um \
-I$(windowssdkdir)include/$(windowssdklibversion)shared \
-I$(windowssdkdir)include/$(windowssdklibversion)ucrt \
-I$(msvcroot)include \
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
	@PATH=$(msvcbinroot$(platform)):$(windowssdkbin$(platform)):$$PATH && set -x && $(linker) $(linkerflags) -OUT:$(outdir)$(project)$(projectext) $^
else
	@PATH=$(windowssdkbin$(platform)):$$PATH && set -x && $(linker) $(linkerflags) -OUT:$(outdir)$(project)$(projectext) $^
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
$(objdir)dtsample.obj: \
$(arborsrcdir)dlllayer/arbor.h \
$(arborsrcdir)dlllayer/arborvis.h \
$(arborsrcdir)graph/edge.h \
$(arborsrcdir)graph/vector.h \
$(arborsrcdir)graph/vertex.h \
$(arborsrcdir)ns/arbor.h \
$(arborsrcdir)ns/stladd.h \
$(arborsrcdir)ns/wapi.h \
$(arborsrcdir)sdkver.h \
$(arborsrcdir)service/sse.h \
$(arborsrcdir)service/stladdon.h \
$(arborsrcdir)service/winapi/heap.h \
$(arborsrcdir)service/winapi/uh.h \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/miscutil.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)resource.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/functype.h \
$(srcdir)service/miscutil.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/strgutil.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/handle.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/icon.h \
$(srcdir)service/winapi/menu.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/window/top/onscreen/dtsmpwnd.h \
$(srcdir)ui/window/top/twinimpl.h \
$(srcdir)ui/window/wi.h

$(objdir)dtsmpwnd.obj: \
$(arborsrcdir)dlllayer/arbor.h \
$(arborsrcdir)dlllayer/arborvis.h \
$(arborsrcdir)graph/edge.h \
$(arborsrcdir)graph/vector.h \
$(arborsrcdir)graph/vertex.h \
$(arborsrcdir)ns/arbor.h \
$(arborsrcdir)ns/stladd.h \
$(arborsrcdir)ns/wapi.h \
$(arborsrcdir)sdkver.h \
$(arborsrcdir)service/sse.h \
$(arborsrcdir)service/stladdon.h \
$(arborsrcdir)service/winapi/heap.h \
$(arborsrcdir)service/winapi/uh.h \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/miscutil.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)resource.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/functype.h \
$(srcdir)service/miscutil.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/handle.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/icon.h \
$(srcdir)service/winapi/menu.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/window/appmsg.h \
$(srcdir)ui/window/top/onscreen/dtsmpwnd.h \
$(srcdir)ui/window/top/twinimpl.h \
$(srcdir)ui/window/wi.h

$(objdir)miscutil.obj: \
$(srcdir)ns/miscutil.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/functype.h \
$(srcdir)service/miscutil.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)newdel.obj: \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)stladdon.obj: \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)strgutil.obj: \
$(srcdir)ns/miscutil.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/functype.h \
$(srcdir)service/miscutil.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/strgutil.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/uh.h

$(objdir)twinimpl.obj: \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/wapi.h \
$(srcdir)resource.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/functype.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/icon.h \
$(srcdir)service/winapi/menu.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/window/appmsg.h \
$(srcdir)ui/window/top/twinimpl.h \
$(srcdir)ui/window/wi.h

$(objdir)wi.obj: \
$(srcdir)ns/atladd.h \
$(srcdir)ns/dxu.h \
$(srcdir)ns/stladd.h \
$(srcdir)ns/wapi.h \
$(srcdir)service/com/comptr.h \
$(srcdir)service/functype.h \
$(srcdir)service/stladdon.h \
$(srcdir)service/strgutil.h \
$(srcdir)service/winapi/chkerror.h \
$(srcdir)service/winapi/directx/dx.h \
$(srcdir)service/winapi/heap.h \
$(srcdir)service/winapi/icon.h \
$(srcdir)service/winapi/menu.h \
$(srcdir)service/winapi/theme.h \
$(srcdir)service/winapi/uh.h \
$(srcdir)service/winapi/wam/animatio.h \
$(srcdir)ui/window/top/twinimpl.h \
$(srcdir)ui/window/wi.h
