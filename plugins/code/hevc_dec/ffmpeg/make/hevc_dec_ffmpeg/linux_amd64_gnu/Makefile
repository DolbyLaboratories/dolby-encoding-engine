#-*-makefile-*-

BASE=../../../../../../
CCDEP = $(CC)
CXXFLAGS_debug = -std=c++11 -g -ggdb3 -O0 -fPIC -m64 -c -Wall
CXXFLAGS_release = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common -I$(BASE)code/hevc_dec/ffmpeg/src -I$(BASE)code/hevc_dec
LD = $(CC)
LDFLAGS_debug = -fPIC -shared -m64
LDFLAGS_release = -O2 -fPIC -shared -m64
LDLIBS = -ldl


INPUTS_LINK_debug = $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg.hevc_dec_ffmpeg_linux_amd64_gnu_debug.o $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg_utils.hevc_dec_ffmpeg_linux_amd64_gnu_debug.o\
                    $(BASE)code/hevc_dec/ffmpeg/src/NamedPipe.hevc_dec_ffmpeg_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg.hevc_dec_ffmpeg_linux_amd64_gnu_release.o $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg_utils.hevc_dec_ffmpeg_linux_amd64_gnu_release.o\
                      $(BASE)code/hevc_dec/ffmpeg/src/NamedPipe.hevc_dec_ffmpeg_linux_amd64_gnu_release.o


default: hevc_dec_ffmpeg_debug.so hevc_dec_ffmpeg_release.so

hevc_dec_ffmpeg_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

hevc_dec_ffmpeg_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

%.hevc_dec_ffmpeg_linux_amd64_gnu_debug.o: %.cpp
	$(CCDEP) -std=c++11 -MM $(INCLUDES) -MT $@ -o $*.hevc_dec_ffmpeg_linux_amd64_gnu_debug.d $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

%.hevc_dec_ffmpeg_linux_amd64_gnu_release.o: %.cpp
	$(CCDEP) -std=c++11 -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $*.hevc_dec_ffmpeg_linux_amd64_gnu_release.d $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_debug = $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg.hevc_dec_ffmpeg_linux_amd64_gnu_debug.d $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg_utils.hevc_dec_ffmpeg_linux_amd64_gnu_debug.d\
                      $(BASE)code/hevc_dec/ffmpeg/src/NamedPipe.hevc_dec_ffmpeg_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg.hevc_dec_ffmpeg_linux_amd64_gnu_release.d $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg_utils.hevc_dec_ffmpeg_linux_amd64_gnu_release.d\
                        $(BASE)code/hevc_dec/ffmpeg/src/NamedPipe.hevc_dec_ffmpeg_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) hevc_dec_ffmpeg_debug.so
	$(RM) hevc_dec_ffmpeg_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " hevc_dec_ffmpeg_debug.so"
	@echo " hevc_dec_ffmpeg_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg.hevc_dec_ffmpeg_linux_amd64_gnu_debug.d)
include $(wildcard $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg_utils.hevc_dec_ffmpeg_linux_amd64_gnu_debug.d)
include $(wildcard $(BASE)code/hevc_dec/ffmpeg/src/NamedPipe.hevc_dec_ffmpeg_linux_amd64_gnu_debug.d)
include $(wildcard $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg.hevc_dec_ffmpeg_linux_amd64_gnu_release.d)
include $(wildcard $(BASE)code/hevc_dec/ffmpeg/src/hevc_dec_ffmpeg_utils.hevc_dec_ffmpeg_linux_amd64_gnu_release.d)
include $(wildcard $(BASE)code/hevc_dec/ffmpeg/src/NamedPipe.hevc_dec_ffmpeg_linux_amd64_gnu_release.d)


