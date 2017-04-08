all:
	luajit ./dynasm/dynasm.lua -o jljit.c jljit.dasc
	cc -g jljit.c -o jljit
