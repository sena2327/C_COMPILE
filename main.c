#include "9cc.h"
#include "stdio.h"

// 入力プログラム
char *user_input;
// 現在着目しているトークン
Token *token;
Node *code[100];
LVar *locals;

int main(int argc, char **argv) {
    if (argc != 2) {
      fprintf(stderr, "引数の個数が正しくありません\n");
      return 1;
    }
  
    user_input = argv[1];
    // トークナイズする
    token = tokenize(argv[1]);
    Node *node = program();
  
    printf(".intel_syntax noprefix\n");
    for (int i = 0; code[i]; i++) {
      gen(code[i]);
  }
    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
  }