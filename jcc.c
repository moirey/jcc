#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

typedef enum {
  tOP,
  tNUM,
  tEOF,
} TokenType;

typedef enum {
  nADD,
  nSUB,
  nMUL,
  nDIV,
  nNUM,
} NodeType;

typedef struct Node Node;
  
struct Node {
  NodeType type;
  Node *ln;
  Node *rn;
  int val;
};

typedef struct Token Token;
struct Token {
  char type;
  Token *next;
  int val;
  char* str;
};

Node* new_node(NodeType type, Node *ln, Node *rn) {
  Node *node = calloc(1, sizeof(Node));
  node->type = type;
  node->ln = ln;
  node->rn = rn;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->type = nNUM;
  node->val = val;
  return node;
}

Token *token;
char *user_input;

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); 
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

Token* create_new_token(TokenType type,char *p,Token* curr)
{
  Token *t = (Token*)malloc(sizeof(Token));
  t->next = NULL;
  curr->next = t;
  t->str = p;
  t->type = type;
  
  return t;
}

void tokenize(char* tok)
{
  Token head;
  head.next = NULL;
  Token *curr = &head;

  while (*tok) {    
    //printf("tok-%c\n",*tok);
    if (isspace(*tok)) {
      // ignore space
      tok++;
      continue;
    }
    
    if (*tok=='+' || *tok=='-' || *tok=='*' || *tok=='/' || *tok=='(' || *tok==')') {
      curr = create_new_token(tOP,tok++,curr);
      continue;
    }
    
    if (isdigit(*tok) && *tok!='0') {
      curr = create_new_token(tNUM,tok,curr);
      curr->val = strtol(tok,&tok,10);
      continue;
    }
    
    error("can't tokenize");
  }
  
  curr = create_new_token(tEOF,tok,curr);
  
  token = head.next;
}

int expect_number(void)
{
  if (token->type!=tNUM) {
    error("token is not number, type %d, str %s",token->type,token->str);  
  }
  int val = token->val;
  token = token->next;
  return val;
}

int is_op(char op)
{
  if (token->type!=tOP || token->str[0]!=op) return 0;
  
  token = token->next;
  return 1;
}

/* ---------------
 * expr()  = mul ('+' mul | '-' mul)*
 * mul() = term ( '*' term | '/' term )*
 * term() = num | ( expr )
 */
Node* expr();

Node* term()
{  
  if (is_op('(')) {
    Node *n = expr();
    if (is_op(')')) return n;
    error("end has to ).");
  }
  
  return new_node_num(expect_number());
}

Node* mul()
{
  Node* n = term();
  //printf("%s , n->type %d,n->val %d\n",__func__,n->type,n->val);
  for (;;) {
    if (is_op('*')) n=new_node(nMUL,n,term());
    else if (is_op('/')) n=new_node(nDIV,n,term());
    else return n;
  }
}

Node* expr()
{
  Node *n = mul();
  //printf("%s , n->type %d,n->val %d\n",__func__,n->type,n->val);
  for (;;) {
    if (is_op('+')) n = new_node(nADD,n,mul());
    else if (is_op('-')) n = new_node(nSUB,n,mul());
    else return n;
  }
}

void gen(Node *node) {
  
  if (node->type == nNUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->ln);
  gen(node->rn);

  printf("  pop edi\n");
  printf("  pop eax\n");

  switch (node->type) {
  case nADD:
    printf("  add eax, edi\n");
    break;
  case nSUB:
    printf("  sub eax, edi\n");
    break;
  case nMUL:
    printf("  imul eax, edi\n");
    break;
  case nDIV:
  /*mov edx, 0        ; clear dividend
mov eax, 0x8003   ; dividend
mov ecx, 0x100    ; divisor
div ecx           ; EAX = 0x80, EDX = 0x3 
*/
    printf("  mov edx, 0\n");    
    printf("  div edi\n");
    break;
  }

  printf("  push eax\n");
}

int main(int argc, char *argv[])
{
  if (argc!=2) {
    fprintf(stderr,"input variable is wrong\n");
    return -1;
  }
  user_input = argv[1];
  tokenize(user_input);
    

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
    
  gen( expr() );
  
  printf("  pop eax\n");
  printf("  ret\n");

  
  return 0;
}
