#
# Here we define the RBUILD default implicit CPP object build rule.
#
# This overrides Make's default implicit rule for CPP objects.
#

-include $(OBJECTS:.o=.d)												# Include all dependency information for all objects. (Prefixing - means if they exist).

NEW_OBJECTS = $(addprefix $(BUILD_DIR)/,$(OBJECTS))

ifeq ($(V), 3)
qd=
else
qd=@
endif

$(BUILD_DIR)/%.o: $(BUILD_BASE)%.cpp
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -MD -MP $(CXXFLAGS) $< -o $@
	$(POST_CC)

$(BUILD_DIR)/%.o: $(BASE)/%.cpp
ifeq ($(DBUILD_VERBOSE_CMD), 0)											# Pretty print on successful compile, but still display errors when they occur.
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -MD -MP $(CXXFLAGS) $< -o $@
	$(POST_CC)

%.o : %.cpp
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -MD -MP $(CXXFLAGS) $< -o $@
	$(POST_CC)


$(BUILD_DIR)/%.o: $(BUILD_BASE)%.cc
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -MD -MP $(CXXFLAGS) $< -o $@
	$(POST_CC)

$(BUILD_DIR)/%.o: $(BASE)/%.cc
ifeq ($(DBUILD_VERBOSE_CMD), 0)											# Pretty print on successful compile, but still display errors when they occur.
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -MD -MP $(CXXFLAGS) $< -o $@
	$(POST_CC)

%.o : %.cc
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(subst $(BUILD_DIR)/,"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -MD -MP $(CXXFLAGS) $< -o $@
	$(POST_CC)

