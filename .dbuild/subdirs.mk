#
#	Riegl Builder - Subdirectory handling.
#
#	@author	James Walmsley <jwalmsley@riegl.com>
#

#
#	Each listed item in the SUBDIRS variable shall be executed in parallel.
#	We must therefore take care to provide a make job server.
#	Hence the +make command via $(MAKE).
#

#
#	This is a special feature allowing make to descend into multiple directories,
#	without having to call another makefile recursively.
#
include $(addsuffix objects.mk, $(SUB_OBJDIRS))
include $(addsuffix objects.mk, $(SUB_OBJDIRS-y))
-include $(addsuffix .config.mk, $(SUB_OBJDIRS))
-include $(addsuffix .config.mk, $(SUB_OBJDIRS-y))

#
#	Add optional SUBDIR variables for simple build configuration system.
#
SUBDIRS 	+= $(SUBDIRS-y)
SUB_KBUILD 	+= $(SUB_KBUILD-y)
SUB_GENERIC += $(SUB_GENERIC-y)
SUB_SAFE 	+= $(SUB_SAFE-y)

#
#	Concatenate all lists into a single SUBDIR_LIST variable for convenience.
#
SUBDIR_LIST += $(SUBDIRS)
SUBDIR_LIST += $(SUB_KBUILD)
SUBDIR_LIST += $(SUB_GENERIC)
SUBDIR_LIST += $(SUB_SAFE)


#
#	In the following section we define the implicit SUBDIR rules. Note there are
#	multiple rules for slight variations, but they all follow the same pattern.
#

#
#	Standard SUBDIR mechanism for the best case, that the SUBDIR contains a project
#	that also uses DBUILD.
#
$(SUBDIRS:%=%):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:%=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$@" "$^"
endif
endif
	$(Q)$(MAKE) $(MAKE_FLAGS) -C $@ DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET)
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post


#
#	Sub-dir Clean targets. (Creates $SUBDIR.clean).
#
$(SUBDIRS:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	$(Q)$(MAKE) $(MAKE_FLAGS) -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post


#
#	A number of projects use KBuild, from kernel.org. We want to allow the KBuild output
#	to be normalised into the standard DBUILD output format.
#
#	Hence we have a separate rule...
#
$(SUB_KBUILD:%=%):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $@"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$@" "$^"
endif
endif
	$(Q)$(MAKE) $(MAKE_FLAGS) -C $@ DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET) |  $(PRETTY_SUBKBUILD) $@
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	Again the same, adding a .clean() method to the SUB_KBUILD targets.
#
$(SUB_KBUILD:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	$(Q)$(MAKE) $(MAKE_FLAGS) -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBKBUILD) "$(@:%.clean=%)"
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	99.99999% of Makefile projects simply output the GCC/libtool or whatever else they use.
#	This provides thes case for normalising these outputs as much as possible.
#	The PRETTY_SUBGENERIC parser is used, is very basic, and should be improved over time.
#
$(SUB_GENERIC:%=%):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:%=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$@" "$^"
endif
endif
	$(Q)$(MAKE) $(MAKE_FLAGS) -C $@ DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET)  | $(PRETTY_SUBGENERIC) $@
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	Again provide a clean method for that.
#
$(SUB_GENERIC:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	$(Q)$(MAKE) $(MAKE_FLAGS) -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBGENERIC)  "$(@:%.clean=%)"
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	This provides a method for building modules in the absolute worst case scenario!
#	This is required if its impossible to build the target in parrallel reliably.
#
#	It just forces the build to use a single thread (-j1) and attempts to silence MAKEs
#	inscessant need to nag you its attention.
#
#	Yes "WARNING: Jobserver disabled message... I'm talking about you....pointless!
#
$(SUB_SAFE:%=%):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "!SAFE!" $(MODULE_NAME) "Building $(@:%=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$@" "$^"
endif
endif
	$(Q)cd $@ && bash -c "$(MAKE) -s -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET) | $(PRETTY_SUBGENERIC) $@"
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#   .... Of course another clean method!
#
$(SUB_SAFE:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	$(Q)cd $(@:%.clean=%) && bash -c "$(MAKE) -s -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBGENERIC) $(@:%.clean=%)"
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post


#
#	Handle pre and post targets
#
$(SUBDIR_LIST:%=%): %: %.pre
(SUBDIR_LIST:%=%.clean): %.clean: %.clean.pre

clean: $(SUBDIR_LIST:%=%.clean)

info.cleanlist:
	@echo $(SUBDIR_LIST:%=%.clean)

$(SUBDIR_LIST:%=%.pre): | silent
$(SUBDIR_LIST:%=%.post): | silent

.PHONY: \
		$(SUBDIR_LIST) \
		$(SUBDIR_LIST:%=%.pre) \
		$(SUBDIR_LIST:%=%.post) \
		clean \
		$(SUBDIR_LIST:%=%.clean) \
		$(SUBDIR_LIST:%=%.clean.pre) \
		$(SUBDIR_LIST:%=%.clean.post) \
		info.cleanlist
