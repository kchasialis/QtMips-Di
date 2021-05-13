#!/bin/bash
asm_file=$1
fname=$(basename -- "$asm_file")
fname="${fname%.*}"
obj_file=$fname".o"
mips-elf-gcc -ggdb -c $asm_file -o $obj_file
mips-elf-gcc -ggdb -nostdlib -nodefaultlibs -nostartfiles $obj_file -o $fname
