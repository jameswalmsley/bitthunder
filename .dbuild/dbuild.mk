#
#	Dark Builder - Designed and Created by James Walmsley
#
#	Dark Builder attempts to provide a clean framework for creating organised
#	and "pretty" builds with minimal effort.
#
#	Dark Builder is based upon the vebuild as found in the FullFAT project
#	in the .vebuild directory.
#
#	@see		github.com/FullFAT/FullFAT/
#	@author		James Walmsley	<jwalmsley@riegl.com>
#
#	@version	1.6.0 (BiCEP2 (Gravitational Waves))
#

DBUILD_VERSION_MAJOR:=1
DBUILD_VERSION_MINOR:=6
DBUILD_VERSION_REVISION:=0

DBUILD_VERSION_NAME:=BiCEP2 (Gravitational Waves)
DBUILD_VERSION_DATE:=March 2014

#
#	Get dbuild root directory.
#
DBUILD_ROOT:=$(dir $(lastword $(MAKEFILE_LIST)))../

#
#	Let's ensure we have a pure make environment.
#	(Delete all rules and variables).
#
MAKEFLAGS += -rR --no-print-directory

all: dbuild_entry _all

#
#	A top-level configureation file can be found in the project root dir.
#
-include $(DBUILD_ROOT)dbuild.config.mk

include $(DBUILD_ROOT).dbuild/defines.mk

#
#	A config file can be overidden or extended in any sub-directory
#
-include dbuild.config.mk

#
#	Optional Include directive, blue build attempts to build using lists of objects,
#	targets and subdirs as found in objects.mk and subdirs.mk
#
-include $(BUILD_BASE)/.config.mk
#-include $(BUILD_BASE)/.config
-include $(BUILD_BASE)/objects.mk
-include targets.mk
-include subdirs.mk

#
#	Simple backwards compatible for configurable object builds!
#
OBJECTS += $(OBJECTS-y)

#
#	A config file can be overidden or extended in any sub-directory
#
-include dbuild.config.mk
-include $(BUILD_ROOT)dbuild.config.mk

CONFIG_ ?= CONFIG_
CONFIG_PATH ?= $(DBUILD_ROOT)
CONFIG_HEADER_PATH ?= $(DBUILD_ROOT)
CONFIG_HEADER_NAME ?= "config.h"

#
#	Defaults for compile/build toolchain
#
override TOOLCHAIN 	:= $(shell echo $($(CONFIG_)TOOLCHAIN))
override AR			= $(TOOLCHAIN)ar
override AS			= $(TOOLCHAIN)as
override CC		 	= $(TOOLCHAIN)gcc
override CXX		= $(TOOLCHAIN)g++
override LD			= $(TOOLCHAIN)ld
override OBJCOPY	= $(TOOLCHAIN)objcopy
override OBJDUMP	= $(TOOLCHAIN)objdump
override SIZE		= $(TOOLCHAIN)size

export 	TOOLCHAIN
export	AR
export 	AS
export	CC
export	CXX
export	LD
export	OBJCOPY
export	OBJDUMP
export	SIZE

CFLAGS		+= -c

CFLAGS 		+= $(ADD_CFLAGS)
CXXFLAGS 	+= $(ADD_CXXFLAGS)
LDFLAGS 	+= $(ADD_LDFLAGS)

#
#	Incase the objects.mk or the .config.mk file does not exist, create a blank one.
#	We should eventually integrate this with KConfig or something nice.
#
.config.mk:
#	@touch .config.mk

objects.mk:
#	@touch objects.mk

$(TARGETS) $(TARGET_DEPS): .config.mk

$(TARGETS): objects.mk .config.mk

include $(DBUILD_ROOT).dbuild/os-detect.mk
include $(DBUILD_ROOT).dbuild/verbosity.mk
include $(DBUILD_ROOT).dbuild/pretty.mk
include $(DBUILD_ROOT).dbuild/subdirs.mk
include $(DBUILD_ROOT).dbuild/clean.mk
include $(DBUILD_ROOT).dbuild/module-link.mk
include $(DBUILD_ROOT).dbuild/c-objects.mk
include $(DBUILD_ROOT).dbuild/cpp-objects.mk
include $(DBUILD_ROOT).dbuild/asm-objects.mk
include $(DBUILD_ROOT).dbuild/info.mk

#
#	Provide a default target named all,
#	This is dependent on $(TARGETS), $(MODULE_TARGET) and $(SUBDIRS)
#
#	All is finally dependent on silent, to keep make silent when it has
#	nothing to do.
#
dbuild_entry: dbuild_splash | _all
$(TARGETS) $(SUBDIR_LIST) $(MODULE_TARGET) $(OBJECTS) clean: | dbuild_splash
_all: $(TARGETS) $(BASIC_TARGETS) $(MULTI_TARGETS) $(SUBDIR_LIST) $(MODULE_TARGET) | silent

