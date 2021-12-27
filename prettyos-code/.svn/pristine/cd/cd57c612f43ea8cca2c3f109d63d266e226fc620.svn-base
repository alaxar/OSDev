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
	STDLIBC= ..\stdlibc
	OBJDIR= ..\..\object_files\user\user_tools
else
	STDLIBC= ../stdlibc
	OBJDIR= ../../object_files/user/user_tools
endif

# Compiler-/Linker flags
NASMFLAGS= -Ox -f elf
CCFLAGS= -c -std=c11 -march=i486 -mtune=generic -Wshadow -Wstrict-prototypes -m32 -Werror -Wall -ffunction-sections -fdata-sections -O2 -nostdinc -fno-strict-aliasing -fno-builtin -fno-stack-protector -fomit-frame-pointer -fno-common -Iinclude
CPPFLAGS= -c -std=c++11 -march=i486 -mtune=generic -Wshadow -m32 -Werror -Wall -ffunction-sections -fdata-sections -O2 -nostdinc -fno-strict-aliasing -fno-builtin -fno-stack-protector -fomit-frame-pointer -fno-common -Iinclude -fno-exceptions -fno-rtti
ifeq ($(COMPILER),CLANG)
	CCFLAGS+= -Xclang -triple=i486-pc-unknown
	CPPFLAGS+= -Wno-implicit-exception-spec-mismatch -Xclang -triple=i486-pc-unknown
	ifeq ($(MESSAGEFORMAT), VS)
		CCFLAGS+= -fdiagnostics-format=msvc
		CPPFLAGS+= -fdiagnostics-format=msvc
	endif
else
	CCFLAGS+= -fno-pic
	CPPFLAGS+= -fno-pic
endif
ARFLAGS= rs

CTARGET= prettylib.a
CPPTARGET= prettylib++.a

# Targets to build one asm or C file to an object file
vpath %.o $(OBJDIR)
%.o: %.c
	$(CC) $< $(CCFLAGS) -I $(STDLIBC) -o $(OBJDIR)/$@
%.o: %.cpp
	$(CC) $< $(CPPFLAGS) -I $(STDLIBC) -o $(OBJDIR)/$@
%.o: %.asm
	$(NASM) $< $(NASMFLAGS) -o $(OBJDIR)/$@

# Dependancies
CTARGETOBJ := $(patsubst %.c, %.o, $(wildcard *.c)) $(patsubst %.asm, %.o, $(wildcard *.asm))
CPPTARGETOBJ := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

# Targets to build the userprog
.PHONY: clean all

all: $(CTARGET) $(CPPTARGET)

$(CTARGET): $(CTARGETOBJ)
	$(AR) $(ARFLAGS) $(CTARGET) $(addprefix $(OBJDIR)/,$(CTARGETOBJ))

$(CPPTARGET): $(CPPTARGETOBJ)
	$(AR) $(ARFLAGS) $(CPPTARGET) $(addprefix $(OBJDIR)/,$(CPPTARGETOBJ))

clean:
ifeq ($(OS),WINDOWS)
	$(RM) $(OBJDIR)\*.o
else
	$(RM) $(OBJDIR)/*.o
endif