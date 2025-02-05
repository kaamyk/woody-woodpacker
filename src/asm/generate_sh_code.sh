#!/usr/bin/env bash

nasm -f elf64 parasite.s -o parasite.o
objdump -d parasite.o | grep '[0-9a-f]:' | cut -f2 | tr -s ' ' | tr -d '\n' | tr ' ' '\n' > hexdump
cat hexdump | tr '\n' ' ' && echo 
rm -f payload
for f in $(cat hexdump); do
	echo -ne "\\\\x$f" >> payload
done
echo -ne $(cat payload) > opcode
rm -f hexdump
rm -f payload
echo "File 'opcode' generated successfully"