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

# general configuration items
# set to 1 to have verbose output
VERBOSE?=0

# destination directories for install
DESTDIR?=/
BASEDIR?=/
DEST_INSTALL=$(DESTDIR)/usr/bin/
DEST_SYSTEMD=$(DESTDIR)/lib/systemd/system/

# versions
VER=0.1.0
REV=r0
ARCH=armv7a

########################################################################################

CXX=arm-angstrom-linux-gnueabi-g++
DOXYGEN=/usr/bin/doxygen
AR=/usr/bin/ar
MV=/bin/mv
RM=/bin/rm -rf
TAR=/bin/tar
INSTALL=/usr/bin/install
SYSD=/bin/systemctl

HEADERS=include $(BASEDIR)/include/
SRC=src

SOURCES=$(SRC)/xbee $(SRC)/pollux $(SRC)/beaglebone

DEFINES=
ifeq "$(VERBOSE)" "1"
DEFINES+=-DVERBOSE 
endif

CFLAGS = -ldl -ljson -g3 -rdynamic -std=c++0x -Wno-write-strings $(DEFINES)
#CFLAGS += -pedantic
#CFLAGS += -Wall
#CFLAGS += -Wpointer-arith
#CFLAGS += -Wcast-qual
#CFLAGS += -Wcast-align
#CFLAGS += -Wwrite-strings
#CFLAGS += -Wconversion


CHDIR_SHELL := $(SHELL)
define chdir
   $(eval _D=$(firstword $(1) $(@D)))
   $(info $(MAKE): cd $(_D)) $(eval SHELL = cd $(_D); $(CHDIR_SHELL))
endef

.PHONY: all

all: pollux_gateway pollux_calibrator extensions/datastores/citypulse.so extensions/datastores/local.so

SOURCE_FILES=$(wildcard $(patsubst %,%/*.C,$(SOURCES)))
OBJECT_FILES=$(patsubst %.C,%.o,$(SOURCE_FILES))
INCLUDES=$(patsubst %,-I%,$(HEADERS))

pollux_gateway: $(SRC)/pollux_gateway.C $(patsubst %pollux_calibrator.o,,$(OBJECT_FILES))
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $^

pollux_calibrator: $(SRC)/pollux_calibrator.C $(patsubst %pollux_prober.o,,$(OBJECT_FILES))
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $^


extensions/datastores/citypulse.so: extensions/datastores/citypulse.C
	$(CXX) -shared -lgcrypt -lgpg-error -lcurl -g3 -std=c++0x -Wno-write-strings $(INCLUDES) $(DEFINES) -o $@ $^

extensions/datastores/local.so: extensions/datastores/local.C
	$(CXX) -shared -std=c++0x -Wno-write-strings $(INCLUDES) $(DEFINES) -o $@ $^

$(SRC)/%.o: $(SOURCE_FILES)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(SRC)/$*.o -c $(SRC)/$*.C

clean:
	$(RM) $(OBJECT_FILES) extensions/datastores/citypulse.so extensions/datastores/local.so pollux_gateway pollux-gateway_$(VER)-$(REV)_$(ARCH).ipk doc target

doc:
	$(DOXYGEN) -g
	$(DOXYGEN) pollux_gateway.doxyfile

install-doc: doc
	$(INSTALL) -d $(DESTDIR)/usr/share/doc/pollux/
	$(RM) $(DESTDIR)/usr/share/doc/pollux/gateway/
	$(MV) -f doc/ $(DESTDIR)/usr/share/doc/pollux/gateway/

package: install-systemd install-doc
	$(call chdir,$(DESTDIR))
	$(TAR) -cvzf data.tar.gz etc/ usr/ lib/ var/
	cp ../debian/* ./
	$(TAR) -cvzf control.tar.gz control postinst postrm prerm
	$(AR) ru ../pollux-gateway_$(VER)-$(REV)_$(ARCH).ipk debian-binary data.tar.gz control.tar.gz

install: all
	$(INSTALL) -d $(DESTDIR)
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) -d $(DESTDIR)/etc/pollux
	$(INSTALL) -d $(DESTDIR)/var/lib/pollux
	$(INSTALL) -d $(DESTDIR)/usr/lib/pollux/extensions/datastores
	$(INSTALL) -D extensions/datastores/*.so $(DESTDIR)/usr/lib/pollux/extensions/datastores
	$(INSTALL) -D pollux_gateway $(DEST_INSTALL)

install-systemd: install
	$(INSTALL) -d $(DEST_SYSTEMD)
	$(INSTALL) pollux_gateway.service $(DEST_SYSTEMD)

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

