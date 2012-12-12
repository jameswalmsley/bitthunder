#
# Here we define the RBUILD default implicit C object build rule.
#
# This overrides Make's default implicit rule for C objects.
#

-include $(OBJECTS:.o=.d)												# Include all dependency information for all objects. (Prefixing - means if they exist).

NEW_OBJECTS = $(addprefix $(BUILD_DIR),$(OBJECTS))

ifeq ($(V), 3)
qd=
else
qd=@
endif

$(BUILD_DIR)%.o: $(BUILD_BASE)%.cpp
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(notdir $@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -MD -MP $(CXXFLAGS) $(CFLAGS) $< -o $@
	$(POST_CC)

%.o : %.cpp
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP" $(MODULE_NAME) $(notdir $@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -MD -MP $(CXXFLAGS) $(CFLAGS) $< -o $@
	$(POST_CC)
