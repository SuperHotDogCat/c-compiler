#include <stdbool.h>
#include <stdlib.h>

// トークンの種類
typedef enum {
  TK_RESERVED, //記号
  TK_IDENT, // 識別子
  TK_NUM, // 整数トークン
  TK_EOF, // 入力の終わりを表すトークン
  TK_RETURN, // return文を意味するトークン
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
bool is_alnum(char c);

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
  ND_LVAR,   // ローカル変数
  ND_ASSIGN, // = 
  ND_RETURN, // return
} NodeKind;

typedef struct Node Node;// 抽象構文木のノードの型

struct Node {
  NodeKind kind; // ノードの型  
  Node *lhs;     // 左辺  
  Node *rhs;     // 右辺  
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
};

// codegen.cに追加
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
// ASTからStack machineへのコンパイル
void gen(Node *node);
// codegenなどで使われるglobal変数 extern 修飾子は変数が別のファイルで定義されていることを示します。
// 現在着目しているトークン
extern Token *token;
// 入力プログラムargv[1]を格納する変数
extern char *user_input;
Token *consume_ident(); // identの処理のための関数

// 再帰的に使われるのでプロト関数を定義
Node *program();
Node *stmt();
Node *expr();
Node *assign();
Node *primary();
Node *mul();
Node *unary();
Node *equality();
Node *relational();
Node *add();
//codeを格納する場所
extern Node *code[100];
