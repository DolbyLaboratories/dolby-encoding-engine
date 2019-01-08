#-*-makefile-*-

BASE=../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS_debug = -std=c++11 -g -ggdb3 -O0 -fPIC -m64 -c -Wall
CXXFLAGS_release = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common/GenericPlugin -I$(BASE)code/common -I${APPLE_PRORES_LIB_ROOT}/include -I$(BASE)code/prores_dec/apple/src -I$(BASE)code/prores_dec
LD = $(CC)
LDFLAGS_debug = -fPIC -shared -m64
LDFLAGS_release = -O2 -fPIC -shared -m64
LDLIBS = $(APPLE_PRORES_LIB_ROOT)/lib/linux64/libProRes64.a -ldl -Wl,-rpath,\$${ORIGIN} -lpthread -lm


INPUTS_LINK_debug = $(TMP)code/common/GenericPlugin/GenericPlugin.prores_dec_apple_linux_amd64_gnu_debug.o $(TMP)code/prores_dec/apple/src/ProresDecApple.prores_dec_apple_linux_amd64_gnu_debug.o\
                    $(TMP)code/prores_dec/proresDecApiWrapper.prores_dec_apple_linux_amd64_gnu_debug.o $(TMP)code/prores_dec/prores_dec_api.prores_dec_apple_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(TMP)code/common/GenericPlugin/GenericPlugin.prores_dec_apple_linux_amd64_gnu_release.o $(TMP)code/prores_dec/apple/src/ProresDecApple.prores_dec_apple_linux_amd64_gnu_release.o\
                      $(TMP)code/prores_dec/proresDecApiWrapper.prores_dec_apple_linux_amd64_gnu_release.o $(TMP)code/prores_dec/prores_dec_api.prores_dec_apple_linux_amd64_gnu_release.o


default: prores_dec_apple_debug.so prores_dec_apple_release.so

prores_dec_apple_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

prores_dec_apple_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.prores_dec_apple_linux_amd64_gnu_debug.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -std=c++11 -MM $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(TMP)%.prores_dec_apple_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -std=c++11 -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_debug = $(TMP)code/common/GenericPlugin/GenericPlugin.prores_dec_apple_linux_amd64_gnu_debug.d $(TMP)code/prores_dec/apple/src/ProresDecApple.prores_dec_apple_linux_amd64_gnu_debug.d\
                      $(TMP)code/prores_dec/proresDecApiWrapper.prores_dec_apple_linux_amd64_gnu_debug.d $(TMP)code/prores_dec/prores_dec_api.prores_dec_apple_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(TMP)code/common/GenericPlugin/GenericPlugin.prores_dec_apple_linux_amd64_gnu_release.d $(TMP)code/prores_dec/apple/src/ProresDecApple.prores_dec_apple_linux_amd64_gnu_release.d\
                        $(TMP)code/prores_dec/proresDecApiWrapper.prores_dec_apple_linux_amd64_gnu_release.d $(TMP)code/prores_dec/prores_dec_api.prores_dec_apple_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) prores_dec_apple_debug.so
	$(RM) prores_dec_apple_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " prores_dec_apple_debug.so"
	@echo " prores_dec_apple_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)code/common/GenericPlugin/GenericPlugin.prores_dec_apple_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/prores_dec/apple/src/ProresDecApple.prores_dec_apple_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/prores_dec/proresDecApiWrapper.prores_dec_apple_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/prores_dec/prores_dec_api.prores_dec_apple_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/common/GenericPlugin/GenericPlugin.prores_dec_apple_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/prores_dec/apple/src/ProresDecApple.prores_dec_apple_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/prores_dec/proresDecApiWrapper.prores_dec_apple_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/prores_dec/prores_dec_api.prores_dec_apple_linux_amd64_gnu_release.d)


