
info.config:
	@echo "CONFIG_    :       " $(CONFIG_)
	@echo "TOOLCHAIN  :       " $($(CONFIG_)TOOLCHAIN)
	@echo "CONFIG_PATH:       " $(CONFIG_PATH)
	@echo "CONFIG_HEADER_PATH:" $(CONFIG_HEADER_PATH)
	@echo "CONFIG_HEADER_NAME:" $(CONFIG_HEADER_NAME)

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

info.os:
	@echo "DBUILD_OS : " $(DBUILD_OS)
