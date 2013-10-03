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
SUB_NOMAKE	+= $(SUB_NOMAKE-y)

#
#	Concatenate all lists into a single SUBDIR_LIST variable for convenience.
#
SUBDIR_LIST += $(SUBDIRS)
SUBDIR_LIST += $(SUB_KBUILD)
SUBDIR_LIST += $(SUB_GENERIC)
SUBDIR_LIST += $(SUB_SAFE)
SUBDIR_LIST += $(SUB_NOMAKE)

###########################################################################################################
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
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	$(Q)$(MAKE) -C $@ DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET)
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post


#
#	Sub-dir Clean targets. (Creates $SUBDIR.clean).
#
$(SUBDIRS:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)$(MAKE) -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post


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
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	$(Q)$(MAKE)  -C $@ DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET) |  $(PRETTY_SUBKBUILD) $@
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	Again the same, adding a .clean() method to the SUB_KBUILD targets.
#
$(SUB_KBUILD:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)$(MAKE) -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBKBUILD) "$(@:%.clean=%)"
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	99.99999% of Makefile projects simply output the GCC/libtool or whatever else they use.
#	This provides thes case for normalising these outputs as much as possible.
#	The PRETTY_SUBGENERIC parser is used, is very basic, and should be improved over time.
#
$(SUB_GENERIC:%=%): MAKEFLAGS=
$(SUB_GENERIC:%=%):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:%=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$@" "$^"
endif
endif
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	$(Q)$(MAKE) $(JOBS) $(MAKE_FLAGS) -C $@ DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET)  | $(PRETTY_SUBGENERIC) $@
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	Again provide a clean method for that.
#
$(SUB_GENERIC:%=%.clean): MAKEFLAGS=
$(SUB_GENERIC:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)$(MAKE) $(JOBS) $(MAKE_FLAGS) -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBGENERIC)  "$(@:%.clean=%)"
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

###########################################################################################################
#
#	This provides a method for building modules in the absolute worst case scenario!
#	This is required if its impossible to build the target in parrallel reliably.
#
#	It just forces the build to use a single thread (-j1) and attempts to silence MAKEs
#	inscessant need to nag you its attention.
#
#	Yes "WARNING: Jobserver disabled message... I'm talking about you....pointless!
#
$(SUB_SAFE:%=%): MAKEFLAGS=
$(SUB_SAFE:%=%):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "!SAFE!" $(MODULE_NAME) "Building $(@:%=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$@" "$^"
endif
endif
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	$(Q)cd $@ && bash -c "$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET) | $(PRETTY_SUBGENERIC) $@"
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#   .... Of course another clean method!
#
$(SUB_SAFE:%=%.clean): MAKEFLAGS=
$(SUB_SAFE:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)cd $(@:%.clean=%) && bash -c "$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBGENERIC) $(@:%.clean=%)"
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

###########################################################################################################
#
#	This is for sub directories which use no make build system.
#
$(SUB_NOMAKE:%=%): MAKEFLAGS=
$(SUB_NOMAKE:%=%):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:%=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$@" "$^"
endif
endif
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.do
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

#
#	Again provide a clean method for that.
#
$(SUB_NOMAKE:%=%.clean): MAKEFLAGS=
$(SUB_NOMAKE:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.do
	-$(Q)$(MAKE)  $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post

###########################################################################################################
#
# This provides a module dependency mechanism. Especially usefull for libraries which require
# configuration before make.
#
DSUBDIRS 	 += $(DSUBDIRS-y)
DSUB_GENERIC += $(DSUB_GENERIC-y)
DSUB_KBUILD	 += $(DSUB_KBUILD-y)
DSUB_SAFE 	 += $(DSUB_SAFE-y)
DSUB_NOMAKE += $(DSUB_NOMAKE-y)

DSUBDIR_LIST += $(DSUBDIRS)
DSUBDIR_LIST += $(DSUB_GENERIC)
DSUBDIR_LIST += $(DSUB_KBUILD)
DSUBDIR_LIST += $(DSUB_SAFE)
DSUBDIR_LIST += $(DSUB_NOMAKE)

SUBDIR_LIST += $(DSUBDIR_LIST)

DEPS_ROOT_DIR =.deps/

$(DSUBDIR_LIST:%=%.deps):
	$(Q)mkdir -p $(DEPS_ROOT_DIR)$(dir $@)
	$(Q)bash $(DBUILD_ROOT).dbuild/makedeps.sh $(DEPS_ROOT_DIR)$(@:%.deps=%) $(@:%.deps=%) $(EXTRA_DEPSDIRS) > $(DEPS_ROOT_DIR)$(@:%.deps=%).d

initdeps:
	$(Q)rm -rf $(DEPS_ROOT_DIR)

$(DSUBDIR_LIST:%=%.deps): initdeps

deps: $(DSUBDIR_LIST:%=%.deps)

.PHONY: initdeps deps $(DSUBDIR_LIST:%=%.deps)

-include $(DSUBDIR_LIST:%=$(DEPS_ROOT_DIR)%.d)

#
#	For standard sub directories
#
$(DSUBDIRS:%=$(DEPS_ROOT_DIR)%.stamp):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:$(DEPS_ROOT_DIR)%.stamp=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$(@:$(DEPS_ROOT_DIR)%.stamp=%)" "$^"
endif
endif
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).pre
	$(Q)$(MAKE)  -C $(@:$(DEPS_ROOT_DIR)%.stamp=%) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET)
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).post
	$(Q)touch $@

