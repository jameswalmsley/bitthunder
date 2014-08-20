CFLAGS += -I $(BASE)/os/include/
CFLAGS += -I $(BASE)/arch/$(ARCH)/include/
CFLAGS += -I $(BASE)/drivers/
ifneq ($(PROJECT_CONFIG),y)
CFLAGS += -I $(BASE)
endif
CFLAGS += -DBT_CONFIG_OS
