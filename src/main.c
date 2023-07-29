/*
    XCCSolve - Toolset to solve exact cover problems and extensions
    Copyright (C) 2021-2023  Maximilian Heisinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcc/algorithm.h>
#include <xcc/log.h>
#include <xcc/ops.h>
#include <xcc/parse.h>
#include <xcc/xcc.h>

static void
print_help() {
  printf("xccsolve -- solve XCC problems using different algorithms\n");
  printf("OPTIONS:\n");
  printf("  -h\t\tprint help\n");
  printf("  -p\t\tprint selected options\n");
  printf("  -e\t\tenumerate all solutions\n");
  printf("ALGORITHM SELECTORS:\n");
  printf("  --naive\tuse naive in-order for i selection\n");
  printf("  --mrv\t\tuse MRV for i selection (default)\n");
  printf("  --smrv\tuse slack-aware MRV for i selection (default for M)\n    "
         "    \t    (see answer to ex. 166, p. 271)\n");
  printf("  -x\t\tuse Algorithm X\n");
  printf("  -c\t\tuse Algorithm C\n");
  printf("  -m\t\tuse Algorithm M\n");
  printf("  -k\t\tcall external binary to solve with SAT\n    \t\t    (Knuth's "
         "trivial encoding)\n");
}

static void
parse_cli(xcc_config* cfg, int argc, char* argv[]) {
  int c;

  int sel[5];
  memset(sel, 0, sizeof(sel));

  struct option long_options[] = {
    { "verbose", no_argument, &cfg->verbose, 1 },
    { "help", no_argument, 0, 'h' },
    { "print", no_argument, 0, 'p' },
    { "enumerate", no_argument, 0, 'e' },
    { "naive", no_argument, &sel[0], XCC_ALGORITHM_NAIVE },
    { "mrv", no_argument, &sel[1], XCC_ALGORITHM_MRV },
    { "smrv", no_argument, &sel[1], XCC_ALGORITHM_MRV },
    { "x", no_argument, &sel[2], XCC_ALGORITHM_X },
    { "c", no_argument, &sel[3], XCC_ALGORITHM_C },
    { "m", no_argument, &sel[3], XCC_ALGORITHM_M },
    { "k", no_argument, &sel[4], XCC_ALGORITHM_KNUTH_CNF },
    { 0, 0, 0, 0 }
  };

  while(1) {

    int option_index = 0;

    c = getopt_long(argc, argv, "epsxcmkhv", long_options, &option_index);

    if(c == -1)
      break;

    switch(c) {
      case 'v':
        cfg->verbose = 1;
        break;
      case 'p':
        cfg->print_options = 1;
        break;
      case 'e':
        cfg->enumerate = 1;
        break;
      case 'h':
        print_help();
        exit(EXIT_SUCCESS);
      case 'x':
        cfg->algorithm_select |= XCC_ALGORITHM_X;
        break;
      case 'c':
        cfg->algorithm_select |= XCC_ALGORITHM_C;
        break;
      case 'm':
        cfg->algorithm_select |= XCC_ALGORITHM_M;
        break;
      case 'k':
        cfg->algorithm_select |= XCC_ALGORITHM_KNUTH_CNF;
        break;
      default:
        break;
    }
  }

  if(optind < argc) {
    cfg->input_files = &argv[optind];
    cfg->input_files_count = argc - optind;
  }

  for(size_t i = 0; i < sizeof(sel) / sizeof(sel[0]); ++i)
    cfg->algorithm_select |= sel[i];
}

static int
process_file(xcc_config* cfg) {
  xcc_algorithm a;
  if(!xcc_algorithm_from_select(cfg->algorithm_select, &a)) {
    err("Could not extract algorithm from algorithm select! Try different "
        "algorithm selection.");
    return EXIT_FAILURE;
  }

  xcc_problem* p =
    xcc_parse_problem_file(&a, cfg->input_files[cfg->current_input_file]);
  if(!p)
    return EXIT_FAILURE;

  p->cfg = cfg;

  if(cfg->verbose)
    xcc_print_problem_matrix(p);

  int return_code = EXIT_SUCCESS;

  if(!a.compute_next_result) {
    err("Algorithm does not support solving!");
    return EXIT_FAILURE;
  }

  int solution = 0;

  do {
    bool has_solution = a.compute_next_result(&a, p);
    if(!has_solution) {
      return_code = 20;
      break;
    } else {
      ++solution;
      return_code = 10;

      if(cfg->print_options) {
        for(xcc_link o = 0; o < p->l; ++o) {
          xcc_link o_ = p->x[o];

          // Go back to beginning of option
          while(TOP(o_ - 1) > 0)
            --o_;

          while(TOP(o_) > 0) {
            printf("%s", NAME(TOP(o_)));
            if(o_ < p->color_size && COLOR(o_) != 0)
              printf(":%s", p->color_name[o_]);
            ++o_;

            if(TOP(o_) > 0)
              printf(" ");
          }
          printf(";\n");
        }
      } else {
        xcc_link solution[p->l];
        xcc_extract_solution_option_indices(p, solution);
        for(size_t i = 0; i < p->l; ++i) {
          printf("%d ", solution[i]);
        }
        printf("\n");
      }
    }
    if(cfg->enumerate)
      printf("\n");

    if(cfg->verbose){
      xcc_print_problem_matrix(p);
      printf("\n");
    }
  } while(cfg->enumerate);

  if(cfg->enumerate) {
    printf("Found %d solutions!\n", solution);
  }

  xcc_problem_free(p, &a);
  return return_code;
}

int
main(int argc, char* argv[]) {
  xcc_config cfg;
  memset(&cfg, 0, sizeof(cfg));
  parse_cli(&cfg, argc, argv);

  int status = EXIT_FAILURE;

  if(cfg.input_files) {
    for(cfg.current_input_file = 0;
        cfg.current_input_file < cfg.input_files_count;
        ++cfg.current_input_file) {
      if(cfg.input_files_count > 1) {
        printf(">>> %s <<<\n", cfg.input_files[cfg.current_input_file]);
      }
      status = process_file(&cfg);
    }
  }

  return status;
}
