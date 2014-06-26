ifeq ($(PYTHON),)
PYTHON := python
endif

RELPATH		:= $(PYTHON) $(DBUILD_ROOT).dbuild/relpath.py
PRETTY 		:= $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/pretty.py
PRETTIFY	:= $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/prettify.py
PCP			:= $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/prettycp.py --dbuild "CP"
PMV			:= $(DBUILD_ROOT).dbuild/pretty/prettymv.py --dbuild "MV"
PMD			:= $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/prettymd.py
PRM			:= $(PRETTIFY) --dbuild "RM"
PCHMOD		:= $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/prettychmod.py
PRETTYSAMBA	:= $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/prettysamba.py
PRETTYLINUX := $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/prettylinux.py
ifndef PRETTY_SUBKBUILD
  PRETTY_SUBKBUILD := $(PRETTYLINUX)
endif
ifndef PRETTY_SUBGENERIC
  PRETTY_SUBGENERIC := $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/prettygeneric.py
endif
PTODO		:= $(PYTHON) $(DBUILD_ROOT).dbuild/pretty/todo.py

PRETTYPOST	= $(PRETTY) "POST" $(@:%.post=%)
PRETTYPREP	= $(PRETTY) "PREP" $(@:%.pre=%)

ifeq ($(DBUILD_VERBOSE_CMD), 1)
#PRETTY=@ #
endif

#
#	PRM (Pretty Remove) flags
#
PRM_PIPE=
PRM_FLAGS=-rf

ifeq ($(DBUILD_VERBOSE_CMD), 0)
PRM_PIPE=| $(PRM) $(MODULE_NAME)
PRM_FLAGS=-rvf
endif

#
#	PCP Flags
#
PCP_PIPE=
PCP_FLAGS=-r

ifeq ($(DBUILD_VERBOSE_CMD), 0)
PCP_PIPE=| $(PCP) $(MODULE_NAME)
PCP_FLAGS=-vr
endif

#
#	PMV Flags
#
PMV_PIPE=
PMV_FLAGS=

ifeq ($(DBUILD_VERBOSE_CMD), 0)
PMV_PIPE=| $(PMV) $(MODULE_NAME)
PMV_FLAGS=-v
endif

#
# 	PMD Flags
#
PMD_FLAGS=-p
PMD_PIPE=

ifeq ($(DBUILD_VERBOSE_CMD), 0)
PMD_FLAGS=-pv
PMD_PIPE=| $(PMD) $(MODULE_NAME)
endif
