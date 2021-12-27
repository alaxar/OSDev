# Define OS-dependant tools
NASM= nasm
ifeq ($(OS),WINDOWS)
	RM= - del /s
	MV= cmd /c move /Y
	CC= i586-elf-gcc
	LD= i586-elf-ld
	FLOPPYIMAGE= tools/CreateFloppyImage2
else
	RM= rm -f
	MV= mv
	ifeq ($(OS),MACOSX)
		CC= i586-elf-gcc
		LD= i586-elf-ld
		FLOPPYIMAGE= tools/osx_CreateFloppyImage2
	else
		CC= gcc
		LD= ld
		FLOPPYIMAGE= tools/linux_CreateFloppyImage2
	endif
endif

ifeq ($(COMPILER),CLANG)
	CC= clang
endif

# Folders
OBJDIR= object_files
STAGE1DIR= stage1_bootloader
STAGE2DIR= stage2_bootloader
KERNELDIR= kernel
USERDIR= user

ifeq ($(OS),WINDOWS)
	SHELLDIR= $(USERDIR)\shell
	USERPROGDIR= $(USERDIR)\other_userprogs
	USERTOOLS= $(USERDIR)\user_tools
	STDLIBC= $(USERDIR)\stdlibc
else
	SHELLDIR= $(USERDIR)/shell
	USERPROGDIR= $(USERDIR)/other_userprogs
	USERTOOLS= $(USERDIR)/user_tools
	STDLIBC= $(USERDIR)/stdlibc
endif

# Dependancies
KERNEL_OBJECTS := $(patsubst %.c, %.o, $(wildcard $(KERNELDIR)/*.c $(KERNELDIR)/*/*.c)) $(patsubst %.asm, %.o, $(wildcard $(KERNELDIR)/*.asm))

# Compiler-/Linker flags
NASMFLAGS= -Ox -f elf
CCFLAGS= -c -std=c11 -march=i486 -mtune=generic -Wshadow -Wstrict-prototypes -m32 -Werror -Wall -O2 -ffreestanding -nostdinc -fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-omit-frame-pointer -fno-common -I$(KERNELDIR)
ifeq ($(COMPILER),CLANG)
	CCFLAGS+= -Wno-invalid-source-encoding -Wno-address-of-packed-member -Xclang -triple=i486-pc-unknown
	ifeq ($(MESSAGEFORMAT), VS)
		CCFLAGS+= -fdiagnostics-format=msvc
	endif
else
	CCFLAGS+= -fno-pic -fno-delete-null-pointer-checks
endif
ifeq ($(CONFIG),RELEASE)
	CCFLAGS+= -ffunction-sections -fdata-sections
endif
LDFLAGS= -nostdlib --warn-common -nmagic -gc-sections -s

# Targets to build one asm or C file to an object file
vpath %.o $(OBJDIR)
%.o: %.c
	$(CC) $< $(CCFLAGS) -o $(OBJDIR)/$@
%.o: %.asm
	$(NASM) $< $(NASMFLAGS) -I$(KERNELDIR)/ -o $(OBJDIR)/$@

# Targets to build PrettyOS
.PHONY: clean all shell other_userprogs userlibs
.SILENT: shell other_userprogs userlibs clean
.NOTPARALLEL: userlibs

all: FloppyImage.img

$(STAGE1DIR)/boot.bin: $(STAGE1DIR)/boot.asm $(STAGE1DIR)/*.inc
	$(NASM) -f bin -Ox $(STAGE1DIR)/boot.asm -I$(STAGE1DIR)/ -o $(STAGE1DIR)/boot.bin
$(STAGE2DIR)/BOOT2.BIN: $(STAGE2DIR)/boot2.asm $(STAGE2DIR)/*.inc
	$(NASM) -f bin -Ox $(STAGE2DIR)/boot2.asm -I$(STAGE2DIR)/ -o $(STAGE2DIR)/BOOT2.BIN

$(USERDIR)/vm86/VIDSWTCH.COM: $(USERDIR)/vm86/vidswtch.asm
	$(NASM) $(USERDIR)/vm86/vidswtch.asm -Ox -o $(USERDIR)/vm86/VIDSWTCH.COM
$(USERDIR)/vm86/APM.COM: $(USERDIR)/vm86/apm.asm
	$(NASM) $(USERDIR)/vm86/apm.asm -Ox -o $(USERDIR)/vm86/APM.COM

$(OBJDIR)/$(KERNELDIR)/data.o: $(KERNELDIR)/data.asm shell $(USERDIR)/vm86/VIDSWTCH.COM $(USERDIR)/vm86/APM.COM
	$(NASM) $(KERNELDIR)/data.asm $(NASMFLAGS) -I$(KERNELDIR)/ -o $(OBJDIR)/$(KERNELDIR)/data.o

$(KERNELDIR)/KERNEL.BIN: $(OBJDIR)/$(KERNELDIR)/data.o $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) $(addprefix $(OBJDIR)/,$(KERNEL_OBJECTS)) -T $(KERNELDIR)/kernel.ld -Map documentation/kernel.map -o $(KERNELDIR)/KERNEL.BIN

shell: userlibs
	$(MAKE) --no-print-directory -C $(SHELLDIR)

other_userprogs: userlibs
	$(MAKE) --no-print-directory -C $(USERPROGDIR)

userlibs:
	$(MAKE) --no-print-directory -C $(STDLIBC)
	$(MAKE) --no-print-directory -C $(USERTOOLS)

FloppyImage.img: other_userprogs $(STAGE1DIR)/boot.bin $(STAGE2DIR)/BOOT2.BIN $(KERNELDIR)/KERNEL.BIN
	$(FLOPPYIMAGE) PRETTYOS FloppyImage.img $(STAGE1DIR)/boot.bin $(STAGE2DIR)/BOOT2.BIN $(KERNELDIR)/KERNEL.BIN $(USERDIR)/sound/START.WAV $(wildcard $(USERPROGDIR)/*.ELF) $(USERDIR)/vm86/bootscr.bmp

clean:
# OS-dependant code because of different interpretation of "/" in Windows and UNIX-based OS (Linux and Mac OS X)
ifeq ($(OS),WINDOWS)
	$(RM) $(STAGE1DIR)\boot.bin
	$(RM) $(STAGE2DIR)\BOOT2.BIN
	$(RM) $(KERNELDIR)\KERNEL.BIN
	$(RM) $(OBJDIR)\$(KERNELDIR)\*.o
	$(RM) $(USERDIR)\vm86\*.COM
	$(RM) documentation\*.map
else
	$(RM) $(STAGE1DIR)/boot.bin
	$(RM) $(STAGE2DIR)/BOOT2.BIN
	$(RM) $(KERNELDIR)/KERNEL.BIN
	find $(OBJDIR)/$(KERNELDIR) -name '*.o' -delete
	$(RM) $(USERDIR)/vm86/*.COM
	$(RM) documentation/*.map
endif
	$(MAKE) --no-print-directory -C $(SHELLDIR) clean
	$(MAKE) --no-print-directory -C $(USERPROGDIR) clean
	$(MAKE) --no-print-directory -C $(STDLIBC) clean
	$(MAKE) --no-print-directory -C $(USERTOOLS) clean
