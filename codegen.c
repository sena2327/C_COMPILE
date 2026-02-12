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
  //for用
  Node* init;
  Node* inc;
  //block
  Node* body;
  Node* next;
  //関数用
  char* func_name;
  int func_len;
  Node* func_arg[6];
  int arg_len;

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

struct LVar {
  struct LVar *next;
  char *name;
  int len;
  int offset;
};
extern LVar *locals;

static int count_locals(void) {
  int n = 0;
  for (LVar *v = locals; v; v = v->next) n++;
  return n;
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
      case ND_FUNCDEF:
        printf(".globl %.*s\n", node->func_len, node->func_name);
        printf("%.*s:\n", node->func_len, node->func_name);
    
        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
    
        int nlocals = count_locals();
        int stack_size = nlocals * 8;
        // 16バイトアライン（任意だが安全）
        if (stack_size % 16) stack_size += 8;
        printf("  sub rsp, %d\n", stack_size);
    
        // 引数退避
        char *regs[6] = {"rdi","rsi","rdx","rcx","r8","r9"};
    
        // locals を配列化して最後の arg_len を params として扱う
        LVar *vars[128];
        int idx = 0;
        for (LVar *v = locals; v; v = v->next) vars[idx++] = v;
    
        for (int i = 0; i < node->arg_len && i < 6; i++) {
          LVar *param = vars[idx - 1 - i]; // param1 が最後尾
          printf("  mov [rbp-%d], %s\n", param->offset, regs[i]);
        }
    
        // 本体
        for (Node *cur = node->body; cur; cur = cur->next) {
          gen(cur);
          printf("  pop rax\n");
        }
    
        // エピローグ
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
      case ND_IF:
        int if_id = label_id++;
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if(node->els){
          printf("  je .Lelse%d\n", if_id);
          gen(node->then);
          printf("  jmp .Lend%d\n", if_id);
          printf("  .Lelse%d:\n", if_id);
          gen(node->els);
          printf("  .Lend%d:\n", if_id);
        }
        else{
          printf("  je  .Lend%d\n", if_id);
          gen(node->then);
          printf("  push 0\n"); //mainでpopするので、入れておく
          printf("  .Lend%d:\n", if_id);
        }
        return;
      case ND_WHILE:
        int while_id = label_id++;
        printf("  .Lbegin%d:\n", while_id);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", while_id);
        gen(node->then);
        printf("  push 0\n"); //mainでpopするので、入れておく
        printf("  jmp .Lbegin%d\n", while_id);
        printf("  .Lend%d:\n", while_id);
        return;
      case ND_FOR:
        int for_id = label_id++;
        gen(node->init);
        printf("  pop rax\n");
        printf("  .Lbegin%d:\n", for_id);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", for_id);
        gen(node->then);
        gen(node->inc);
        printf("  pop rax\n");
        printf("  jmp .Lbegin%d\n", for_id);
        printf("  .Lend%d:\n", for_id);
        printf("  push 0\n"); //mainでpopするので、入れておく
        return;
      case ND_BLOCK:
        Node *cur = node->body;
        while (cur) {
          gen(cur);
          printf("  pop rax\n");
          cur = cur->next;
        }
        return;
      case ND_FUNCTION:
        for(int i = 0; i < node->arg_len; i++){
          gen(node->func_arg[i]);
        }
        char* reg[6] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
        for(int i = 0; i < node->arg_len; i++){
          printf("  pop %s\n",reg[i]);
        }

        printf("  call %.*s\n", node->func_len, node->func_name);
        printf("  push rax\n"); //mainでpopするので、入れておく
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