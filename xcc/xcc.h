#ifndef XCC_H
#define XCC_H

#include <stdbool.h>
#include <stdint.h>

typedef int32_t xcc_link;
typedef xcc_link xcc_color;
typedef char* xcc_name;

typedef struct xcc_item {
  xcc_link item;
  union {
    struct {
      xcc_link slack;
      xcc_link bound;
    };
    xcc_color color;
  };
} xcc_item;

typedef struct xcc_header_node {
  xcc_name name;
  xcc_link llink, rlink;
} xcc_header_node;

typedef struct xcc_node {
  xcc_link ulink, dlink;
  xcc_color color;
} xcc_node;

typedef struct xcc_problem {
  xcc_header_node header_node;
  xcc_node node;
} xcc_problem;

xcc_problem*
xcc_problem_allocate();

void
xcc_problem_free(xcc_problem* p);

xcc_link
xcc_option_from_ident(xcc_problem* p, const char* ident);

#endif
