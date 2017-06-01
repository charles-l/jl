test:
	cc -g test.c && ./a.out

jit:
	luajit ./dynasm/dynasm.lua -o jljit.c jljit.dasc
	cc -g jljit.c -o jljit
