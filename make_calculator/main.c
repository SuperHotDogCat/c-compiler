// トークナイザ機能を追加して機能を向上した場合のもの
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h> // bool型を扱えるようになる。たまげたなあ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

// 現在着目しているトークン
Token *token;
// 入力プログラムargv[1]を格納する変数
char *user_input;

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input; // locから何バイト離れているかを示すint
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // %*sでsを指定した文字数だけ表示する
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号の時には, トークンを一つ進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op){
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    // 異なる場合はtokenを進めずfalseのみ出す
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待している記号の時にトークンを一つ進める
// それ以外の時にはエラーを報告する
void expect(char *op){
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    error_at(token->str,"'%s'ではありません。", op);
  }
  token = token->next;
}

// 次のトークンが数値の場合トークンを一つ読み進める
// それ以外の場合にはエラーを報告する
int expect_number(){
  if (token->kind != TK_NUM){
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  // eofに到達したか否かを判断する
  return token->kind == TK_EOF;
}

// 再帰的に使われるのでプロト関数を定義
Node *expr();
Node *primary();
Node *mul();
Node *unary();
Node *equality();
Node *relational();
Node *add();

Node *expr(){
  // expr文法のparser
  // expr = equality
  return equality();
}

Node *equality(){
  // equality   = relational ("==" relational | "!=" relational)*
  Node *node = relational();
  for (;;){ // for(;;)を*に
    if (consume("==")){
      node = new_node(ND_EQ, node, relational());
    }
    else if (consume("!=")){
      node = new_node(ND_NE, node, relational());
    }
    else {
      return node;
    }
  }
}

Node *relational(){
  // relational = add ("<" add | "<=" add | ">" add | ">=" add)*
  Node *node = add();
  for (;;){
    if (consume("<")){
      node = new_node(ND_LT, node, add());
    }
    else if (consume("<=")){
      node = new_node(ND_LE, node, add());
    }
    else if (consume(">")){
      node = new_node(ND_LT, add(), node); // これで>をND_LTで解釈できる
    }
    else if (consume(">=")){
      node = new_node(ND_LE, add(), node); // これで>=をND_LEで解釈できる
    }
    else {
      return node;
    }
  }
}

Node *add(){
  // add = mul ("+" mul | "-" mul)*
  Node *node = mul();
  for(;;){
    if (consume("+")){
      node = new_node(ND_ADD, node, mul()); // not return new_node(ND_ADD, node, mul()); 再帰的に掘っていく
    }
    else if (consume("-")){
      node = new_node(ND_SUB, node, mul());
    }
    else {
      return node;
    }
  }
}

Node *mul(){
  // mul = primary ("*" primary | "/" primary)* の展開を制作する
  Node *node = unary();
  for (;;){
    if (consume("*")){
      node = new_node(ND_MUL, node, unary());
    }
    if (consume("/")){
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *primary(){
  // primary = num | '(' expr ')' を表現する
  if (consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }
  // そうじゃなければ数値のはず
  return new_node_num(expect_number()); // 1 * 2 + (3 + 4)の場合1はここに来る
}

Node *unary(){
  // unary = ("+" | "-")? unary | primaryらしい, なのでunaryにする
  if (consume("+")){
    return unary(); 
  }
  if (consume("-")){
    return new_node(ND_SUB, new_node_num(0), unary());
  }
  return primary();
}

int main(int argc, char **argv){
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }
  user_input = argv[1]; // 先にuser_inputにargvを渡しておかないとuser_inputがnullであるため注意
  token = tokenize(argv[1]);
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  Node *node = expr(); // Create AST
  gen(node); // compile AST to assembly

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
