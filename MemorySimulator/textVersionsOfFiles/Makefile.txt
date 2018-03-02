binaries=memorySim

make:
	gcc -o memorySim -g memSim.c
all:
	gcc -o memorySim -g memSim.c
clean:
	rm -f $(binaries) *.o
