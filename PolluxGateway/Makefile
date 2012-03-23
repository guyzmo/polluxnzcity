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

CXX=/usr/bin/g++
RM=/bin/rm -rf
HEADERS=include/
SRC=src/

CFLAGS=-g3 -std=c++0x -Wno-write-strings #-DVERBOSE

%.o:
	$(CXX) $(CFLAGS) -I$(HEADERS) -o $*.o -c $*.C

pollux_gateway: $(SRC)/pollux_gateway.C src/beagle_serial.o src/xbee_communicator.o
	$(CXX) $(CFLAGS) -I$(HEADERS) -o $@ $^

clean:
	$(RM) src/*.o pollux_gateway
