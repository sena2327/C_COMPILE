#include "9cc.h"
#include "stdio.h"

// 抽象構文木のノード
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
  //if用
  Node* cond;
  Node* then;
  Node* els;
};
// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
  };

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
      error("代入の左辺値が変数ではありません");
  
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

static int label_id = 0;
//抽象構文木をアセンブリに書き換える
void gen(Node *node) {
    switch (node->kind){
      case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
      case ND_IF:
        int id = label_id++;
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if(node->els){
          printf("  je .Lelse%d\n", id);
          gen(node->then);
          printf("  pop rax\n");  //値を捨てる
          printf("  jmp .Lend%d\n", id);
          printf("  .Lelse%d:\n", id);
          gen(node->els);
          printf("  pop rax\n");  //値を捨てる
          printf("  .Lend%d:\n", id);
        }
        else{
          printf("  je  .Lend%d\n", id);
          gen(node->then);
          printf("  pop rax\n");  //値を捨てる
          printf("  .Lend%d:\n", id);
        }
        return;
    }
    switch (node->kind) {
        case ND_NUM:
          printf("  push %d\n", node->val);
          return;
        case ND_LVAR:
          gen_lval(node);
          printf("  pop rax\n");
          printf("  mov rax, [rax]\n");
          printf("  push rax\n");
          return;
        case ND_ASSIGN:
          gen_lval(node->lhs);
          gen(node->rhs);
      
          printf("  pop rdi\n");
          printf("  pop rax\n");
          printf("  mov [rax], rdi\n");
          printf("  push rdi\n");
          return;
    }
  
    gen(node->lhs);
    gen(node->rhs);
  
    printf("  pop rdi\n");
    printf("  pop rax\n");
  
    switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_GT:
      printf("  cmp rdi, rax\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_GE:
      printf("  cmp rdi, rax\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    }
  
    printf("  push rax\n");
  }