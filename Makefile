build:
	gcc process_generator.c -lm -o process_generator.out
	gcc clk.c -lm -o clk.out
	gcc scheduler_SJF.c -lm -o scheduler_SJF.out
	gcc scheduler_HPF.c -lm -o scheduler_HPF.out
	gcc scheduler_RR.c -lm -o scheduler_RR.out
	gcc scheduler_MLFL.c -lm -o scheduler_MLFL.out
	gcc process.c -lm -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out

run:
	./process_generator.out processes.txt -sch 3 -q 2

all: clean build run


