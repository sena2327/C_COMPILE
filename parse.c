#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    //関数よう
    char* func_name;
    int func_len;
  };

struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
  };

// ローカル変数の型
struct LVar {
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
  };

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    if (strlen(op) != token->len || memcmp(token->str, op, token->len))
      return false;
    if (token->kind != TK_RETURN 
      && token->kind != TK_IF
      && token->kind != TK_ELSE 
      && token->kind != TK_WHILE
      && token->kind != TK_FOR
      && token->kind != TK_BLOCK
      && token->kind != TK_RESERVED)
      return false;
    token = token->next;
    return true;
  }

Token *consume_ident(void) {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
      error("'%s'ではありません", op);
    token = token->next;
  }

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
  }

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = 1;
    cur->next = tok;
    return tok;
  }

//与えられた文字がトークンを構成する文字かを判定
int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}
 
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if(!strncmp(p, "return", 6) && !is_alnum(p[6])){
      cur = new_token(TK_RETURN, cur, p);
      cur->len = 6; 
      p+=6;
      continue;
    }
    else if(!strncmp(p, "if", 2) && !is_alnum(p[2])){
      cur = new_token(TK_IF, cur, p);
      cur->len = 2; 
      p+=2;
      continue;
    }
    else if(!strncmp(p, "else", 4) && !is_alnum(p[4])){
      cur = new_token(TK_ELSE, cur, p);
      cur->len = 4; 
      p+=4;
      continue;
    }
    else if(!strncmp(p, "while", 5) && !is_alnum(p[5])){
      cur = new_token(TK_WHILE, cur, p);
      cur->len = 5; 
      p+=5;
      continue;
    }
    else if(!strncmp(p, "for", 3) && !is_alnum(p[3])){
      cur = new_token(TK_FOR, cur, p);
      cur->len = 3; 
      p+=3;
      continue;
    }
    else if(strchr("{}", *p)){
      cur = new_token(TK_BLOCK, cur, p++);
      continue;
    }
    else if ('a' <= *p && *p <= 'z') {
      char *q = p;
      p++;
      while (is_alnum(*p)) p++;
      cur = new_token(TK_IDENT, cur, q);
      cur->len = p - q;
      continue;
    }
    else if(!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "<=", 2) || !strncmp(p, ">=", 2)){
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p+=2;
      continue;
    }
    else if (strchr("+-*/()<>=;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error("トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}
  
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
      if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
        return var;
    return NULL;
  }

Node *expr();


Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    //引数なしの関数
    if(consume("(")){
      expect(")");
      node->kind = ND_FUNCTION;
      node->func_name = tok->str;
      node->len = tok->len;
    }
    //変数扱い
    else{
      node->kind = ND_LVAR;
      LVar *lvar = find_lvar(tok);
      if (lvar) {
        node->offset = lvar->offset;
      } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = locals ?  locals->offset + 8 : 8 ;
        node->offset = lvar->offset;
        locals = lvar;
      }
    }
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_GT, node, add());
    else if (consume(">="))
      node = new_node(ND_GE, node, add());
    else
      return node;
  }
}
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *assign() {
    Node *node = equality();
    if (consume("="))
      node = new_node(ND_ASSIGN, node, assign());
    return node;
  }
  
Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } 
  else if (consume("{")) {
    Node *head = NULL;
    Node *cur = NULL;
    while(!consume("}")){
      Node *s = stmt();
      if (!head) {
        head = s;
        cur = s;
      } else {
        cur->next = s;
        cur = s;
      }
    }
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->body = head;
    return node;
  } 
  else if(consume("if")){
    if (consume("(")) {
      node = calloc(1, sizeof(Node));
      node->kind = ND_IF;
      node->cond = expr();
      expect(")");
      node->then = stmt();
      if(consume("else")){
      node->els = stmt();
      }
      return node;
    }
    else{
      error_at(token->str, "ifの使い方が正しくありません");
    }
  }
  else if(consume("while")){
    if (consume("(")) {
      node = calloc(1, sizeof(Node));
      node->kind = ND_WHILE;
      node->cond = expr();
      expect(")");
      node->then = stmt();
      return node;
    }
    else{
      error_at(token->str, "whileの使い方が正しくありません");
    }
  }
  else if(consume("for")){
    if (consume("(")) {
      node = calloc(1, sizeof(Node));
      node->kind = ND_FOR;
      if (consume(";")) node->init = NULL; else {node->init = expr(); expect(";");}
      if (consume(";")) node->cond = NULL; else { node->cond = expr(); expect(";"); }
      if (consume(")")) node->inc = NULL; else { node->inc = expr(); expect(")"); }
      node->then = stmt();
      return node;
    }
    else{
      error_at(token->str, "forの使い方が正しくありません");
    }
  }
  else {
    node = expr();
  }


  if (!consume(";"))
    error_at(token->str, "';'ではありません");
  return node;
}


Node *program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}