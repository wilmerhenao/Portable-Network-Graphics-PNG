CXX=gcc
FLAGS=-std=c99 -Wall -Wextra -Werror
pngdbg:main.c png_debug.c
	${CXX} main.c CRCChecks.c png_debug.c ${FLAGS} -o pngdbg
