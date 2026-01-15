CC=cl65

make:
	$(CC) -O -o out\CH1.PRG -t cx16 main.c

run:
	C:\Users\Maxi\x16emu_win64-r49\x16emu -prg out\CH1.PRG -run