#
#	Sub-module DISTCLEAN calling convention.
#
#	@author Andreas Friedl <afriedl@riegl.com>
#


$(DISTCLEAN_LIST:%=%.distclean): %: %.pre
	@[ ! -f $(@:%.distclean=%)/Makefile ] || $(MAKE) -C $(@:%.distclean=%) distclean
	$(Q)$(MAKE) -s $(MAKE_FLAGS) DBUILD_SPLASHED=1 $@.post

$(DISTCLEAN_LIST:%=%.distclean): %: %.pre

distclean: $(DISTCLEAN_LIST:%=%.distclean)

$(DISTCLEAN_LIST:%=%.distclean.pre): | silent
$(DISTCLEAN_LIST:%=%.distclean.post): | silent

info.distcleanlist:
	@echo $(DISTCLEAN_LIST)

.PHONY: distclean $(DISTCLEAN_LIST:%=%.distclean) $(DISTCLEAN_LIST:%=%.distclean.pre) $(DISTCLEAN_LIST:%=%.distclean.post) info.distcleanlist
