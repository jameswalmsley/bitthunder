
info.config:
	@echo "CONFIG_  : " $(CONFIG_)
	@echo "TOOLCHAIN: " $($(CONFIG_)TOOLCHAIN)

info.toolchain:
	@echo "BUILD_ROOT: " $(DBUILD_ROOT)
	@echo "TOOLCHAIN : " $(TOOLCHAIN)
	@echo "CC        : " $(CC)
	@echo "CXX       : " $(CXX)

info.subdirs:
	@echo $(SUBDIRS)

info.flags:
	@echo "LDFLAGS   : " $(LDFLAGS)
	@echo "CFLAGS    : " $(CFLAGS)
