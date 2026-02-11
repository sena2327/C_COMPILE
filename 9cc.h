#ifndef CC9_H
#define CC9_H

#include <stdbool.h>

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  //!=
  ND_GE,  // >=
  ND_GT,  // >
  ND_LE,  // <=
  ND_LT,  // <
  ND_NUM, // 整数
  ND_ASSIGN, // =
  ND_LVAR,   // ローカル変数
  ND_RETURN,  //return
  ND_IF,  //if
} NodeKind;

typedef struct Node Node;

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_IDENT,    // 識別子
    TK_EOF,      // 入力の終わりを表すトークン
    TK_RETURN,  //return
    TK_IF,  //if
    TK_ELSE,  //else
  } TokenKind;
  
typedef struct Token Token;
typedef struct LVar LVar;

// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

// ローカル変数
extern LVar *locals;

extern Node *code[100];

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...);

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...);

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op);

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op);

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number();

Token *tokenize(char *p);
Node *program();
void gen(Node *node);

#endif