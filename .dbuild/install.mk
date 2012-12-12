#
#	Sub-module Installation calling convention.
#
#	@author	Andreas Friedl <afriedl@riegl.com>
#

$(INSTALL_LIST:%=%.install): %: %.pre
	@[ ! -f $(@:%.install=%)/Makefile ] || $(MAKE) -C $(@:%.install=%) DESTDIR=$(INSTALL_DESTDIR)$(@:%.install=%).destdir install
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 DESTDIR=$(INSTALL_DESTDIR) $@.post

$(INSTALL_LIST:%=%.install): %: %.pre

install: $(INSTALL_LIST:%=%.install)

$(INSTALL_LIST:%=%.install.pre): | silent
$(INSTALL_LIST:%=%.install.post): | silent


info.installlist:
	@echo $(INSTALL_LIST)

.PHONY: install $(INSTALL_LIST:%=%.install) $(INSTALL_LIST:%=%.install.pre) $(INSTALL_LIST:%=%.install.post) info.installlist
