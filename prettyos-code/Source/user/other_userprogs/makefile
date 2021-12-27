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
	PRETTYLIBPP= $(USERTOOLS)\prettylib++.a
	STDLIBCA= $(STDLIBC)\stdlibc.a
	OBJDIR= ..\..\object_files\user\other_userprogs
else
	USERTOOLS= ../user_tools
	STDLIBC= ../stdlibc
	PRETTYLIB= $(USERTOOLS)/prettylib.a
	PRETTYLIBPP= $(USERTOOLS)/prettylib++.a
	STDLIBCA= $(STDLIBC)/stdlibc.a
	OBJDIR= ../../object_files/user/other_userprogs
endif

# Compiler-/Linker flags
NASMFLAGS= -Ox -f elf
CCFLAGS= -c -std=c11 -march=i486 -mtune=generic -Wshadow -Wstrict-prototypes -m32 -Werror -Wall -O2 -ffunction-sections -fdata-sections -nostdinc -fno-strict-aliasing -fno-builtin -fno-stack-protector -fomit-frame-pointer -fno-common -Iinclude
CPPFLAGS= -c -std=c++11 -march=i486 -mtune=generic -Wshadow -m32 -Werror -Wall -O2 -ffunction-sections -fdata-sections -nostdinc -fno-strict-aliasing -fno-builtin -fno-stack-protector -fomit-frame-pointer -fno-common -Iinclude -fno-exceptions -fno-rtti
ifeq ($(COMPILER),CLANG)
	CCFLAGS+= -Wno-invalid-source-encoding -Xclang -triple=i486-pc-unknown
	CPPFLAGS+= -Wno-invalid-source-encoding -Xclang -triple=i486-pc-unknown
	ifeq ($(MESSAGEFORMAT), VS)
		CCFLAGS+= -fdiagnostics-format=msvc
		CPPFLAGS+= -fdiagnostics-format=msvc
	endif
else
	CCFLAGS+= -fno-pic
	CPPFLAGS+= -fno-pic -lgcc
endif
LDFLAGS= -u _start -nostdlib --warn-common -nmagic -gc-sections -s

# Targets to build an executable from a source file
%.ELF: %.c $(STDLIBCA) $(PRETTYLIB)
	$(CC) $< $(CCFLAGS) -I $(STDLIBC) -I $(USERTOOLS) -o $(OBJDIR)/$<.o
	$(LD) $(OBJDIR)/$<.o $(STDLIBCA) $(PRETTYLIB) -T $(USERTOOLS)/user.ld $(LDFLAGS) -Map ../../documentation/user_$@.map -o $@

%.ELF: %.cpp $(STDLIBCA) $(PRETTYLIB) $(PRETTYLIBPP)
	$(CC) $< $(CPPFLAGS) -I $(STDLIBC) -I $(USERTOOLS) -o $(OBJDIR)/$<.o
	$(LD) $(OBJDIR)/$<.o $(PRETTYLIBPP) $(STDLIBCA) $(PRETTYLIB) -T $(USERTOOLS)/user.ld $(LDFLAGS) -Map ../../documentation/user_$@.map -o $@

# Dependancies
TARGETEXE := $(patsubst %.c, %.ELF, $(wildcard *.c)) $(patsubst %.cpp, %.ELF, $(wildcard *.cpp))

# Targets to build the userprog
.PHONY: clean all

all: $(TARGETEXE)

clean:
	$(RM) *.ELF
ifeq ($(OS),WINDOWS)
	$(RM) $(OBJDIR)\*.o
else
	$(RM) $(OBJDIR)/*.o
endif