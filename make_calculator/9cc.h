#include <stdbool.h>
#include <stdlib.h>

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
  int len; // 文字列長さ
};

// エラー出力関数
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

// 新しいトークンを作成し, curに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

// parse.cに格納
bool startswith(char *p, char *q);
Token *tokenize(char *p);

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +  
  ND_SUB, // -  
  ND_MUL, // *  
  ND_DIV, // /  
  ND_NUM, // 整数
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // < or >
  ND_LE,  // <= or >=
} NodeKind;

typedef struct Node Node;// 抽象構文木のノードの型

struct Node {
  NodeKind kind; // ノードの型  
  Node *lhs;     // 左辺  
  Node *rhs;     // 右辺  
  int val;       // kindがND_NUMの場合のみ使う
};

// codegen.cに追加
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
// ASTからStack machineへのコンパイル
void gen(Node *node);