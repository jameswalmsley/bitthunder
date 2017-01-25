# BitThunder

[![Build status](https://git.bitthunder.org/VitalElement/bitthunder/badges/ci/build.svg)](https://git.bitthunder.org/VitalElement/bitthunder/commits/ci)


A Reliable Real-Time Operating System & Application Framework

(c) 2012-2014 James Walmsley <james@fullfat-fs.co.uk>

Currently released under GNU GPL version 2.0
See LICENSE for more information.

## Directory Tree

    .dbuild                 - Dark Builder build system. (BitThunder edition).
    arch                    - Contains architecture specific code & bootstrapping for the OS.
    + $(ARCH)               - Contains common architecture components, e.g. NVIC/GIC drivers for ARM.
      + mach/$(SUBARCH)     - Machine sub-architectures, e.g. zynq, or cortex-m3/stm32 etc etc.
    doc                     - Documentation...
    drivers                 - Architecture independent drivers, e.g. I2C/USB/PCIe devices etc.
    kernel                  - Contains the RTOS scheduler (FreeRTOS).
    lib                     - Contains all BitThunder library code, i.e. the stuff not implementing the OS. Useful structures etc.
    os                      - Contains all of the BitThunder platform independent OS code.

## Development Process

From now on the master branch is locked into a stable development cycle. All work must be carried
out on feature branches in the form:

    feature/{username|shortcode}[.branchname]

Feature branches may only enter the master branch through a rebase, to keep the project history linear,
during which the commit history of the feature branch should be cleaned up (or squashed if appropriate).

In order for a feature branch to be accepted into the master, it must:

 * Easily rebase onto the latest master with no conflicts.
 * Cleanly compile -- NO ERRORS, NO WARNINGS, output should be "pretty".
 * PASS all unit tests (when we have a test system in place!).

In order to initiate the merging process, you must make a merge request. This is easily done on GitHUB
using the pull request feature. For those working with me on GitLab, simply make a merge request.

Merges are made with a --no-ff merge so that all commits pertaining to a feature can be easily identified.

It MUST be possible to build the kernel using a sensible configuration from any merge-point on the master branch.

## Stable Master Branch

As a consequence the master branch should remain relatively stable. By stable, it is meant that the build
process is not broken, and that a kernel image can be generated without much effort.

Even though the master branch can be considered stable in this respect, if you really need a stable
kernel, then you should use the last marked stable tag, e.g:

stable-v1.0.0

# Getting Started

With BitThunder its easy to get a standalone kernel up and running.

    make menuconfig
    make

# Project

You can create 3 types of BitThunder project.

  * A standalone bitthunder kernel, usually calls an initscript on the filesystem.
  * An out-of-tree project.
  * A GIT managed out-of-tree project, which pulls BitThunder as a submodule.

## Standalone Kernel's

These are simple:

    make menuconfig
    make

## Out-of-tree Projects

    mkdir myproject    # Create a directory for managing and building the new project.
    cd myproject

    # The next command creates an empty project with reference to the base kernel sources.
    make -C ../bitthunder/ PROJECT_DIR=$(pwd) project.init
    make menuconfig
    make

## GIT Managaed Projects

    mkdir myproject.git
    cd myproject.git
    make -C ../bitthunder/ PROJECT_DIR=$(pwd) project.git.init
	make menuconfig
    make

## Project Files

All projects create the following files:

    $(PROJECT_DIR)/Makefile
    $(PROJECT_DIR)/Kconfig
    $(PROJECT_DIR)/objects.mk
    $(PROJECT_DIR)/main.c

After configuration the following files will be generated:

    $(PROJECT_DIR)/include/bt_bsp_config.h
    $(PROJECT_DIR)/.config

### Makefile
The project Makefile configures a pair of variables and includes the kernel Makefile.

    PROJECT_DIR    - Is the directory in which the project exists. (Use $(shell pwd)).
    PROJECT_CONFIG - y (default) causes the config system to include $(PROJECT_DIR)/Kconfig.

### objects.mk

In a project, the objects.mk file lists the project objects that need to be additionally built
and linked into the kernel.

To add a file to the build process you simply add a line to the objects.mk file:

    objs += $(APP)/main.o
    objs += $(APP)/path/to/my/object.o
    include $(PROJECT_DIR)/path/to/module/objects.mk

Note you may also use configuration options (as defined in your Kconfig file) to optionally compile objects:

    objs-$(BT_CONFIG_FEATURE_A_SUPPORT) += $(APP)/feature_a.o

Note $(APP) is essentially the path to $(PROJECT_DIR), but must be used to create the correct Makefile targets.
It should also be noted that application objects.mk files use a slightly different convention.

### Kconfig

Its possible for a project to hook into the Kernel configuration system. Simply add any configuration
options to this file, and they will appear under "Project Options" on the menuconfig system.

Note: PROJECT_CONFIG=y must be set in the project Makefile ($(PROJECT_DIR)/Makefile).

### main.c

To begin with this file is blank. Booting the BitThunder kernel by default will cause it to only print

    Welcome to BitThunder

repeatedly on the boot-logger device (boot console).

To override this behaviour, you simply need to create a function called main().

# Build System
BitThunder uses a GNU Make based build system called dbuild. (Dark Builder).
The core of the build-system is found under:

    $(BASE)/.dbuild/

## PATH conventions.

All path variables in dbuild must have a trailing slash appended (for consistency and readability).

    $(BASE)/		- Root folder of the BitThunder kernel.
    $(PROJECT_DIR)/	- Root of the project based on BitThunder being developed.
    $(BUILD_DIR)/   - Path where all objects and dependency files will be generated.

The convention was recently changed so that all variables are consistent. You can use some sed rules
to update old projects easily:

    find . -name "objects.mk" | xargs sed -i 's:$(BASE):$(BASE)/:g'
    find . -name "objects.mk" | xargs sed -i 's:$(PROJECT_DIR):$(PROJECT_DIR)/:g'
    find . -name "objects.mk" | xargs sed -i 's:$(BUILD_DIR):$(BUILD_DIR)/:g'

## Variables

    $(ARCH)     - The architecture variant used. e.g. arm/blackfin/mips.
    $(SUBARCH)  - The machine variant used. e.g. zynq.

These are used in paths of object files etc.

# Kernel Development

## objects.mk files

The build system specifies all objects to be built in the objects.mk files.
Adding a new object is simple:

    OBJECTS += $(BUILD_DIR)/arch/arm/mach/zynq/myobject.o

You can also use configuration variables like:

    OBJECTS-$(BT_CONFIG_FEATURE_A) += $(BUILD_DIR)/feature_a/feature_a.o

To include another folder in the build system:

    include $(BASE)/drivers/net/objects.mk

You can include other folders based on configuration variables like:

    ifeq($(BT_CONFIG_FEATURE_A),y)
        include $(BASE)/feature_a/objects.mk
    endif
