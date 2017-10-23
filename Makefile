all: huffman
miniDB: huffman.c
	gcc	-g	-Wall	-Werror	-o	huffman	huffman.c
