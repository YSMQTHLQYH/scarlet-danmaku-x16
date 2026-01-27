CC=cl65

make:
	$(CC) -O -o out\CH1.PRG -t cx16 main.c x16.c zp_utils.c math_tests.c zsm_player.c text.c profiler.c bitmap_layer.c

.PHONY: run
run:
# spent a few hours figuring out why this was throwing a bunch of errors
# it's just vscode being vscode, it works fine running it from command prompt
# you need x16emu on the path enviroment variable btw
	x16emu -prg out\CH1.PRG -run