#!/usr/bin/env bash

nasm -f elf64 ./src/asm/parasite.s -o ./src/asm/parasite.o
objdump -d ./src/asm/parasite.o | grep '[0-9a-f]:' | cut -f2 | tr -s ' ' | tr -d '\n' | tr ' ' '\n' > ./src/asm/hexdump
# cat hexdump | tr '\n' ' ' && echo 
rm -f ./src/asm/payload
for f in $(cat ./src/asm/hexdump); do
	echo -ne "\\\\x$f" >> ./src/asm/payload
done
echo -ne $(cat ./src/asm/payload) > ./src/asm/opcode
rm -f ./src/asm/hexdump
rm -f ./src/asm/payload
# echo "File 'opcode' generated successfully"