cat.exe: cat.o
	ld -m elf_i386 cat.o -o cat.exe

cat.o: cat.S
	as --32 cat.S -o cat.o

clean:
	rm -f *.exe *.o