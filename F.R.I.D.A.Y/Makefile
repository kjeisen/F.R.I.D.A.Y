#
# Makefile for MPX
#

KERNEL_OBJECTS =\
kernel/irq.o\
kernel/core.o\
kernel/kmain.o\
kernel/serial.o\
kernel/comhand.o\
kernel/clock.o\
kernel/pcb.o\
kernel/r3cmd.o\
kernel/sys_call.o\
kernel/alarm.o\
kernel/heap.o

LIB_OBJECTS =\
lib/ctype.o\
lib/stdlib.o\
lib/string.o\
lib/stdio.o\
lib/struct/linked_list.o\
lib/struct/hash_map.o\
lib/math.o\
lib/time_zone.o\
lib/color.o\
lib/print_format.o

USER_OBJECTS =\
user/system.o\
user/commands.o\
user/games/bomb_catcher.o\
user/games/mine_sweeper.o\
user/games/dragon_maze.o

########################################################################
### Nothing below here needs to be changed
########################################################################

AS	= nasm
ASFLAGS = -f elf -g

CC	= clang
CFLAGS  = -std=c18 --target=i386-elf -Wall -Wextra -Werror -ffreestanding -g -Iinclude -mno-sse

ifeq ($(shell uname), Darwin)
LD	= i686-elf-ld
else
LD      = i686-linux-gnu-ld
endif
LDFLAGS = -melf_i386 -znoexecstack

OBJFILES = kernel/boot.o $(KERNEL_OBJECTS) $(LIB_OBJECTS) $(USER_OBJECTS)

all: kernel.bin

kernel.bin: $(OBJFILES) kernel/link.ld
	$(LD) $(LDFLAGS) -T kernel/link.ld -o $@ $(OBJFILES)

doc: Doxyfile
	doxygen

clean:
	rm -f $(OBJFILES) kernel.bin
