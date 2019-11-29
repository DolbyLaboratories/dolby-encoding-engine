#-*-makefile-*-

BASE=../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common -I$(BEAMR_SDK)/common_primitives/include -I$(BEAMR_SDK)/inc -I$(BASE)code/hevc_enc/beamr/src -I$(BASE)code/hevc_enc
LD = $(CC)
LDFLAGS = -Wl,--as-needed -Wl,-O1 -fPIC -shared -m64
LDLIBS = ${BEAMR_SDK}/lib/libhevc-enc-l64ie.a ${BEAMR_SDK}/lib/libhevc-cmn-l64ie.a ${BEAMR_SDK}/lib/libvideoopt-l64ie.a ${BEAMR_SDK}/lib/libvpl-l64ie.a ${BEAMR_SDK}/lib/libvsl-l64ie.a ${BEAMR_SDK}/lib64/libimf.a ${BEAMR_SDK}/lib64/libippcore.a ${BEAMR_SDK}/lib64/libippcv.a ${BEAMR_SDK}/lib64/libippi.a ${BEAMR_SDK}/lib64/libipps.a ${BEAMR_SDK}/lib64/libirc.a ${BEAMR_SDK}/lib64/libsvml.a ${BEAMR_SDK}/lib64/libtbb.so.2 -ldl -lpthread -lm -Wl,-rpath,\$${ORIGIN}


INPUTS_LINK_release = $(BEAMR_SDK)/common_primitives/src/AllocationPool.hevc_enc_beamr_linux_amd64_gnu_release.o\
                      $(BEAMR_SDK)/common_primitives/src/vh3_default_callbacks.hevc_enc_beamr_linux_amd64_gnu_release.o\
                      $(BEAMR_SDK)/common_primitives/src/vh3_thread_manager.hevc_enc_beamr_linux_amd64_gnu_release.o\
                      $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr.hevc_enc_beamr_linux_amd64_gnu_release.o $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr_impl.hevc_enc_beamr_linux_amd64_gnu_release.o\
                      $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr_utils.hevc_enc_beamr_linux_amd64_gnu_release.o


default: hevc_enc_beamr_release.so

hevc_enc_beamr_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS) -o $@ $+ $(LDLIBS)

$(TMP)%.hevc_enc_beamr_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

$(BEAMR_SDK)%.hevc_enc_beamr_linux_amd64_gnu_release.o: $(BEAMR_SDK)%.cpp | $(TMP)%/..
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_release = $(BEAMR_SDK)/common_primitives/src/AllocationPool.hevc_enc_beamr_linux_amd64_gnu_release.d\
                        $(BEAMR_SDK)/common_primitives/src/vh3_default_callbacks.hevc_enc_beamr_linux_amd64_gnu_release.d\
                        $(BEAMR_SDK)/common_primitives/src/vh3_thread_manager.hevc_enc_beamr_linux_amd64_gnu_release.d\
                        $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr.hevc_enc_beamr_linux_amd64_gnu_release.d $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr_impl.hevc_enc_beamr_linux_amd64_gnu_release.d\
                        $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr_utils.hevc_enc_beamr_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_release)
	$(RM) hevc_enc_beamr_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " hevc_enc_beamr_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(BEAMR_SDK)/common_primitives/src/AllocationPool.hevc_enc_beamr_linux_amd64_gnu_release.d)
include $(wildcard $(BEAMR_SDK)/common_primitives/src/vh3_default_callbacks.hevc_enc_beamr_linux_amd64_gnu_release.d)
include $(wildcard $(BEAMR_SDK)/common_primitives/src/vh3_thread_manager.hevc_enc_beamr_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr.hevc_enc_beamr_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr_impl.hevc_enc_beamr_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/hevc_enc/beamr/src/hevc_enc_beamr_utils.hevc_enc_beamr_linux_amd64_gnu_release.d)


