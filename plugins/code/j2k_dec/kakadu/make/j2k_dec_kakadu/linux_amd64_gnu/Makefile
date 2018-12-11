#-*-makefile-*-

BASE=../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS_debug = -std=c++11 -g -ggdb3 -O0 -fPIC -m64 -c -Wall
CXXFLAGS_release = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common -I${KDUROOT}/common -I${KDUROOT}/support -I$(BASE)code/j2k_dec/kakadu/src -I$(BASE)code/j2k_dec
LD = $(CC)
LDFLAGS_debug = -L${KDUROOT}/lib/linux64 -fPIC -shared -m64
LDFLAGS_release = -L${KDUROOT}/lib/linux64 -O2 -fPIC -shared -m64
LDLIBS = -Wl,-rpath,\$${ORIGIN} -lkdu_v7AR -ldl


INPUTS_LINK_debug = $(KDUROOT)/support/avx2_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_debug.o\
                    $(KDUROOT)/support/kdu_stripe_decompressor.j2k_dec_kakadu_linux_amd64_gnu_debug.o\
                    $(KDUROOT)/support/ssse3_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_debug.o\
                    $(KDUROOT)/support/supp_local.j2k_dec_kakadu_linux_amd64_gnu_debug.o\
                    $(TMP)code/j2k_dec/kakadu/src/j2k_dec_kakadu.j2k_dec_kakadu_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(KDUROOT)/support/avx2_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_release.o\
                      $(KDUROOT)/support/kdu_stripe_decompressor.j2k_dec_kakadu_linux_amd64_gnu_release.o\
                      $(KDUROOT)/support/ssse3_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_release.o\
                      $(KDUROOT)/support/supp_local.j2k_dec_kakadu_linux_amd64_gnu_release.o\
                      $(TMP)code/j2k_dec/kakadu/src/j2k_dec_kakadu.j2k_dec_kakadu_linux_amd64_gnu_release.o


default: j2k_dec_kakadu_debug.so j2k_dec_kakadu_release.so

j2k_dec_kakadu_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

j2k_dec_kakadu_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.j2k_dec_kakadu_linux_amd64_gnu_debug.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(TMP)%.j2k_dec_kakadu_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

$(KDUROOT)%.j2k_dec_kakadu_linux_amd64_gnu_debug.o: $(KDUROOT)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(KDUROOT)%.j2k_dec_kakadu_linux_amd64_gnu_release.o: $(KDUROOT)%.cpp | $(TMP)%/..
	$(CXXDEP) -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<


CLEAN_COMPILE_debug = $(KDUROOT)/support/avx2_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_debug.d\
                      $(KDUROOT)/support/kdu_stripe_decompressor.j2k_dec_kakadu_linux_amd64_gnu_debug.d\
                      $(KDUROOT)/support/ssse3_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_debug.d\
                      $(KDUROOT)/support/supp_local.j2k_dec_kakadu_linux_amd64_gnu_debug.d\
                      $(TMP)code/j2k_dec/kakadu/src/j2k_dec_kakadu.j2k_dec_kakadu_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(KDUROOT)/support/avx2_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_release.d\
                        $(KDUROOT)/support/kdu_stripe_decompressor.j2k_dec_kakadu_linux_amd64_gnu_release.d\
                        $(KDUROOT)/support/ssse3_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_release.d\
                        $(KDUROOT)/support/supp_local.j2k_dec_kakadu_linux_amd64_gnu_release.d\
                        $(TMP)code/j2k_dec/kakadu/src/j2k_dec_kakadu.j2k_dec_kakadu_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) j2k_dec_kakadu_debug.so
	$(RM) j2k_dec_kakadu_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " j2k_dec_kakadu_debug.so"
	@echo " j2k_dec_kakadu_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(KDUROOT)/support/avx2_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_debug.d)
include $(wildcard $(KDUROOT)/support/kdu_stripe_decompressor.j2k_dec_kakadu_linux_amd64_gnu_debug.d)
include $(wildcard $(KDUROOT)/support/ssse3_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_debug.d)
include $(wildcard $(KDUROOT)/support/supp_local.j2k_dec_kakadu_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/j2k_dec/kakadu/src/j2k_dec_kakadu.j2k_dec_kakadu_linux_amd64_gnu_debug.d)
include $(wildcard $(KDUROOT)/support/avx2_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_release.d)
include $(wildcard $(KDUROOT)/support/kdu_stripe_decompressor.j2k_dec_kakadu_linux_amd64_gnu_release.d)
include $(wildcard $(KDUROOT)/support/ssse3_stripe_transfer.j2k_dec_kakadu_linux_amd64_gnu_release.d)
include $(wildcard $(KDUROOT)/support/supp_local.j2k_dec_kakadu_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/j2k_dec/kakadu/src/j2k_dec_kakadu.j2k_dec_kakadu_linux_amd64_gnu_release.d)


