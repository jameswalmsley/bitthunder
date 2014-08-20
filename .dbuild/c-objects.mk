#
# Here we define the RBUILD default implicit C object build rule.
#
# This overrides Make's default implicit rule for C objects.
#

#
#	Include all object dependencies.
#
-include $(OBJECTS:.o=.d)

NEW_OBJECTS = $(addprefix $(BUILD_DIR)/,$(OBJECTS))

#NEW_OBJECTS = $(subst $(BASE)/,$(BUILD_DIR)/,$(OBJECTS))

ifeq ($(V), 3)
qd=
else
qd=@
endif


%.o: %.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)											# Pretty print on successful compile, but still display errors when they occur.
	$(Q)$(PRETTY) --dbuild "CC" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CC) -MD -MP $(filter-out $(CFLAGS_REMOVE), $(CFLAGS)) $< -o $@
	$(POST_CC)

$(BUILD_DIR)/%.o: $(BUILD_BASE)%.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)											# Pretty print on successful compile, but still display errors when they occur.
	$(Q)$(PRETTY) --dbuild "CC" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CC) -MD -MP $(filter-out $(CFLAGS_REMOVE), $(CFLAGS)) $< -o $@
	$(POST_CC)

$(BUILD_DIR)/%.o: $(BASE)/%.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)											# Pretty print on successful compile, but still display errors when they occur.
	$(Q)$(PRETTY) --dbuild "CC" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CC) -MD -MP $(filter-out $(CFLAGS_REMOVE), $(CFLAGS)) $< -o $@
	$(POST_CC)

$(BUILD_DIR)/application/%.o: $(PROJECT_DIR)/%.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)											# Pretty print on successful compile, but still display errors when they occur.
	$(Q)$(PRETTY) --dbuild "CC" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CC) -MD -MP $(filter-out $(CFLAGS_REMOVE), $(CFLAGS)) $< -o $@
	$(POST_CC)
