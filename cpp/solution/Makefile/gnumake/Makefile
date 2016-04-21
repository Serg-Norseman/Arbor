# Makefile for GNU make tool.
#
# Variables to control result:
# * `project`: project to build.

SHELL := /bin/bash

.DEFAULT_GOAL := all

include values.mk

# Define project name. Supported values: `arborgvt` and `dtsample`
project := $(dtsample)

ifndef project
$(error Project is not defined. The `project` variable must be set to `$(arborgvt)` or `$(dtsample)`)
endif
ifneq ($(arborgvt), $(project))
ifneq ($(dtsample), $(project))
$(error `$(project)` is an incorrect value for the `project` variable; must be set to `$(arborgvt)` or `$(dtsample)`)
endif
endif

.PHONY: all
all:
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(icc) platform=$(x86-64) releasetype=$(release) --file=arborgvt.mk --jobs=$$CPUCOUNT --output-sync=target
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(icc) platform=$(x86) releasetype=$(release) --file=arborgvt.mk --jobs=$$CPUCOUNT --output-sync=target
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(msvc) platform=$(x86-64) releasetype=$(release) --file=arborgvt.mk --jobs=$$CPUCOUNT --output-sync=target
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(msvc) platform=$(x86) releasetype=$(release) --file=arborgvt.mk --jobs=$$CPUCOUNT --output-sync=target
ifeq ($(dtsample), $(project))
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(icc) platform=$(x86-64) releasetype=$(release) --file=dtsample.mk --jobs=$$CPUCOUNT --output-sync=target
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(icc) platform=$(x86) releasetype=$(release) --file=dtsample.mk --jobs=$$CPUCOUNT --output-sync=target
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(msvc) platform=$(x86-64) releasetype=$(release) --file=dtsample.mk --jobs=$$CPUCOUNT --output-sync=target
	@CPUCOUNT=`grep -c ^processor /proc/cpuinfo` && set -x && \
$(MAKE) toolchain=$(msvc) platform=$(x86) releasetype=$(release) --file=dtsample.mk --jobs=$$CPUCOUNT --output-sync=target
endif

.PHONY: clean
clean:
	$(MAKE) toolchain=$(icc) platform=$(x86-64) releasetype=$(release) --file=arborgvt.mk $@
	$(MAKE) toolchain=$(icc) platform=$(x86) releasetype=$(release) --file=arborgvt.mk $@
	$(MAKE) toolchain=$(msvc) platform=$(x86-64) releasetype=$(release) --file=arborgvt.mk $@
	$(MAKE) toolchain=$(msvc) platform=$(x86) releasetype=$(release) --file=arborgvt.mk $@
# `arborgvt` and `dtsample` use the same `outdir`, therefore it ain't necessary to remove it twice.
#ifeq ($(dtsample), $(project))
#	$(MAKE) toolchain=$(icc) platform=$(x86-64) releasetype=$(release) --file=dtsample.mk $@
#	$(MAKE) toolchain=$(icc) platform=$(x86) releasetype=$(release) --file=dtsample.mk $@
#	$(MAKE) toolchain=$(msvc) platform=$(x86-64) releasetype=$(release) --file=dtsample.mk $@
#	$(MAKE) toolchain=$(msvc) platform=$(x86) releasetype=$(release) --file=dtsample.mk $@
#endif
