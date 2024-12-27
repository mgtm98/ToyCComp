compile:
	gcc -g *c -o ToyCComp

run: ast runs
	
runs:
	nasm -f elf64 out.s -o out.o
	gcc -no-pie -o out out.o
	./out

ast: compile
	./ToyCComp

clean:
	rm out*