CC=cl65
CFLAGS=-O -o out\CH1.PRG -t cx16
SRC=src
SRCS=$(wildcard $(SRC)/*.c)

make:
	$(CC) $(CFLAGS) $(SRCS)

.PHONY: run
run:
# spent a few hours figuring out why this was throwing a bunch of errors
# it's just vscode being vscode, it works fine running it from command prompt
# you need x16emu on the path enviroment variable btw
	x16emu -prg out\CH1.PRG -run

.PHONY: clean
clean:
	del out\*.o out\*.s out\*.PRG