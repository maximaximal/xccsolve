#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

#include <xcc_parser.h>

#include <xcc_lexer.h>

#include <algorithm_x.h>

int
xcc_parse_problem() {
  int i;

  xcc_algorithm algorithm;
  xcc_problem* problem = NULL;
  memset(&algorithm, 0, sizeof(xcc_algorithm));

  xcc_algoritihm_x_set(&algorithm);

  yyscan_t scanner;
  if(xcclex_init(&scanner))
    goto DESTROY_SCANNER;

  YY_BUFFER_STATE buf = xcc_scan_string("< test test2 > [ test3 test4 ] test;", scanner);
  if(xccparse(scanner, &algorithm, &problem))
    goto DELETE_BUFFER;

  xcc_delete_buffer(buf, scanner);

  xcclex_destroy(scanner);

  if(problem)
    xcc_problem_free(problem);

  return EXIT_SUCCESS;

DELETE_BUFFER:
  xcc_delete_buffer(buf, scanner);
DESTROY_SCANNER:
  xcclex_destroy(scanner);

  if(problem)
    xcc_problem_free(problem);
  return EXIT_FAILURE;
}
