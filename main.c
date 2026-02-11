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
    printf(".globl main\n");
    printf("main:\n");
    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // 式の評価結果としてスタックに一つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
  
    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    printf(".section .note.GNU-stack,\"\",@progbits\n");
    return 0;
  }