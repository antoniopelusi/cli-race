.SILENT: clean compile compile_min run run_min

all: compile compile_min

clean:
	rm -rf bin

compile: clean
	mkdir -p bin
	gcc src/cli-race.c -o bin/cli-race
	
compile_min: clean
	mkdir -p bin
	gcc -w -s -Os -g0 -ffunction-sections -fdata-sections -Wl,--gc-sections src/min-cli-race.c -o bin/min-cli-race

run:
	./bin/cli-race

run_min:
	./bin/min-cli-race