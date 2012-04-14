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
DEST_INSTALL=/usr/local/bin/
DEST_SYSTEMD=/lib/systemd/system/

########################################################################################

CXX=/usr/bin/g++
RM=/bin/rm -rf
INSTALL=/usr/bin/install
SYSD=/bin/systemctl

HEADERS=include
SRC=src

SOURCES=$(SRC)/xbee $(SRC)/pollux $(SRC)/beaglebone

DEFINES=
ifeq "$(VERBOSE)" "1"
DEFINES+=-DVERBOSE 
endif

CFLAGS=-lgcrypt -lgpg-error -lcurl -ljson -g3 -std=c++0x -Wno-write-strings $(DEFINES)

.PHONY: all

all: pollux_gateway

SOURCE_FILES=$(wildcard $(patsubst %,%/*.C,$(SOURCES)))
OBJECT_FILES=$(patsubst %.C,%.o,$(SOURCE_FILES))

pollux_gateway: $(SRC)/pollux_gateway.C $(OBJECT_FILES)
	$(CXX) $(CFLAGS) -I$(HEADERS) -o $@ $^

$(SRC)/%.o: $(SOURCE_FILES)
	$(CXX) $(CFLAGS) -I$(HEADERS) -o $(SRC)/$*.o -c $(SRC)/$*.C

clean:
	$(RM) $(OBJECT_FILES) pollux_gateway

package:
	@echo "TODO"

install:
	$(INSTALL) -d /var/lib/pollux
	$(INSTALL) -D pollux_gateway $(DEST_INSTALL)
	$(INSTALL) pollux_gateway.service $(DEST_SYSTEMD)
	$(SYSD) daemon-reload
	$(SYSD) enable pollux_gateway.service
	@echo "to launch me just do : systemctl start pollux_gateway.service or reboot"

uninstall:
	$(SYSD) stop pollux_gateway.service
	$(SYSD) disable pollux_gateway.service
	$(SYSD) --system daemon-reload
	$(RM) $(DEST_SYSTEMD)/pollux_gateway.service
	$(RM) $(DEST_INSTALL)/pollux_gateway 
