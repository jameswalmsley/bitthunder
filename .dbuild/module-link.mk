#
#	Standard module linker
#

#
# Here we filter out "archive" targets, because they are special!
#
AR_TARGETS=$(filter %.a, $(TARGETS))

$(AR_TARGETS):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "AR" $(MODULE_NAME) $@
endif
	$(Q)$(AR) rvs $@ $(OBJECTS) 1> /dev/null 2> /dev/null

#
# RI Targets are the standard targets with the AR_TARGETS filtered out.
#


TEMP_TARGETS=$(filter-out $(AR_TARGETS), $(TARGETS))
#SO_TARGETS=$(filter %.so, $(TEMP_TARGETS))
KERN_TARGETS=$(filter %.img, $(TEMP_TARGETS))

RI_TARGETS=$(filter-out $(KERN_TARGETS), $(TEMP_TARGETS))
#RI_TARGETS=$(filter-out $(SO_TARGETS), $(TEMP2_TARGETS))


$(SO_TARGETS):


$(RI_TARGETS):
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	$(Q)$(CXX) $(OBJECTS) $(ARCHIVES) $(LDFLAGS) $(LDLIBS) -o $@


%: %.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CC|LD" $(MODULE_NAME) $@
endif
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

output/%: source/%.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CC|LD" $(MODULE_NAME) $@
endif
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

output/%: source/%.o
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	$(Q)$(CC) $(LDFLAGS) $(LDLIBS) $< -o $@


#
#	Auto linker script generation
#
$(BUILD_DIR)%.lds: $(BASE)%.lds.S
ifeq ($(DBUILD_VERBOSE_CMD), 0)											# Pretty print on successful compile, but still display errors when they occur.
	$(Q)$(PRETTY) --dbuild "LDS" $(MODULE_NAME) $(subst $(BUILD_DIR),"",$@)
endif
	@mkdir -p $(dir $@)
	$(Q)$(CC) -E -P $(CFLAGS) $< -o $@
	$(POST_CC)
