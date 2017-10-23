#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "huffman.h"
#define COMPRESS 1
#define DECOMPRESS 2

void deleteTree(treeNode* p) {
	//a function that deletes(frees) the data structure
    	//used during the decompression algorithm
	if(p){
		deleteTree(p->left);
		deleteTree(p->right);
		free(p->code);
		free(p);
		p = NULL;
	}
}

void deleteList(SLLIST* sl) {
	//a function that deletes(frees) the data
    	//structure you were using during your compression algorithm
	hfcode* node = sl->head;
	while(node){
		hfcode* temp = node;
		node = node->next;
		free(temp->code);
		free(temp);	
	}
	sl->head = NULL;
	sl->tail = NULL;
}

void writeByte(char* byte, FILE* output) {
	unsigned char ch = 0;
	int i;	
	for (i = 0; i < 8; i++){
		ch = ch<<1;
		ch |= (byte[i]== '0'? 0:1);
	}
	fwrite(&ch, sizeof(char), 1, output);
}

void printList(SLLIST* sl) {
	hfcode* tmp = sl->head;
	while(tmp){
		printf("symbol is %c\n", tmp->symbol);
		printf("code is %s\n", tmp->code);
		int i = strlen(tmp->code);
		printf("%i\n",i);
		tmp = tmp->next;
	}
}

void compressFile(FILE* src, FILE* codeFile, FILE* output) {
	SLLIST sl;
	initList(&sl);
	buildLinkedList(codeFile,&sl);
	//printList(&sl);
	char string[160];
	char encodedStr[300];
	encodedStr[0] = '\0';
	while(1) {
		if (fgets(string, 160, src) == NULL) break;
		//printf("%s\n", string);
		char* str = string;
		while(*str){
			hfcode* temp = sl.head;
			while(temp){
				if (temp->symbol == *str) {
					strcat(encodedStr, temp->code);
					if (strlen(encodedStr)>=8) {
						char byte[9] = "";
						memcpy(byte, encodedStr, 8);
						char * tmp = encodedStr + 8;
						strcpy(encodedStr, tmp);
						writeByte(byte, output);			
					}
				}
				temp = temp->next;
			}
			str++;
		}
	}
	int len = strlen(encodedStr);
	if ( len > 0 && len < 8) {
		int difference = 8 - len;
		char padding[8] = "";
		memset(padding, '0', difference);
		strcat(encodedStr, padding);
		char byte[9] = "";
		memcpy(byte, encodedStr, 8);
		writeByte(byte, output);		
	}
}

void decompressFile(FILE* src, FILE* code, FILE* dest) {
	SLLIST sl;
	initList(&sl);
	buildLinkedList(code, &sl);
	hfTree tree;
	initTree(&tree);
	hfcode* curr = sl.head;
	while(curr) {
		appendTree(&tree, curr);
		curr = curr->next;
	}
	unsigned char buffer;
	char codeStr[300];
	codeStr[0]= '\0';

	while(fread(&buffer, 1, 1, src) == 1) {
		char tmp[9];
		int i;
		for (i = 0; i < 8; i++) {
			tmp[i] = (buffer>>i) & 1;
		}
		tmp[8] = '\0';
		//flip the order
		for (i = 0; i < 4; i++) {
			char t = tmp[i];
			tmp[i] = tmp[7 - i];
			tmp[7 - i] = t;
		}
		strcat(codeStr, tmp);
		
		char * tmpCode = codeStr;
		treeNode* tmpNode = tree.root;

		//convert to string
		//traverse the tree to get the symbol
		//write the symbol into output file
		while(*tmpCode && tmpNode) {
			if(tmpNode->left == NULL && tmpNode->right == NULL) {
				fwrite(&tmpNode->symbol, sizeof(char), 1, dest);
				strcpy(codeStr, tmpCode);				
				break;	
			}			
			
			if(*tmpCode == '0') {
				tmpNode = tmpNode->left;
			}
			
			if (*tmpCode == '1') {
				tmpNode = tmpNode->right;
			}
			tmpCode++;
		}
	}
	deleteTree(tree.root);	
}

void initTree(hfTree* tree) {
	tree->root = NULL;
}

void appendTree(hfTree* tree, hfcode* curr) {
	if (tree->root == NULL) {
		tree->root = (treeNode*) malloc(sizeof(treeNode));
		tree->root->parent = NULL;
		tree->root->left = NULL;
		tree->root->right = NULL;
		tree->root->symbol = 0;
	}
	char* code = curr->code;
	treeNode * tmp = tree->root;
	while(*code) {
		if(*code == '0') {
			if (tmp->left == NULL) {
				treeNode* newTreeN =(treeNode*)malloc(sizeof(treeNode));
				newTreeN->parent = tmp;
				newTreeN->left = NULL;
				newTreeN->right = NULL;
				newTreeN->code = strdup(curr->code);
				newTreeN->symbol = curr->symbol;
				tmp->left = newTreeN;
			}
			tmp = tmp->left;
		}
		if (*code == '1') {
			if (tmp->right == NULL) {
				treeNode* newTreeN = (treeNode*)malloc(sizeof(treeNode));
				newTreeN->parent = tmp;
				newTreeN->left = NULL;
				newTreeN->right = NULL;
				newTreeN->code = strdup(curr->code);
				newTreeN->symbol = curr->symbol;
				tmp->right = newTreeN;
			}
			tmp = tmp->right;
		}		
		code++;
	}
}

