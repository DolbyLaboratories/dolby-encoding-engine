#-*-makefile-*-

BASE=../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS_debug = -std=c++11 -g -ggdb3 -O0 -fPIC -m64 -c -Wall
CXXFLAGS_release = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common -I$(BASE)code/hevc_enc -I${X265ROOT}/include -I$(BASE)code/hevc_enc/x265/src
LD = $(CC)
LDFLAGS_debug = -L${X265ROOT}/lib/linux64 -fPIC -shared -m64
LDFLAGS_release = -L${X265ROOT}/lib/linux64 -O2 -fPIC -shared -m64
LDLIBS = -ldl -Wl,-rpath,\$${ORIGIN} -lx265


INPUTS_LINK_debug = $(TMP)code/hevc_enc/x265/src/hevc_enc_x265.hevc_enc_x265_linux_amd64_gnu_debug.o $(TMP)code/hevc_enc/x265/src/hevc_enc_x265_utils.hevc_enc_x265_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(TMP)code/hevc_enc/x265/src/hevc_enc_x265.hevc_enc_x265_linux_amd64_gnu_release.o $(TMP)code/hevc_enc/x265/src/hevc_enc_x265_utils.hevc_enc_x265_linux_amd64_gnu_release.o


default: hevc_enc_x265_debug.so hevc_enc_x265_release.so

hevc_enc_x265_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

hevc_enc_x265_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.hevc_enc_x265_linux_amd64_gnu_debug.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(TMP)%.hevc_enc_x265_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_debug = $(TMP)code/hevc_enc/x265/src/hevc_enc_x265.hevc_enc_x265_linux_amd64_gnu_debug.d $(TMP)code/hevc_enc/x265/src/hevc_enc_x265_utils.hevc_enc_x265_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(TMP)code/hevc_enc/x265/src/hevc_enc_x265.hevc_enc_x265_linux_amd64_gnu_release.d $(TMP)code/hevc_enc/x265/src/hevc_enc_x265_utils.hevc_enc_x265_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) hevc_enc_x265_debug.so
	$(RM) hevc_enc_x265_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " hevc_enc_x265_debug.so"
	@echo " hevc_enc_x265_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)code/hevc_enc/x265/src/hevc_enc_x265.hevc_enc_x265_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/hevc_enc/x265/src/hevc_enc_x265_utils.hevc_enc_x265_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/hevc_enc/x265/src/hevc_enc_x265.hevc_enc_x265_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/hevc_enc/x265/src/hevc_enc_x265_utils.hevc_enc_x265_linux_amd64_gnu_release.d)


