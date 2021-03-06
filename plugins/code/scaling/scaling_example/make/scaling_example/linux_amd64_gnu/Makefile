#-*-makefile-*-

BASE=../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS_debug = -std=c++11 -g -ggdb3 -O0 -fPIC -m64 -c -Wall
CXXFLAGS_release = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common -I$(BASE)code/scaling -I$(BASE)code/scaling/scaling_example/src
LD = $(CC)
LDFLAGS_debug = -fPIC -shared -m64
LDFLAGS_release = -O2 -fPIC -shared -m64
LDLIBS = -ldl


INPUTS_LINK_debug = $(TMP)code/scaling/scaling_example/src/scaling_example.scaling_example_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(TMP)code/scaling/scaling_example/src/scaling_example.scaling_example_linux_amd64_gnu_release.o


default: scaling_example_debug.so scaling_example_release.so

scaling_example_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

scaling_example_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.scaling_example_linux_amd64_gnu_debug.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(TMP)%.scaling_example_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_debug = $(TMP)code/scaling/scaling_example/src/scaling_example.scaling_example_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(TMP)code/scaling/scaling_example/src/scaling_example.scaling_example_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) scaling_example_debug.so
	$(RM) scaling_example_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " scaling_example_debug.so"
	@echo " scaling_example_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)code/scaling/scaling_example/src/scaling_example.scaling_example_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/scaling/scaling_example/src/scaling_example.scaling_example_linux_amd64_gnu_release.d)


