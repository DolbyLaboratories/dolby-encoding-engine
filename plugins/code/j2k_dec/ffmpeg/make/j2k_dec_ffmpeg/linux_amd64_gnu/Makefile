#-*-makefile-*-

BASE=../../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS_debug = -g -ggdb3 -O0 -fPIC -m64 -c -Wall -std=c++11
CXXFLAGS_release = -O3 -ftree-vectorize -fPIC -m64 -c -Wall -std=c++11
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)phoenix/Plugins/common -I$(BASE)phoenix/Plugins/j2k_dec/ffmpeg/src -I$(BASE)phoenix/Plugins/j2k_dec -I$(BASE)contrib -I$(BASE)contrib/ddp_enc_pro_static/v1.9 -I$(BASE)contrib/dlb_buffer/include -I$(BASE)contrib/dlb_wave/include -I$(BASE). -I$(BASE)dlb_buffer/include -I$(BASE)dlb_wave/include
LD = $(CC)
LDFLAGS_debug = -fPIC -shared -m64
LDFLAGS_release = -O2 -fPIC -shared -m64
LDLIBS = -ldl


INPUTS_LINK_debug = $(TMP)phoenix/Plugins/j2k_dec/ffmpeg/src/j2k_dec_ffmpeg.j2k_dec_ffmpeg_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(TMP)phoenix/Plugins/j2k_dec/ffmpeg/src/j2k_dec_ffmpeg.j2k_dec_ffmpeg_linux_amd64_gnu_release.o


default: j2k_dec_ffmpeg_debug.so j2k_dec_ffmpeg_release.so

j2k_dec_ffmpeg_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

j2k_dec_ffmpeg_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.j2k_dec_ffmpeg_linux_amd64_gnu_debug.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM -std=c++11 $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(TMP)%.j2k_dec_ffmpeg_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM -std=c++11 $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_debug = $(TMP)phoenix/Plugins/j2k_dec/ffmpeg/src/j2k_dec_ffmpeg.j2k_dec_ffmpeg_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(TMP)phoenix/Plugins/j2k_dec/ffmpeg/src/j2k_dec_ffmpeg.j2k_dec_ffmpeg_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) j2k_dec_ffmpeg_debug.so
	$(RM) j2k_dec_ffmpeg_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " j2k_dec_ffmpeg_debug.so"
	@echo " j2k_dec_ffmpeg_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)phoenix/Plugins/j2k_dec/ffmpeg/src/j2k_dec_ffmpeg.j2k_dec_ffmpeg_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)phoenix/Plugins/j2k_dec/ffmpeg/src/j2k_dec_ffmpeg.j2k_dec_ffmpeg_linux_amd64_gnu_release.d)


