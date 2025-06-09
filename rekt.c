#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define panic(X)                                                               \
  do {                                                                         \
    fprintf(stderr, "panic: %s\n", X);                                         \
    exit(1);                                                                   \
  } while (0)

#define BUF_CAP 100

enum {
  ARCH_I64,
  ARCH_RISCV,
  ARCH_UNKOWN,
};

enum {
  SYM,
  INT,
  CONS,
  NIL,
  LPAREN,
  RPAREN,
  QUOTE,
};

struct obj {
  int type;
  union {
    char *sym;
    int val;
    struct {
      struct obj *car;
      struct obj *cdr;
    };
  };
};

#define car(X) X->car
#define cdr(X) X->cdr

FILE *fp;
struct obj *all_symbols;

void setinput(FILE *f) { fp = f; }

struct obj *cons(struct obj *h, struct obj *t) {
  struct obj *ret = malloc(sizeof(struct obj));
  if (ret == NULL)
    panic("failed to malloc");
  ret->type = CONS;
  car(ret) = h;
  cdr(ret) = t;
  return ret;
}

struct obj *nil = &(struct obj){NIL};
struct obj *lparen = &(struct obj){LPAREN};
struct obj *rparen = &(struct obj){RPAREN};
struct obj *quote = &(struct obj){QUOTE};

char buf[100]; int buflen = 0;

#define buf_add(X) buf[buflen++] = X
#define buf_pop() buf[buflen--] = 0
#define getbuf(X) strndup(buf, buflen)

const char valid_sym[] = "!$%&*+-./:<=>?@^_~";

#define isvalidsym(c) isalpha(c) || strchr(valid_sym, c)

struct obj *mkint() {
  struct obj *ret = malloc(sizeof(struct obj));
  ret->type = INT;
  int n = 0;
  for(int i=0; i<buflen; i++)
    n = n * 10 + ((int)buf[i] - '0');
  ret->val = n;
  return ret;
}

struct obj *mksym(char *sym) {
  struct obj *ret = malloc(sizeof(struct obj));
  ret->type = SYM;
  ret->sym  = sym;
  return ret;
}

char peek() {
  int c = getc(fp);
  ungetc(c, fp);
  return c;
}

struct obj *readint(char c) {
  ungetc(c, fp);
  while(isdigit(peek())) 
    buf_add(getc(fp));
  return mkint();
}

struct obj *readsym(char c) {
  ungetc(c, fp);
  while(isvalidsym(peek())) 
    buf_add(getc(fp));
  return mksym(getbuf());
}

struct obj *gettok() {
  int c;

  buflen = 0;

  do {
  } while((isspace(c = getc(fp))));

  if(c == EOF) return NULL;
  else if(c == '(') return lparen;
  else if(c == ')') return rparen;
  else if(c == '\'') return quote;
  else if(isdigit(c)) return readint(c);
  else if(isvalidsym(c)) return readsym(c);
  else panic("can't parse this symbol");
}

void dispob(struct obj *ob) {
  switch(ob->type) {
    case SYM:
      printf("<symbol> [%s]\n", ob->sym);
      break;
    case INT:
      printf("<int> [%d]\n", ob->val);
      break;
    case CONS:
      printf("<cons>\n");
      break;
    case NIL:
      printf("<nil>\n");
      break;
    case LPAREN:
      printf("<lparen>\n");
      break;
    case RPAREN:
      printf("<rparen>\n");
      break;
    case QUOTE:
      printf("<quote>\n");
      break;
    default:
      printf("<illegal>\n");
      break;
  }
}

int main() {
  setinput(stdin);
  for(;;) {
      struct obj *ob = gettok();
      if(!ob) return 0;
      dispob(ob);
  }
  return 0;
}