$(DSUBDIRS:%=%):
	$(Q)$(MAKE) SUBDIR_PARAMS=$(SUBDIR_PARAMS) SUBDIR_TARGET=$(SUBDIR_TARGET) $(@:%=$(DEPS_ROOT_DIR)%.stamp)

$(DSUBDIRS:%=%.force):
	$(Q)rm -f $(@:%.force=$(DEPS_ROOT_DIR)%.stamp)
	$(Q)$(MAKE) SUBDIR_PARAMS=$(SUBDIR_PARAMS) SUBDIR_TARGET=$(SUBDIR_TARGET) $(@:%.force=%)

$(DSUBDIRS:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)$(MAKE) -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean
	-$(Q)$(MAKE) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post
	$(Q)rm -f $(@:%.clean=$(DEPS_ROOT_DIR)%.stamp)

#
#	For kbuild sub directories
#
$(DSUB_KBUILD:%=$(DEPS_ROOT_DIR)%.stamp):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:$(DEPS_ROOT_DIR)%.stamp=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$(@:$(DEPS_ROOT_DIR)%.stamp=%)" "$^"
endif
endif
	-$(Q)$(MAKE)  DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).pre
	$(Q)$(MAKE)  -C $(@:$(DEPS_ROOT_DIR)%.stamp=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET) |  $(PRETTY_SUBKBUILD) "$(@:$(DEPS_ROOT_DIR)%.stamp=%)"
	-$(Q)$(MAKE)  DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).post
	$(Q)touch $@;

$(DSUB_KBUILD:%=%):
	$(Q)$(MAKE) SUBDIR_PARAMS=$(SUBDIR_PARAMS) SUBDIR_TARGET=$(SUBDIR_TARGET) $(@:%=$(DEPS_ROOT_DIR)%.stamp)

$(DSUB_KBUILD:%=%.force):
	$(Q)rm -f $(@:%.force=$(DEPS_ROOT_DIR)%.stamp)
	$(Q)$(MAKE) SUBDIR_PARAMS=$(SUBDIR_PARAMS) SUBDIR_TARGET=$(SUBDIR_TARGET) $(@:%.force=%)

$(DSUB_KBUILD:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE)  DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)$(MAKE)  -C $(@:%.clean=%) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBKBUILD) "$(@:%.clean=%)"
	-$(Q)$(MAKE)  DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post
	$(Q)rm -f $(@:%.clean=$(DEPS_ROOT_DIR)%.stamp)

#
#	For generic sub directories
#
$(DSUB_GENERIC:%=$(DEPS_ROOT_DIR)%.stamp): MAKEFLAGS=
$(DSUB_GENERIC:%=$(DEPS_ROOT_DIR)%.stamp):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:$(DEPS_ROOT_DIR)%.stamp=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$(@:$(DEPS_ROOT_DIR)%.stamp=%)" "$^"
endif
endif
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).pre
	$(Q)$(MAKE) $(JOBS) -C $(@:$(DEPS_ROOT_DIR)%.stamp=%) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET) | $(PRETTY_SUBGENERIC) "$(@:$(DEPS_ROOT_DIR)%.stamp=%)"
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).post;
	$(Q)touch $@;

$(DSUB_GENERIC:%=%):
	$(Q)$(MAKE)  $(@:%=$(DEPS_ROOT_DIR)%.stamp)

$(DSUB_GENERIC:%=%.force):
	$(Q)rm -f $(@:%.force=$(DEPS_ROOT_DIR)%.stamp)
	$(Q)$(MAKE)  $(@:%.force=%)

$(DSUB_GENERIC:%=%.clean): MAKEFLAGS=
$(DSUB_GENERIC:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)$(MAKE) $(JOBS) -C $(@:%.clean=%) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBGENERIC)  "$(@:%.clean=%)"
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post
	$(Q)rm -f $(@:%.clean=$(DEPS_ROOT_DIR)%.stamp)

