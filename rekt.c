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
  NIL,
  CONS,
  SYM,
  INT,
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

#define car(X)  (X->car)
#define cdr(X)  (X->cdr)
#define cadr(X) ((X->cdr)->car)
#define cddr(X) ((X->cdr)->cdr)

struct obj *nil = &(struct obj){NIL};
struct obj *lparen = &(struct obj){LPAREN};
struct obj *rparen = &(struct obj){RPAREN};
struct obj *quote = &(struct obj){QUOTE};

FILE *ifp, *ofp;
#define BUF_CAP 100
char buf[BUF_CAP];
int buflen;

void setinput(FILE *f)  { ifp = f; }
void setoutput(FILE *f) { ofp = f; }

void buf_add(char c) { if(buflen < BUF_CAP - 1) buf[buflen++] = c; }
void buf_pop(void) { buf[buflen--] = 0; }
char *getbuf(void) { return strndup(buf, buflen); }

const char valid_sym[] = "!$%&*+-./:<=>?@^_~";
int isvalidsym(char c) { return isalpha(c) || strchr(valid_sym, c); }

/* Tokenizer */

struct obj *cons(struct obj *h, struct obj *t) {
  struct obj *ret = malloc(sizeof(struct obj));
  if (ret == NULL)
    panic("failed to malloc");
  ret->type = CONS;
  car(ret) = h;
  cdr(ret) = t;
  return ret;
}

struct obj *mkint(void) {
  struct obj *ret = malloc(sizeof(struct obj));
  ret->type = INT;
  int n = 0;
  for (int i = 0; i < buflen; i++)
    n = n * 10 + ((int)buf[i] - '0');
  ret->val = n;
  return ret;
}

struct obj *mksym(void) {
  struct obj *ret = malloc(sizeof(struct obj));
  ret->type = SYM;
  ret->sym = getbuf();
  return ret;
}

char peek(void) {
  int c = getc(ifp);
  ungetc(c, ifp);
  return c;
}

struct obj *readint(char c) {
  ungetc(c, ifp);
  while (isdigit(peek()))
    buf_add(getc(ifp));
  return mkint();
}

struct obj *readsym(char c) {
  ungetc(c, ifp);
  while (isvalidsym(peek()))
    buf_add(getc(ifp));
  return mksym();
}

struct obj *gettok(void) {
  int c; buflen = 0;

  do {
    // eat whitespace
  } while ((isspace(c = getc(ifp))));

  if (c == EOF) return nil;
  else if (c == '(') return lparen;
  else if (c == ')') return rparen;
  else if (c == '\'') return quote;
  else if (isdigit(c)) return readint(c);
  else if (isvalidsym(c)) return readsym(c);
  else panic("can't parse this symbol");
}

struct obj *readlist();
struct obj *readexpr(struct obj *ob);
struct obj *reverse(struct obj *ob);
void pprint(struct obj *ob);

struct obj *reverse(struct obj *p) {
  struct obj *ret = nil;
  while (p != nil) {
    struct obj *head = p;
    p = cdr(p);
    cdr(head) = ret;
    ret = head;
  }
  return ret;
}

struct obj *readlist() {
  struct obj *acc = nil;
  struct obj *ob;
  while ((ob = gettok()) != rparen) {
    if(ob == nil) panic("missing rparen");
    acc = cons(readexpr(ob), acc);
  }
  return reverse(acc);
}

struct obj *readexpr(struct obj *ob) {
  return ob == lparen ? readlist() : ob;
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
  for (;;) {
    struct obj *ob = readexpr(gettok());
    if (ob == nil)
      return 0;
    pprint(ob);
  }
  return 0;
}
