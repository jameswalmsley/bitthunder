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

SO_TARGETS=$(filter %.so, $(TARGETS))

#
# RI Targets are the standard targets with the AR_TARGETS and SO_TARGETS filtered out.
#
TEMP_TARGETS=$(filter-out $(AR_TARGETS), $(TARGETS))

KERN_TARGETS=$(filter %.img, $(TEMP_TARGETS))
RI_TARGETS=$(filter-out $(KERN_TARGETS), $(TEMP_TARGETS))

$(RI_TARGETS): $(OBJECTS)
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $(OBJECTS) $(ARCHIVES) $(LDFLAGS) $(LDLIBS) -o $@

.SECONDEXPANSION:
$(MULTI_TARGETS): %: $$(%-OBJECTS)
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $^ $(ARCHIVES) $(LDFLAGS) $(LDLIBS) -o $@


%: %.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CC|LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CC) -MD -MP $(CFLAGS) $< -o $@.o
	$(POST_CC)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $@.o -o $@
	@rm -f $@.o

output/%: source/%.c
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CC|LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CC) -MD -MP $(CFLAGS) $< -o $@.o
	$(POST_CC)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $@.o -o $@
	@rm -f $@.o

%: %.cpp
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP|LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -MD -MP $(CXXFLAGS) $< -o $@.o
	$(POST_CC)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $@.o -o $@
	@rm -f $@.o

output/%: source/%.cpp
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP|LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -MD -MP $(CXXFLAGS) $< -o $@.o
	$(POST_CC)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $@.o -o $@
	@rm -f $@.o

%: %.cc
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP|LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -MD -MP $(CXXFLAGS) $< -o $@.o
	$(POST_CC)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $@.o -o $@
	@rm -f $@.o

output/%: source/%.cc
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "CPP|LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -MD -MP $(CXXFLAGS) $< -o $@.o
	$(POST_CC)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $@.o -o $@
	@rm -f $@.o

%: %.o
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $< -o $@

output/%: source/%.o
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $(LDFLAGS) $(LDLIBS) $< -o $@

%.so: %.o
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $< $(LDFLAGS) $(LDLIBS) -o $@

output/%.so: source/%.o
ifeq ($(DBUILD_VERBOSE_CMD), 0)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
endif
	@mkdir -p $(dir $@)
	$(Q)$(CXX) $< $(LDFLAGS) $(LDLIBS) -o $@

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


test_dir:
	@echo $(BUILD_DIR)
	@echo $(BASE)
	@echo $(LINKER_SCRIPTS)
