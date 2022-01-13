.PHONY = all clean

CC = i686-elf-gcc
LINKER = i686-elf-ld
ASM = nasm
CFLAGS = -c	# compiler flags
LDFLAGS = -T kernel.ld#text 0x1000# for the linker
NFLAGS = -f elf32		# for nasm assembler

C_SOURCES = $(wildcard kernel/*.c drivers/*.c include/*.c tools/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h include/*.h tools/*.h)

OBJ = $(C_SOURCES:.c=.o)

all: os-image run

run: all
	echo 'c' | bochs -f bochsrc.bxrc

os-image: boot/bootloader.bin kernel.bin #boot/initrd/initrd.tar
	 cat $^ > $@

kernel.bin: kernel/kernel_entry.o ${OBJ}
	${LINKER} -o $@ ${LDFLAGS} $^ --oformat binary

boot/initrd/initrd.bin: boot/initrd/initrd.o
	${LINKER} -o $@ ${LDFLAGS} $^ --oformat binary

kernel/kernel_entry.o: boot/kernel_entry.asm
	${ASM} ${NFLAGS} $< -o $@

boot/initrd/initrd.o: boot/initrd/initrd_ptr.asm
	${ASM} ${NFLAGS} $< -o $@

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} $< -o $@

%.bin: %.asm
	${ASM} -f bin $< -o $@

clean:
	rm *.bin kernel/*.o boot/*.bin boot/initrd/*.o drivers/*.o os-image