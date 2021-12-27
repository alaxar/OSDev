# Define OS-dependant tools
ifeq ($(OS),WINDOWS)
	RM= - del
	NASM= nasm
	CC= i586-elf-gcc
	AR= i586-elf-ar
else
	RM= rm -f
	NASM= nasm
	ifeq ($(OS),MACOSX)
		CC= i586-elf-gcc
		AR= i586-elf-ar
	else
		CC= gcc
		AR= ar
	endif
endif

ifeq ($(COMPILER),CLANG)
	CC= clang
endif

# Folders
ifeq ($(OS),WINDOWS)
	OBJDIR= ..\..\object_files\user\stdlibc
else
	OBJDIR= ../../object_files/user/stdlibc
endif

# Compiler-/Linker flags
CCFLAGS= -c -std=c11 -march=i486 -mtune=generic -Wshadow -Wstrict-prototypes -m32 -Werror -Wall -ffunction-sections -fdata-sections -O2 -nostdinc -fno-builtin -fno-stack-protector -fomit-frame-pointer -fno-common -Iinclude
ifeq ($(COMPILER),CLANG)
	CCFLAGS+= -Xclang -triple=i486-pc-unknown
	ifeq ($(MESSAGEFORMAT), VS)
		CCFLAGS+= -fdiagnostics-format=msvc
	endif
else
	CCFLAGS+= -fno-pic
endif
ARFLAGS= rs

TARGET= stdlibc.a

# Targets to build one C file to an object file
vpath %.o $(OBJDIR)
%.o: %.c
	$(CC) $< $(CCFLAGS) -o $(OBJDIR)/$@

# Dependancies
TARGETOBJ := $(patsubst %.c, %.o, $(wildcard *.c))

# Targets to build the userprog
.PHONY: clean all

all: $(TARGET)

$(TARGET): $(TARGETOBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(addprefix $(OBJDIR)/,$(TARGETOBJ))

clean:
ifeq ($(OS),WINDOWS)
	$(RM) $(OBJDIR)\*.o
else
	$(RM) $(OBJDIR)/*.o
endif