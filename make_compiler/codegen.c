#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "9cc.h"

// 新しいトークンを作成し, curに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// ノード作成関数を定義する
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val){
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

void gen_lval(Node *node){
  // gen_lvalは、与えられたノードが変数を指しているときに、
  // その変数のアドレスを計算して、それをスタックにプッシュします。
  // それ以外の場合にはエラーを表示します。
  if (node->kind != ND_LVAR){
    error("代入の左辺値が変数ではありません");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

// ASTからStack machineへのコンパイル
void gen(Node *node){
  switch (node->kind){
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

  gen(node->lhs); // 左辺compile
  gen(node->rhs); // 右辺compile
  // ここから下は右辺と左辺ができた前提で話が進む
  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->kind){
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
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
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
  }
  printf("  push rax\n");
}

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
/*
consume_identのimplement*/
Token *consume_ident(){
  if (token->kind != TK_IDENT){
    return NULL;
  }
  Token *return_token = token;
  token = token->next;
  return return_token;
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

Node *program(){
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

Node *stmt(){
  Node *node = expr();
  expect(";");
  return node;
}

Node *expr(){
  // expr文法のparser
  // expr = assign
  return assign();
}

Node *assign(){
  Node *node = equality();
  if (consume("=")){
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
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
  // mul = unary ("*" unary | "/" unary)* の展開を制作する
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
  // primary = num | ident | '(' expr ')' を表現する
  if (consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }
  /*if (token->kind == TK_IDENT) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (token->str[0] - 'a' + 1) * 8;
    token = token->next;
    return node;
  }*/
  Token *token = consume_ident();
  if (token){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (token->str[0] - 'a' + 1) * 8;
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
