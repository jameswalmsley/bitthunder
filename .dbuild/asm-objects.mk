
$(BUILD_DIR)%.o: $(BASE)%.s
	$(Q)$(PRETTY) --dbuild "AS" $(MODULE_NAME) $(subst $(BUILD_DIR),"",$@)
	@mkdir -p $(dir $@)
	$(Q)$(AS) $(ASFLAGS) $< -o $@


$(BUILD_DIR)application/%.o: $(APP_DIR)%.s
	$(Q)$(PRETTY) --dbuild "AS" $(MODULE_NAME) $(subst $(BUILD_DIR),"",$@)
	@mkdir -p $(dir $@)
	$(Q)$(AS) $(ASFLAGS) $< -o $@


