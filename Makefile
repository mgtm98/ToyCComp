compile:
	gcc -g *c -lm -o ToyCComp

run: ast runs
	
runs:
	nasm -f elf64 out.s -o out.o
	gcc -no-pie -o out lib/print.c out.o 
	./out

ast: compile
	./ToyCComp $(TEST)

clean:
	rm out*

test: compile
	csh tests/runner.csh