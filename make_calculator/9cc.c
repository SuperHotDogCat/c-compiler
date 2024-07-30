// トークナイザ機能を追加して機能を向上した場合のもの
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h> // bool型を扱えるようになる。たまげたなあ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED, //記号
  TK_NUM, // 整数トークン
  TK_EOF, // 入力の終わりを表すトークン
} TokenKind; 

typedef struct Token Token;

// トークン型, トークンを管理する。データ構造としては線形リスト
struct Token {
  TokenKind kind; //トークン型
  Token *next; //次の入力トークン
  int val; // kindがTK_NUMの場合
  char *str; // トークン文字列
};

// 現在着目しているトークン
Token *token;

// エラーを報告するための関数
// printfと同じ引数を取る

// ひとまず入力プログラムargv[1]を格納する変数
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
bool consume(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op){
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待している記号の時にトークンを一つ進める
// それ以外の時にはエラーを報告する
void expect(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op){
    error_at(token->str,"'%c'ではありません。", op);
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

// 新しいトークンを作成し, curに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;
  while (*p){
    if (isspace(*p)){
      p++;
      continue;
    }
    if (*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
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
  printf("  mov rax, %d\n", expect_number());
  while (!at_eof()){
    if (consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }
    consume('-');
    printf("  sub rax, %d\n", expect_number());
  }
  printf("  ret\n");
  return 0;
}
