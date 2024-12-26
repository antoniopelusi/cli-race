.SILENT: clean compile compile_min run run_min install uninstall

CC = gcc
SOURCE = src/cli-race.c
EXEC = bin/cli-race
INSTALL_PATH = /usr/local/bin

RESET = \033[0m
GREEN = \033[1;32m
BLUE = \033[1;34m

all: compile

clean:
	rm -rf bin

compile: clean
	mkdir -p bin
	gcc $(SOURCE) -o $(EXEC)

run:
	./$(EXEC)

install: all
	sudo install $(EXEC) $(INSTALL_PATH)
	echo "Installed $(GREEN)$(EXEC)$(RESET) to $(BLUE)$(INSTALL_PATH)$(RESET)"

uninstall:
	sudo rm -f $(INSTALL_PATH)/$(EXEC)
	echo "Uninstalled $(GREEN)$(EXEC)$(RESET) from $(BLUE)$(INSTALL_PATH)$(RESET)"
