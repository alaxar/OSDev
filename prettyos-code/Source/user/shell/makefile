# Define OS-dependant tools
ifeq ($(OS),WINDOWS)
	RM= - del
	NASM= nasm
	CC= i586-elf-gcc
	LD= i586-elf-ld
else
	RM= rm -f
	NASM= nasm
	ifeq ($(OS),MACOSX)
		CC= i586-elf-gcc
		LD= i586-elf-ld
	else
		CC= gcc
		LD= ld
	endif
endif

ifeq ($(COMPILER),CLANG)
	CC= clang
endif

# Folders
ifeq ($(OS),WINDOWS)
	USERTOOLS= ..\user_tools
	STDLIBC= ..\stdlibc
	PRETTYLIB= $(USERTOOLS)\prettylib.a
	STDLIBCA= $(STDLIBC)\stdlibc.a
	OBJDIR= ..\..\object_files\user\shell
else
	USERTOOLS= ../user_tools
	STDLIBC= ../stdlibc
	PRETTYLIB= $(USERTOOLS)/prettylib.a
	STDLIBCA= $(STDLIBC)/stdlibc.a
	OBJDIR= ../../object_files/user/shell
endif

# Compiler-/Linker flags
NASMFLAGS= -Ox -f elf
CCFLAGS= -c -std=c11 -march=i486 -mtune=generic -Wshadow -Wstrict-prototypes -m32 -Werror -Wall -O2 -nostdinc -fno-builtin -fno-stack-protector -fomit-frame-pointer -fno-common -Iinclude
ifeq ($(COMPILER),CLANG)
	CCFLAGS+= -Xclang -triple=i486-pc-unknown
	ifeq ($(MESSAGEFORMAT), VS)
		CCFLAGS+= -fdiagnostics-format=msvc
	endif
else
	CCFLAGS+= -fno-pic
endif
LDFLAGS= -u _start -nostdlib --warn-common -nmagic -gc-sections -s

TARGET= SHELL.ELF

# Targets to build one C file to an object file
vpath %.o $(OBJDIR)
%.o: %.c
	$(CC) $< $(CCFLAGS) -I $(STDLIBC) -I $(USERTOOLS) -o $(OBJDIR)/$@

# Dependancies
TARGETOBJ := $(patsubst %.c, %.o, $(wildcard *.c))

# Targets to build the userprog
.PHONY: clean all

all: $(TARGET)

$(TARGET): $(TARGETOBJ) $(PRETTYLIB) $(STDLIBCA)
	$(LD) $(addprefix $(OBJDIR)/,$(TARGETOBJ)) $(STDLIBCA) $(PRETTYLIB) -T $(USERTOOLS)/user.ld -Map ../../documentation/shell.map $(LDFLAGS) -o $(TARGET)

clean:
	$(RM) *.ELF
ifeq ($(OS),WINDOWS)
	$(RM) $(OBJDIR)\*.o
else
	$(RM) $(OBJDIR)/*.o
endif