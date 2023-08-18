#include <algorithm>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <xcc/algorithm.h>
#include <xcc/algorithm_m.h>
#include <xcc/algorithm_x.h>
#include <xcc/parse.h>
#include <xcc/xcc.h>

TEST_CASE("solve standard XCC example") {
  const char* str = "<a b c d e f g> c e; a d g; b c f; a d f; b g; d e g;";

  xcc_algorithm algorithm;
  xcc_algoritihm_x_set(&algorithm);

  xcc_problem_ptr p(xcc_parse_problem(&algorithm, str));
  REQUIRE(p);

  bool has_result = algorithm.compute_next_result(&algorithm, p.get());

  REQUIRE(has_result);

  std::vector<xcc_link> solution(p->l);
  xcc_extract_solution_option_indices(p.get(), solution.data());
  std::sort(solution.begin(), solution.end());

  REQUIRE(solution[0] == 1);
  REQUIRE(solution[1] == 4);
  REQUIRE(solution[2] == 5);
}

TEST_CASE("solve small MCC example") {
  const char* str = "<a : 2 b : 1;2> a; a b; b;";
  xcc_algorithm algorithm;
  xcc_algoritihm_m_set(&algorithm);

  xcc_problem_ptr p(xcc_parse_problem(&algorithm, str));
  REQUIRE(p);

  bool has_result = algorithm.compute_next_result(&algorithm, p.get());

  REQUIRE(has_result);

  std::vector<xcc_link> solution(p->l);
  xcc_extract_solution_option_indices(p.get(), solution.data());
  std::sort(solution.begin(), solution.end());

  std::vector<xcc_link> solution_unsorted(p->l);
  xcc_extract_solution_option_indices(p.get(), solution_unsorted.data());

  const bool has_duplicates = std::adjacent_find(solution.begin(), solution.end()) != solution.end();

  CAPTURE(solution);
  CAPTURE(solution_unsorted);
  REQUIRE_FALSE(has_duplicates);
}
