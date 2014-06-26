# BitThunder

A Reliable Real-Time Operating System & Application Framework

(c) 2012-2014 James Walmsley <james@fullfat-fs.co.uk>

Currently released under GPLv2
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

Branches are made with a --no-ff merge.

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
    make -C ../bitthunder/ PROJECT_CONFIG=y PROJECT_DIR=$(pwd) project.init
    make menuconfig
    make

## GIT Managaed Projects

    mkdir myproject.git
    cd myproject.git
    make -C ../bitthunder/ PROJECT_CONFIG=y PROJECT_DIR=$(pwd) project.git.init
	make menuconfig
    make

## Project Files

All projects create the following files:

    $(PROJECT_DIR)/Kconfig
    $(PROJECT_DIR)/objects.mk
    $(PROJECT_DIR)/main.c

After configuration the following files will be generated:

    $(PROJECT_DIR)/include/bt_bsp_config.h
    $(PROJECT_DIR)/.config

### objects.mk

In a project, the objects.mk file lists the project objects that need to be additionally built
and linked into the kernel.

### Kconfig

Its possible for a project to hook into the Kernel configuration system. Simply add any configuration
options to this file, and they will appear under "Project Options" on the menuconfig system.

### main.c

To begin with this file is blank. Booting the BitThunder kernel by default will cause it to only print

    Welcome to BitThunder

repeatedly on the boot-logger device (boot console).

To override this behaviour, you simply need to create a function called main().
