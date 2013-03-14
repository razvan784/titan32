DEVICE=atmega324p

OBJECTS=main.o
CFLAGS=-Wall -Os -I. -DF_CPU=20000000

EFUSE=$(subst _,,0b_1_1_1_1__1_1_0_0)
#                   ^ ^ ^ ^  ^ ^ ^ ^-- BODLEVEL0 (4.3V)
#                   | | | |  | | +---- BODLEVEL1
#                   | | | |  | +------ BODLEVEL2
#                   | | | |  + ------- 1
#                   | | | +----------- 1
#                   | | +------------- 1
#                   | +--------------- 1
#                   +----------------- 1

HFUSE=$(subst _,,0b_1_1_0_1__0_1_0_1)
#                   ^ ^ ^ ^  ^ ^ ^ ^-- BOOTRST (no boot reset vector)
#                   | | | |  | | +---- BOOTSZ0 (2kB
#                   | | | |  | +------ BOOTSZ1      boot size)
#                   | | | |  + ------- EESAVE (preserve EEPROM over chip erase)
#                   | | | +----------- WDTON
#                   | | +------------- SPIEN (allow serial programming)
#                   | +--------------- JTAGEN (disable JTAG, save 4 pins)
#                   +----------------- OCDEN (disable on-chip debug)

LFUSE=$(subst _,,0b_1_1_1_1__0_1_1_1)
#                   ^ ^ \ /  \--+--/
#                   | |  |      +----- CKSEL 3..0 (external crystal, full-swing)
#                   | |  +------------ SUT 1..0 (long startup time)
#                   | +--------------- CKOUT
#                   +----------------- CKDIV8

LOCKB=$(subst _,,0b_1_1_0_0__0_1_0_0)
#                   ^ ^ ^ ^  ^ ^ ^ ^-- LB1 (no program write
#                   | | | |  | | +---- LB2  or read allowed)
#                   | | | |  | +------ BLB01 (allow bootloader
#                   | | | |  + ------- BLB02  w, !r application)
#                   | | | +----------- BLB11 (do not allow rw
#                   | | +------------- BLB12  to boot section)
#                   | +--------------- 1
#                   +----------------- 1

include Makefile-avr
