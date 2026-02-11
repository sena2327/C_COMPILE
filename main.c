#include "9cc.h"
#include "stdio.h"

// 入力プログラム
char *user_input;
// 現在着目しているトークン
Token *token;

int main(int argc, char **argv) {
    if (argc != 2) {
      fprintf(stderr, "引数の個数が正しくありません\n");
      return 1;
    }
  
    user_input = argv[1];
    // トークナイズする
    token = tokenize(argv[1]);
    Node *node = expr();
  
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    // 抽象構文木を下りながらコード生成
    gen(node);
  
    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
  }