#
#	DBuild Splash
#
.PHONY: dbuild_splash
dbuild_splash:
ifeq ($(DBUILD_SPLASHED), 1)
else
	@echo " Dark Builder"
	@echo " Version ($(DBUILD_VERSION_MAJOR).$(DBUILD_VERSION_MINOR).$(DBUILD_VERSION_REVISION) - $(DBUILD_VERSION_NAME))"
endif



.PHONY: kconfig-info
kconfig-info:
	@echo "Source code: https://github.com/jameswalmsley/kconfig-frontends (supported)"
	@echo "Source code: http://ymorin.is-a-geek.org/projects/kconfig-frontends (original)"
	@echo "MAC OSX    : brew tap PX4/homebrew-px4"
	@echo "MAC OSX    : brew update"
	@echo "MAC OSX    : brew install kconfig-frontends"

.PHONY: mkconfig
mkconfig: $(DBUILD_ROOT).dbuild/scripts/mkconfig/mkconfig
	$(Q)mkdir -p $(CONFIG_HEADER_PATH)
	$(Q)$(DBUILD_ROOT).dbuild/scripts/mkconfig/mkconfig$(OS_EXT) $(PROJECT_DIR)/ > $(CONFIG_HEADER_PATH)/$(CONFIG_HEADER_NAME)

$(CONFIG_HEADER_PATH)/$(CONFIG_HEADER_NAME): $(PROJECT_DIR)/.config $(DBUILD_ROOT).dbuild/scripts/mkconfig/mkconfig$(OS_EXT) | dbuild_splash
$(CONFIG_HEADER_PATH)/$(CONFIG_HEADER_NAME):
	$(Q)$(PRETTY) --dbuild "CONF" $(MODULE_NAME) $(subst $(BASE)/,"",$@)
	$(Q)$(MAKE) mkconfig

$(OBJECTS): $(CONFIG_HEADER_PATH)/$(CONFIG_HEADER_NAME)

CONF:=kconfig-conf
MCONF:=kconfig-mconf
MCONF_TERM:=
ifeq ($(DBUILD_OS), LINUX_64)
MCONF:=$(BASE)/scripts/kconfig-linux64/kconfig-mconf
CONF:=$(BASE)/scripts/kconfig-linux64/kconfig-conf
MCONF_TERM:=TERM=xterm-color
endif
ifeq ($(DBUILD_OS), WIN32)
MCONF:=$(BASE)/scripts/kconfig-win32/kconfig-mconf.exe
endif

.PHONY: menuconfig
menuconfig: $(DBUILD_ROOT).dbuild/scripts/mkconfig/mkconfig$(OS_EXT)
ifneq ($(DBUILD_OS), WIN32)
ifneq ($(DBUILD_OS), LINUX_64)
	@which kconfig-mconf > /dev/null || { echo "*** kconfig-frontends is not installed"; $(MAKE) kconfig-info ; false; }
endif
endif
ifneq ($(CONFIG_PATH),$(BASE))
	touch $(CONFIG_PATH)/.config
	-cp $(BASE)/.config $(CONFIG_PATH)/.config.bak
	cp $(CONFIG_PATH)/.config $(BASE)/.config
endif
	cd $(BASE)/ && $(MCONF_TERM) CONFIG_=$(CONFIG_) PROJECT_DIR=$(PROJECT_DIR) $(MCONF) Kconfig
	$(MAKE) mkconfig
ifneq ($(CONFIG_PATH),$(BASE))
	cp $(BASE)/.config $(CONFIG_PATH)/.config
	-cp $(CONFIG_PATH)/.config.bak $(BASE)/.config
endif
	$(Q)$(MAKE) mkconfig

.PHONY: oldconfig
oldconfig:
	#@which kconfig-conf > /dev/null || { echo "You need to compile and install kconfig-frontends"; false; }
	cd $(BASE)/ && CONFIG_=$(CONFIG_) PROJECT_DIR=$(PROJECT_DIR) $(CONF) --olddefconfig Kconfig .config

$(DBUILD_ROOT).dbuild/scripts/mkconfig/mkconfig$(OS_EXT): $(DBUILD_ROOT).dbuild/scripts/mkconfig/mkconfig.c
	$(Q)gcc $(DBUILD_ROOT).dbuild/scripts/mkconfig/mkconfig.c $(DBUILD_ROOT).dbuild/scripts/mkconfig/cfgparser.c $(DBUILD_ROOT).dbuild/scripts/mkconfig/cfgdefine.c -o $@


#
#	Finally provide an implementation of the silent target.
#
silent:
	@:
