#
# Component Makefile
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

LIBS := kws

COMPONENT_ADD_LDFLAGS     := -L $(COMPONENT_PATH)/lib \
                           $(addprefix -l,$(LIBS))


COMPONENT_ADD_INCLUDEDIRS := include

