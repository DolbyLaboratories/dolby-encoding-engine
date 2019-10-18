#-*-makefile-*-

BASE=../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS_debug = -std=c++11 -g -ggdb3 -O0 -fPIC -m64 -c -Wall
CXXFLAGS_release = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common/GenericPlugin -I$(BASE)code/common/SystemCalls -I$(BASE)code/common -I${LIBTIFFROOT}/include -I$(BASE)code/tiff_dec/libtiff/src -I$(BASE)code/tiff_dec
LD = $(CC)
LDFLAGS_debug = -L${LIBTIFFROOT}/lib/linux64 -fPIC -shared -m64
LDFLAGS_release = -L${LIBTIFFROOT}/lib/linux64 -O2 -fPIC -shared -m64
LDLIBS = -Wl,-rpath,\$${ORIGIN} -ltiff -ldl


INPUTS_LINK_debug = $(TMP)code/common/GenericPlugin/GenericPlugin.tiff_dec_libtiff_linux_amd64_gnu_debug.o $(TMP)code/common/SystemCalls/SystemCalls.tiff_dec_libtiff_linux_amd64_gnu_debug.o\
                    $(TMP)code/tiff_dec/libtiff/src/tiff_dec_libtiff.tiff_dec_libtiff_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(TMP)code/common/GenericPlugin/GenericPlugin.tiff_dec_libtiff_linux_amd64_gnu_release.o $(TMP)code/common/SystemCalls/SystemCalls.tiff_dec_libtiff_linux_amd64_gnu_release.o\
                      $(TMP)code/tiff_dec/libtiff/src/tiff_dec_libtiff.tiff_dec_libtiff_linux_amd64_gnu_release.o


default: tiff_dec_libtiff_debug.so tiff_dec_libtiff_release.so

tiff_dec_libtiff_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

tiff_dec_libtiff_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.tiff_dec_libtiff_linux_amd64_gnu_debug.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -std=c++11 -MM $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(TMP)%.tiff_dec_libtiff_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -std=c++11 -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_debug = $(TMP)code/common/GenericPlugin/GenericPlugin.tiff_dec_libtiff_linux_amd64_gnu_debug.d $(TMP)code/common/SystemCalls/SystemCalls.tiff_dec_libtiff_linux_amd64_gnu_debug.d\
                      $(TMP)code/tiff_dec/libtiff/src/tiff_dec_libtiff.tiff_dec_libtiff_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(TMP)code/common/GenericPlugin/GenericPlugin.tiff_dec_libtiff_linux_amd64_gnu_release.d $(TMP)code/common/SystemCalls/SystemCalls.tiff_dec_libtiff_linux_amd64_gnu_release.d\
                        $(TMP)code/tiff_dec/libtiff/src/tiff_dec_libtiff.tiff_dec_libtiff_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) tiff_dec_libtiff_debug.so
	$(RM) tiff_dec_libtiff_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " tiff_dec_libtiff_debug.so"
	@echo " tiff_dec_libtiff_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)code/common/GenericPlugin/GenericPlugin.tiff_dec_libtiff_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/common/SystemCalls/SystemCalls.tiff_dec_libtiff_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/tiff_dec/libtiff/src/tiff_dec_libtiff.tiff_dec_libtiff_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/common/GenericPlugin/GenericPlugin.tiff_dec_libtiff_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/common/SystemCalls/SystemCalls.tiff_dec_libtiff_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/tiff_dec/libtiff/src/tiff_dec_libtiff.tiff_dec_libtiff_linux_amd64_gnu_release.d)