void buildLinkedList(FILE *codeFile, SLLIST *sl) {
	char buffer[260];
	while(1) {
		if(fgets(buffer, 260, codeFile) == NULL) {
            break;
        }
		char symbol = *buffer;
		char code[257];
		sscanf(buffer, "%*[^=]=%[^'\n']", code);
		append(sl, symbol, code);
	}
}

void append(SLLIST* sl, char character, char* code) {
	hfcode* temp = (hfcode*)malloc(sizeof(hfcode));
	temp->next = NULL;
	temp->symbol = character;
	temp->code = strdup(code);
	
	if(sl->tail == NULL) {
		sl->head = temp;
		sl->tail = temp;
	}else {
		sl->tail->next = temp;
		sl->tail = temp;
	}
}

void initList(SLLIST *sl) {
	sl->head = NULL;
	sl->tail = NULL;
}

/*	Parse the command inputs:
*
*	argv[0] is always program name
*	-c or -d:			will determine whether to compress or decompress: -c or -d 
*	<PATHTOSOURCE>:		will determine what file to compress(or decompress)(Error if it is a folder)
*	--code:				is a flag to say that the next argument is the file with the huffman codes
*	<PATHTOCODE>:		is the path to the huffman codes file
*	-p:(optional) 		to specify a destination file.
*	<PATHTODEST>:		(if -p is there) specifies the filename to compress(decompress) to.
*		Default destination path(no -p flag) is out.huf and out.dec for compress and decompress, respectively
*		if invalid path, error and exit.
*
*  NOTE:  ARGUMENTS CAN BE IN ANY ORDER, AS LONG AS EACH PATH
			IS PRECEDED BY ITS RESPECTIVE FLAG
*    This means that it could be:
*		./huffman --code <PATHTOCODE> -c <PATHTOSOURCE>    or
*		./huffman -d <PATHTOSOURCE> -p <PATHTODEST> --code <PATHTOCODE>    or
*		./huffman -p <PATHTODEST> -d <PATHTOSOURCE> --code <PATHTOCODE>
* 
*		etc..............
*
*	valid syntax shorthand:
*	./huffman <-c|-d> <PATHTOSOURCE> --code <PATHTOCODE> [-p <PATHTODEST>]
*/
int main(int argc, char const *argv[]) {
	
	FILE * code = NULL;
	FILE * src = NULL;
	FILE * dest = NULL;
	int comOrDecom = 0;
	
	if (argc != 5 && argc != 7) {
		// need to print out there is no xxx file specified, etc
		printf("Invalid input00.\n");
		return 0;
	}
	
	int i;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--code")&& i+1 < argc) {
			if ((code = fopen(argv[i+1], "r")) == NULL) {
				printf("Error opening %s\n", argv[i+1]);
				return 1;
			}	
		}
		if (!strcmp(argv[i], "-c") && i+1 < argc) {
			comOrDecom = COMPRESS;
			if ((src = fopen(argv[i+1], "r")) == NULL) {
				printf("Error opening %s\n", argv[i+1]);
				return 1;
			}	
		}
		if (!strcmp(argv[i], "-d") && i+1 < argc) {
			comOrDecom = DECOMPRESS;
			
			if ((src = fopen(argv[i+1], "rb")) == NULL) {
				printf("Error opening %s\n", argv[i+1]);
				return 1;
			}
		}
		if (!strcmp(argv[i], "-p") && i+1 < argc) {
			if ((dest = fopen(argv[i+1], "wb")) == NULL) {
				printf("Error opening %s\n", argv[i+1]);
				return 1;
			}
		}
	}
	// compress
	if (comOrDecom == COMPRESS) {
		//printf("%d\n",comOrDecom);
		if (dest == NULL) { //create a output file
			if((dest = fopen("out.huf", "wb")) == NULL) {
				printf("Error creating out.huf.\n");
				return 1;
			}
		}
		compressFile(src, code, dest);
	}
	// decompress
	if (comOrDecom == DECOMPRESS) {
		if (dest == NULL) { // create a output file
			if ((dest = fopen("out.dec", "w")) == NULL) {
				printf("Error creating out.dec.\n");
				return 1;
			}
		}
		//decompressFile(src, code, dest);
	}
	fclose(code);
	fclose(src);
	if (dest != NULL) {
        fclose(dest);
    }
	return 0;
}
