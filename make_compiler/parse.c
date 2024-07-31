#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "9cc.h"

bool startswith(char *p, char *q){
  return memcmp(p, q, strlen(q)) == 0;
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
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if ('a' <= *p && *p <= 'z'){
      cur = new_token(TK_IDENT, cur, p++, 1);
      cur->len = 1;
      continue;
    }
    if (strchr("+-*/()<>=;", *p)){
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q; // pにはstrtolをすると終了後のaddressが入ってる
      continue;
    }

    error_at(p, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
