MACH_IMX_OBJECTS += $(BUILD_DIR)/arch/arm/mach/$(SUBARCH)/imx6.o			# Provides machine description.
MACH_IMX_OBJECTS += $(ARCH_BD)/gpio.o

MACH_IMX_OBJECTS += $(MACH_IMX_OBJECTS-y)

$(MACH_IMX_OBJECTS): MODULE_NAME="IMX"

.PHONY:
vmthunder.imx: $(PROJECT_DIR)/vmthunder.imx

$(PROJECT_DIR)/vmthunder.imx: $(PROJECT_DIR)/vmthunder.img $(BUILD_DIR)/arch/arm/mach/imx/boards/$(shell echo $(BT_CONFIG_BOARD_DCD_CONFIG_FILE))
	$(Q)$(PRETTY) MKIMAGE imx6 $(subst $(PROJECT_DIR)/,"",$@)
	$(Q)mkimage -T imximage -n $(BUILD_DIR)/arch/arm/mach/imx/boards/$(shell echo $(BT_CONFIG_BOARD_DCD_CONFIG_FILE)) -e 0x00908000 -d $(PROJECT_DIR)/vmthunder.img $@

#
#   Auto DCD config generation. (C pre-processor).
#
$(BUILD_DIR)/%.imx.cfg: $(BASE)/%.imx.cfg
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "IMX|DCD" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)gcc -E -P $(CFLAGS) - < $< > $@
	$(POST_CC)

OBJECTS += $(MACH_IMX_OBJECTS)
all: $(PROJECT_DIR)/vmthunder.imx
