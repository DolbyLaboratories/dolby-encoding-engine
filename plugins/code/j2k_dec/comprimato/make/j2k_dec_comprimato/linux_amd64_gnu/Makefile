#-*-makefile-*-

BASE=../../../../../../
TMP=$(BASE)
.PRECIOUS: %/..
%/..:; @test $(dir $(basename $(@D))) | mkdir -p $(dir $(basename $(@D)))
CXXDEP = $(CXX)
CXXFLAGS_debug = -std=c++11 -g -ggdb3 -O0 -fPIC -m64 -c -Wall
CXXFLAGS_release = -std=c++11 -O3 -ftree-vectorize -fPIC -m64 -c -Wall
DEFINES_release = -DNDEBUG=1
INCLUDES = -I$(BASE)code/common -I${COMPRIMATOROOT}/include -I$(BASE)code/j2k_dec/comprimato/src -I$(BASE)code/j2k_dec
LD = $(CC)
LDFLAGS_debug = -L${COMPRIMATOROOT}/lib/linux64 -fPIC -shared -m64
LDFLAGS_release = -L${COMPRIMATOROOT}/lib/linux64 -O2 -fPIC -shared -m64
LDLIBS = -Wl,-rpath,\$${ORIGIN} -lcmpto_j2k_dec -ldl


INPUTS_LINK_debug = $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato.j2k_dec_comprimato_linux_amd64_gnu_debug.o $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato_plugin.j2k_dec_comprimato_linux_amd64_gnu_debug.o
INPUTS_LINK_release = $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato.j2k_dec_comprimato_linux_amd64_gnu_release.o $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato_plugin.j2k_dec_comprimato_linux_amd64_gnu_release.o


default: j2k_dec_comprimato_debug.so j2k_dec_comprimato_release.so

j2k_dec_comprimato_debug.so: $(INPUTS_LINK_debug)
	$(LD) $(LDFLAGS_debug) -o $@ $+ $(LDLIBS)

j2k_dec_comprimato_release.so: $(INPUTS_LINK_release)
	$(LD) $(LDFLAGS_release) -o $@ $+ $(LDLIBS)

$(TMP)%.j2k_dec_comprimato_linux_amd64_gnu_debug.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -std=c++11 -MM $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_debug) $(INCLUDES) -o $@ $<

$(TMP)%.j2k_dec_comprimato_linux_amd64_gnu_release.o: $(BASE)%.cpp | $(TMP)%/..
	$(CXXDEP) -std=c++11 -MM $(DEFINES_release) $(INCLUDES) -MT $@ -o $(patsubst %.o,%.d,$@) $<
	$(CXX) $(CXXFLAGS_release) $(DEFINES_release) $(INCLUDES) -o $@ $<

CLEAN_COMPILE_debug = $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato.j2k_dec_comprimato_linux_amd64_gnu_debug.d $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato_plugin.j2k_dec_comprimato_linux_amd64_gnu_debug.d
CLEAN_COMPILE_release = $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato.j2k_dec_comprimato_linux_amd64_gnu_release.d\
                        $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato_plugin.j2k_dec_comprimato_linux_amd64_gnu_release.d

cleandeps:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)

clean: cleanself

cleanself:
	$(RM) $(CLEAN_COMPILE_debug)
	$(RM) $(CLEAN_COMPILE_release)
	$(RM) $(INPUTS_LINK_debug)
	$(RM) $(INPUTS_LINK_release)
	$(RM) j2k_dec_comprimato_debug.so
	$(RM) j2k_dec_comprimato_release.so

.PHONY: default FORCE clean cleandeps cleanself help

FORCE:

help:
	@echo "This makefile has the following targets:"
	@echo " j2k_dec_comprimato_debug.so"
	@echo " j2k_dec_comprimato_release.so"
	@echo " default"
	@echo " clean"
	@echo " cleandeps"
	@echo " help"

include $(wildcard $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato.j2k_dec_comprimato_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato_plugin.j2k_dec_comprimato_linux_amd64_gnu_debug.d)
include $(wildcard $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato.j2k_dec_comprimato_linux_amd64_gnu_release.d)
include $(wildcard $(TMP)code/j2k_dec/comprimato/src/j2k_dec_comprimato_plugin.j2k_dec_comprimato_linux_amd64_gnu_release.d)


