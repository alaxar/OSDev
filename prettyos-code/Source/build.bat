SET flags=OS=WINDOWS
IF [%1]==[-release] (
	SET flags=OS=WINDOWS CONFIG=RELEASE
	IF [%2]==[-clang] (
		SET flags=OS=WINDOWS CONFIG=RELEASE COMPILER=CLANG
		IF [%3]==[-vs] (
			SET flags=OS=WINDOWS CONFIG=RELEASE COMPILER=CLANG MESSAGEFORMAT=VS
		)
	)
)
IF [%1]==[-clang] (
	SET flags=OS=WINDOWS COMPILER=CLANG
	IF [%2]==[-vs] (
		SET flags=OS=WINDOWS COMPILER=CLANG MESSAGEFORMAT=VS
	)
)
tools\mingw32-make FloppyImage.img %flags%

@echo off
:Loop
IF [%1]==[] GOTO Continue
	IF [%1]==[bochs] (
		cmd /c tools\bochs.bxrc
	)
	IF [%1]==[qemufloppy] (
		qemu-system-x86_64.exe -usbdevice mouse -fda \\.\a: -boot a -localtime
	)
	IF [%1]==[qemuimage] (
		qemu-system-x86_64.exe -usbdevice mouse -fda FloppyImage.img -boot a -localtime
	)
	IF [%1]==[disc] (
		tools\dd if=stage1_bootloader\boot.bin of=\\.\A: bs=512 count=1 --progress
		copy stage2_bootloader\boot2.bin A:\boot2.bin
		copy kernel\kernel.bin A:\kernel.bin
		copy user\other_userprogs\*.elf A:
		copy user\vm86\bootscr.bmp A:
	)
SHIFT
GOTO Loop
:Continue