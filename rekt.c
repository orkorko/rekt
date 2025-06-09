#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define panic(X)                                                               \
  do {                                                                         \
    fprintf(stderr, "panic: %s\n", X);                                         \
    exit(1);                                                                   \
  } while (0)

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

#define BUF_CAP 100

char buf[BUF_CAP];
int buflen = 0;

#define buf_add(X) buf[buflen++] = X
#define buf_pop() buf[buflen--] = 0
#define getbuf(X) strndup(buf, buflen)

const char valid_sym[] = "!$%&*+-./:<=>?@^_~";

#define isvalidsym(c) isalpha(c) || strchr(valid_sym, c)

struct obj *mkint() {
  struct obj *ret = malloc(sizeof(struct obj));
  ret->type = INT;
  int n = 0;
  for (int i = 0; i < buflen; i++)
    n = n * 10 + ((int)buf[i] - '0');
  ret->val = n;
  return ret;
}

struct obj *mksym(char *sym) {
  struct obj *ret = malloc(sizeof(struct obj));
  ret->type = SYM;
  ret->sym = sym;
  return ret;
}

char peek() {
  int c = getc(fp);
  ungetc(c, fp);
  return c;
}

struct obj *readint(char c) {
  ungetc(c, fp);

  while (isdigit(peek()))
    buf_add(getc(fp));
  return mkint();
}

struct obj *readsym(char c) {
  ungetc(c, fp);

  while (isvalidsym(peek())) {
    buf_add(getc(fp));
  }
  return mksym(getbuf());
}

struct obj *gettok() {
  int c;

  // reset buffer
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

// TODO: refactor this mess
int set;
void pprint(struct obj *ob) {
  switch (ob->type) {
  case CONS: {
    set = 1;
    putc('(', stdout);
    pprint(car(ob));
    if (!set)
      printf(") (");
    set = 0;
    pprint(cdr(ob));
    putc(')', stdout);
    break;
  }
  
  case SYM:
    printf("%s", ob->sym);
    putc(' ', stdout);
    break;

  case INT:
    printf("%d", ob->val);
    putc(' ', stdout);
    break;

  case NIL:
    printf("nil");
    break;

  case LPAREN:
    printf("(");
    break;

  case RPAREN:
    printf(")");
    break;

  case QUOTE:
    printf("'");
    break;

  default:
    printf("<illegal>");
    break;
  }
}

int main() {
  setinput(stdin);
      struct obj *ob = gettok();
  for (;;) {
    if (ob == nil)
      return 0;
    pprint(ob);
  }
  return 0;
}
