# Pollux'NZ City source code
#
# (c) 2012 CKAB / hackable:Devices
# (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
# (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# destination directories for install
DESTDIR?=
BASEDIR?=
DEST_INSTALL=$(DESTDIR)/usr/bin/
DEST_SYSTEMD=$(DESTDIR)/lib/systemd/system/

# versions
VER:=$(shell grep 'VERSION' include/version.h | sed 's/"//g' | awk '{print $$3}')
REV:=r0
ARCH:=armv7a

VERBOSE?=0

########################################################################################

CXX=arm-angstrom-linux-gnueabi-g++
PYCFG=$(BASEDIR)/usr/bin/python2.7-config
DOXYGEN=/usr/bin/doxygen
AR=/usr/bin/ar
MV=/bin/mv
RM=/bin/rm -rf
TAR=/bin/tar
INSTALL=/usr/bin/install
SYSD=/bin/systemctl

DEFINES?=
HEADERS=include $(BASEDIR)/usr/include/
SRC=src

SOURCES=$(SRC)/xbee $(SRC)/pollux $(SRC)/beaglebone

CFLAGS ?= ""
CCFLAGS = -g3 --std=c++0x -Wno-write-strings
CCFLAGS += -pedantic
CCFLAGS += -Wall
CCFLAGS += -Wpointer-arith
CCFLAGS += -Wcast-align
CCFLAGS += -Wwrite-strings
CCFLAGS += $(CFLAGS)

LDFLAGS ?= ""
LDDFLAGS = -lpthread -ldl -lpthread -lutil -lm -lpython2.7 -Xlinker -export-dynamic -ljson
LDDFLAGS += $(LDFLAGS)
$(warning "1" $(shell $(PYCFG) --ldflags))
$(warning "2" $(patsubst -O2,,$(shell $(PYCFG) --ldflags)))
$(warning "3" $(patsubst -I/%,-I$(BASEDIR)/%,$(patsubst -O2,,$(shell $(PYCFG) --ldflags))))
PY_LDDFLAGS=$(patsubst -I/%,-I$(BASEDIR)/%,$(patsubst -O2,,$(shell $(PYCFG) --ldflags)))
PY_INCFLAGS=$(shell $(PYCFG) --includes)

CHDIR_SHELL := $(SHELL)
define chdir
   $(eval _D=$(firstword $(1) $(@D)))
   $(info $(MAKE): cd $(_D)) $(eval SHELL = cd $(_D); $(CHDIR_SHELL))
endef

.PHONY: all

all: pollux_gateway pollux_calibrator

verbose: all pollux_gateway_verbose

