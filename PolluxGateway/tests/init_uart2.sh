#!/bin/sh

# to do it in C, RTFM/UTSL 
# http://code.google.com/p/beagle-borg/source/browse/kernel/arch/arm/mach-omap2/board-am335xevm.c
# looking for macro called AM33XX_MUX 
# and a struct named omap_board_mux

echo 1 > /sys/kernel/debug/omap_mux/spi0_d0
echo 21 > /sys/kernel/debug/omap_mux/spi0_sclk

