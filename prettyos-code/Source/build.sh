#!/bin/bash
if [ $# == "0" ] || [ $1 == "--help" ] || [ $1 == "-h" ]; then
	echo "Aufrufmöglichkeiten:"
	echo "$ ./build.sh bochs"
	echo "$ ./build.sh qemuimage"
	echo "$ ./build.sh qemufloppy:[fdd]"
	echo "-> z. B.: $ ./build.sh qemufloppy:/dev/fd0"
	echo "$ ./build.sh disk:[fdd]"
	echo "-> z. B.: $ ./build.sh disk:/dev/fd0"
	echo
	echo "Diese Argumente können beliebig kombiniert werden."
	echo "Sie werden in der angegebenen Reihenfolge ausgeführt."
	printf "Vorher wird immer \033[4mmake\033[0m ausgeführt.\\n"
	exit
fi
make
for target in $*; do
	if [ $target == "bochs" ]; then
		bochs -qf tools/bochsrc
		continue
	fi
	if [ $target == "qemuimage" ]; then
		qemu -usbdevice mouse -fda FloppyImage.img -boot a -localtime
		continue
	fi
	declare -a arr
	i=0
	for ele in $(echo $target | tr : " "); do
		arr[$i]=$ele
		let i=i+1
	done
	if [ ${arr[0]} == "qemufloppy" ]; then
		qemu -usbdevice mouse -fda ${arr[1]} -boot a -localtime
	fi
	if [ ${arr[0]} == "disk" ]; then
		dd if=FloppyImage.img of=${arr[1]} bs=18432 count=80
	fi
done
