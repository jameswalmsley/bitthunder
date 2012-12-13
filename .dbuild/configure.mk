#
#	Sub-module Configuration calling convention.
#
#	@author Andreas Friedl <afriedl@riegl.com>
#

#
#	Many projects use autoconf system...."shudders" :S
#	This provides a nice way to call those.
#
$(CONFIGURE_LIST:%=%.configure): %: %.pre
	@cd  $(@:%.configure=%) && ./configure $(CONFIG_OPTIONS) CC=$(TOOLCHAIN)gcc CXX=$(TOOLCHAIN)c++ LD=$(TOOLCHAIN)ld AR=$(TOOLCHAIN)ar $(PIPE_OPTIONS)
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 CONFIG_OPTIONS="$(CONFIG_OPTIONS)" $@.post

configure: $(CONFIGURE_LIST:%=%.configure)

$(CONFIGURE_LIST:%=%.configure.pre): | silent
$(CONFIGURE_LIST:%=%.configure.post): | silent

info.configurelist:
	@echo $(CONFIGURE_LIST)

$(CONFIGURE_LIST:%=%.configure): PIPE_OPTIONS=| $(PRETTY_SUBGENERIC) $(@:%.configure=%)

.PHONY: configure $(CONFIGURE_LIST:%=%.configure) $(CONFIGURE_LIST:%=%.configure) $(CONFIGURE_LIST:%=%.configure.post) info.configurelist
