
typedef struct hfcode{
	char symbol;
	char* code;
	struct hfcode* next;

}hfcode;

typedef struct _treeNode{
	struct _treeNode* parent;
	struct _treeNode* left;
	struct _treeNode* right;
	char *code;
	char symbol;

}treeNode;

typedef struct _hfTree{
	treeNode* root;
}hfTree;

typedef struct SLLIST{
	hfcode * head;
	hfcode * tail;
}SLLIST;


void writeByte(char* byte, FILE* output);
void compressFile(FILE* src, FILE* code, FILE* output);
void initList(SLLIST *sl);
void buildLinkedList(FILE *code, SLLIST *sl);
void append(SLLIST* sl, char character, char* code);
void decompressFile(FILE* src, FILE* code, FILE* dest);
void initTree(hfTree* tree);
void appendTree(hfTree* tree, hfcode* node);
void deleteTree(treeNode* p);
void deleteList(SLLIST* sl);