#
#	For safe sub directories
#
$(DSUB_SAFE:%=$(DEPS_ROOT_DIR)%.stamp): MAKEFLAGS=
$(DSUB_SAFE:%=$(DEPS_ROOT_DIR)%.stamp):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "!SAFE!" $(MODULE_NAME) "Building $(@:$(DEPS_ROOT_DIR)%.stamp=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$(@:$(DEPS_ROOT_DIR)%.stamp=%)" "$^"
endif
endif
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).pre
	$(Q)cd $(@:$(DEPS_ROOT_DIR)%.stamp=%) && bash -c "$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(SUBDIR_TARGET) | $(PRETTY_SUBGENERIC) $(@:$(DEPS_ROOT_DIR)%.stamp=%)"
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).post
	$(Q)touch $@

$(DSUB_SAFE:%=%):
	$(Q)$(MAKE)  $(@:%=$(DEPS_ROOT_DIR)%.stamp)

$(DSUB_SAFE:%=%.force):
	$(Q)rm -f $(@:%.force=$(DEPS_ROOT_DIR)%.stamp)
	$(Q)$(MAKE)  $(@:%.force=%)

$(DSUB_SAFE:%=%.clean): MAKEFLAGS=
$(DSUB_SAFE:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	-$(Q)cd $(@:%.clean=%) && bash -c "$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) clean | $(PRETTY_SUBGENERIC) $(@:%.clean=%)"
	-$(Q)$(MAKE) -j1 $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post
	$(Q)rm -f $(@:%.clean=$(DEPS_ROOT_DIR)%.stamp)

#
#	For sub directories with no make build system
#
$(DSUB_NOMAKE:%=$(DEPS_ROOT_DIR)%.stamp): MAKEFLAGS=
$(DSUB_NOMAKE:%=$(DEPS_ROOT_DIR)%.stamp):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "BUILD" $(MODULE_NAME) "Building $(@:$(DEPS_ROOT_DIR)%.stamp=%)"
ifeq ($(DBUILD_VERBOSE_DEPS), 1)
	$(Q)$(PRETTY) --dbuild "^DEPS^" "$(@:$(DEPS_ROOT_DIR)%.stamp=%)" "$^"
endif
endif
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).pre
	$(Q)$(MAKE) MAKEFLAGS= $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).do
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $(@:$(DEPS_ROOT_DIR)%.stamp=%).post
	$(Q)touch $@;

$(DSUB_NOMAKE:%=%):
	$(Q)$(MAKE)  $(@:%=$(DEPS_ROOT_DIR)%.stamp)

$(DSUB_NOMAKE:%=%.force):
	$(Q)rm -f $(@:%.force=$(DEPS_ROOT_DIR)%.stamp)
	$(Q)$(MAKE)  $(@:%.force=%)

$(DSUB_NOMAKE:%=%.clean): MAKEFLAGS=
$(DSUB_NOMAKE:%=%.clean):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CLEAN" $(MODULE_NAME) "$(@:%.clean=%)"
endif
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.pre
	$(Q)$(MAKE) MAKEFLAGS= $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.do
	-$(Q)$(MAKE) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) $@.post
	$(Q)rm -f $(@:%.clean=$(DEPS_ROOT_DIR)%.stamp)

###########################################################################################################
#
# Other usefull targets and dependencies for subdirs ...
#
$(SUBDIR_LIST:%=%.install): %.install: %
	$(Q)$(MAKE) -C $(@:%.install=%) $(MAKE_FLAGS) DBUILD_SPLASHED=1 $(SUBDIR_PARAMS) DESTDIR=$(DESTDIR) install

clean: $(SUBDIR_LIST:%=%.clean)

info.cleanlist:
	@echo $(SUBDIR_LIST:%=%.clean)

$(SUBDIR_LIST:%=%.pre): | silent
$(SUBDIR_LIST:%=%.post): | silent
$(SUBDIR_LIST:%=%.clean.pre): | silent
$(SUBDIR_LIST:%=%.clean.post): | silent
$(SUBDIR_LIST:%=%.install): | silent
$(SUBDIR_LIST:%=$(DEPS_ROOT_DIR)%.stamp): | silent


.PHONY: \
		$(SUBDIR_LIST) \
		$(SUBDIR_LIST:%=%.pre) \
		$(SUBDIR_LIST:%=%.do) \
		$(SUBDIR_LIST:%=%.post) \
		$(SUBDIR_LIST:%=%.force) \
		$(SUBDIR_LIST:%=%.install) \
		clean \
		$(SUBDIR_LIST:%=%.clean) \
		$(SUBDIR_LIST:%=%.clean.pre) \
		$(SUBDIR_LIST:%=%.clean.do) \
		$(SUBDIR_LIST:%=%.clean.post) \
		info.cleanlist