SOURCE_FILES=$(wildcard $(patsubst %,%/*.C,$(SOURCES)))
OBJECT_FILES=$(patsubst %.C,%.o,$(SOURCE_FILES))
OBJECT_DEBUG_FILES=$(patsubst %.C,%_debug.o,$(SOURCE_FILES))
INCLUDES=$(patsubst %,-I%,$(HEADERS))

pollux_gateway: $(SRC)/pollux_gateway.C $(patsubst %pollux_calibrator.o,,$(OBJECT_FILES))
	$(CXX) $(LDDFLAGS) $(CCFLAGS) $(INCLUDES) $(DEFINES) -o $@ $^

pollux_gateway_verbose: $(SRC)/pollux_gateway.C $(patsubst %pollux_calibrator_debug.o,,$(OBJECT_DEBUG_FILES)) 
	$(CXX) $(LDDFLAGS) $(CCFLAGS) $(INCLUDES) $(DEFINES) -DVERBOSE -o $@ $^

pollux_calibrator: $(SRC)/pollux_calibrator.C $(patsubst %pollux_prober.o,,$(OBJECT_FILES))
	$(CXX) $(LDDFLAGS) $(CCFLAGS) $(INCLUDES) $(DEFINES) -o $@ $^

src/pollux/pollux_extension.o: CCFLAGS+=$(patsubst -I/%,-I$(BASEDIR)/%,$(patsubst -O2,,$(shell $(PYCFG) --cflags)))
src/pollux/pollux_extension.o: src/pollux/pollux_extension.C
	$(warning $(PY_INCFLAGS))
	$(CXX) $(CCFLAGS) $(INCLUDES) $(PY_INCFLAGS) $(DEFINES) -o $@ -c $^

src/pollux/pollux_extension_debug.o: CCFLAGS+=$(patsubst -I/%,-I$(BASEDIR)/%,$(patsubst -O2,,$(shell $(PYCFG) --cflags)))
src/pollux/pollux_extension_debug.o: src/pollux/pollux_extension.C
	$(CXX) $(CCFLAGS) $(INCLUDES) $(PY_INCFLAGS) $(DEFINES) -o $@ -c $^ -DVERBOSE

$(SRC)/%.o: $(SOURCE_FILES)
	$(CXX) $(CCFLAGS) $(INCLUDES) $(DEFINES) -o $(SRC)/$*.o -c $(SRC)/$*.C

$(SRC)/%_debug.o: $(SOURCE_FILES)
	$(CXX) $(CCFLAGS) $(INCLUDES) $(DEFINES) -o $(SRC)/$*_debug.o -c $(SRC)/$*.C -DVERBOSE 

clean:
	$(RM) $(OBJECT_FILES) $(patsubst %.o,%_debug.o,$(OBJECT_FILES)) pollux_gateway pollux_calibrator pollux_gateway_verbose pollux-gateway_$(VER)-$(REV)_$(ARCH).ipk doc target

doc:
	$(DOXYGEN) -g
	$(DOXYGEN) pollux_gateway.doxyfile

install-doc: doc
	$(INSTALL) -d $(DESTDIR)/usr/share/doc/pollux/
	$(RM) $(DESTDIR)/usr/share/doc/pollux/gateway/
	$(MV) -f doc/ $(DESTDIR)/usr/share/doc/pollux/gateway/

package: install-systemd install-doc
	$(INSTALL) -d ../feed/
	$(eval FEED := ../feed/)
	$(call chdir,$(DESTDIR))
	$(TAR) -cvzf data.tar.gz ./etc/ ./usr/ ./lib/ ./var/
	cp ../debian/* ./
	sed -i "s/^Version: .*$$/Version: $(VER)/g" control
	$(TAR) -cvzf control.tar.gz control postinst postrm prerm
	$(AR) ru ../pollux-gateway_$(VER)-$(REV)_$(ARCH).ipk debian-binary data.tar.gz control.tar.gz
	cp -f ../pollux-gateway_$(VER)-$(REV)_$(ARCH).ipk ../$(FEED)


install: all
	$(INSTALL) -d $(DESTDIR)/
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) -d $(DESTDIR)/etc/pollux
	$(INSTALL) -d $(DESTDIR)/var/lib/pollux
	$(INSTALL) -d $(DESTDIR)/usr/lib/pollux/extensions/datastores
	$(INSTALL) -D extensions/datastores/*.py $(DESTDIR)/usr/lib/pollux/extensions/datastores
	$(INSTALL) -D pollux_gateway $(DEST_INSTALL)
	$(INSTALL) -D pollux_calibrator $(DEST_INSTALL)
ifeq ($(VERBOSE),1)
	$(INSTALL) -D pollux_gateway_verbose $(DEST_INSTALL)
endif

install-systemd: install
	$(INSTALL) -d $(DEST_SYSTEMD)
	$(INSTALL) conf/pollux_gateway.service $(DEST_SYSTEMD)

uninstall:
	$(RM) $(DEST_INSTALL)/pollux_gateway $(DESTDIR)/usr/lib/pollux/extensions/datastores/*.so

manual-uninstall: uninstall
	$(RM) $(DEST_SYSTEMD)/pollux_gateway.service

enable: install-systemd
	$(SYSD) daemon-reload
	$(SYSD) enable pollux_gateway.service
	@echo "to launch me just do : systemctl start pollux_gateway.service or reboot"

disable:
	$(SYSD) stop pollux_gateway.service
	$(SYSD) disable pollux_gateway.service
	$(SYSD) --system daemon-reload



