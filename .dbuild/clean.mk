#
#	Default Clean Process.
#

.SECONDEXPANSION:
clean:
ifeq ($(strip $(OBJECTS)),)
else
	$(Q)rm $(PRM_FLAGS) $(NEW_OBJECTS)		$(PRM_PIPE)
	$(Q)rm $(PRM_FLAGS) $(OBJECTS)			$(PRM_PIPE)
endif
ifeq ($(strip $(CPPOBJECTS)),)
else
	$(Q)rm $(PRM_FLAGS) $(CPPOBJECTS)		$(PRM_PIPE)
endif
ifeq ($(strip $(ARCHIVES)),)
else
	$(Q)rm $(PRM_FLAGS) $(ARCHIVES)			$(PRM_PIPE)
endif
ifeq ($(strip $(OBJECTS:.o=.d)),)
else
	$(Q)rm $(PRM_FLAGS) $(OBJECTS:.o=.d)	$(PRM_PIPE)
endif
ifeq ($(strip $(OBJECTS:.o=.su)),)
else
	$(Q)rm $(PRM_FLAGS) $(OBJECTS:.o=.su)	$(PRM_PIPE)
endif
ifeq ($(strip $(MODULE_TARGET)),)
else
	$(Q)rm $(PRM_FLAGS) $(MODULE_TARGET)	$(PRM_PIPE)
endif
ifeq ($(strip $(TARGETS)),)
else
	$(Q)rm $(PRM_FLAGS) $(TARGETS)			$(PRM_PIPE)
endif
ifeq ($(strip $(LINKER_SCRIPTS)),)
else
	$(Q)rm $(PRM_FLAGS) $(LINKER_SCRIPTS)			$(PRM_PIPE)
endif
ifeq ($(strip $(BASIC_TARGETS)),)
else
	$(Q)rm $(PRM_FLAGS) $(BASIC_TARGETS:%=%.d)	$(PRM_PIPE)
	$(Q)rm $(PRM_FLAGS) $(BASIC_TARGETS:%.so=%.d) $(PRM_PIPE)
	$(Q)rm $(PRM_FLAGS) $(BASIC_TARGETS:output/%=source/%.d)	$(PRM_PIPE)
	$(Q)rm $(PRM_FLAGS) $(BASIC_TARGETS)	$(PRM_PIPE)
endif
ifeq ($(strip $(MULTI_TARGETS)),)
else
#	$(Q)rm $(PRM_FLAGS) $($(patsubst %,%-OBJECTS,$(MULTI_TARGETS)):%=%.d)	$(PRM_PIPE)
	$(Q)rm $(PRM_FLAGS) $(MULTI_TARGETS)	$(PRM_PIPE)
endif
ifeq ($(CLEAN_EXTRAS), 1)
	$(Q)rm $(PRM_FLAGS) $(CLEAN_LIST)		$(PRM_PIPE)
endif
