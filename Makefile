build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler_SJF.c -o scheduler_SJF.out
	gcc scheduler_HPF.c -o scheduler_HPF.out
	gcc scheduler_RR.c -o scheduler_RR.out
	gcc scheduler_MLFL.c -o scheduler_MLFL.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
