#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

typedef enum {
  tOP,
  tNUM,
  tEOF,
} TokenType;

typedef struct Token Token;
struct Token {
  char type;
  Token *next;
  int val;
  char* str;
};

Token *token;

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
    
    if (*tok=='+' || *tok=='-') {
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

char consume(void)
{
  char c = token->str[0];
  token = token->next;
  return c;
}

int main(int argc, char *argv[])
{
  if (argc!=2) {
    fprintf(stderr,"input variable is wrong\n");
    return -1;
  }
  //printf("%s\n",argv[1]);
  tokenize(argv[1]);
  
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  mov eax, %d\n",expect_number());
  
  while (token->type!=tEOF) {
    if (token->type==tOP) {
      switch (consume()) {
        case '+':
          printf("  add eax, %d\n",expect_number());
          break;
        case '-':
          printf("  sub eax, %d\n",expect_number());
          break;
      }
    }
    
  }
  
  printf("  ret\n");
  
  return 0;
}
