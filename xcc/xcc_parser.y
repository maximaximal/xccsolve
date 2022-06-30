%define api.pure full
%define api.prefix {xcc}
%define parse.error detailed
%define parse.lac full

%lex-param {yyscan_t scanner}
%parse-param {yyscan_t scanner}
%parse-param {struct xcc_algorithm *algorithm}
%parse-param {struct xcc_problem **problem}

%code requires {
#include <xcc.h>
#include <algorithm.h>
#define YYSTYPE XCCSTYPE
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

int yylex();
int yyerror();
}

%{

%}

%code top {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

%union
{
    int32_t num;
    char *str;
    xcc_item item;
    xcc_problem *problem;
}

%code {
#include <xcc_parser.h>
#include <xcc_lexer.h>

#define xstr(s) str(s)
#define str(s) #s

#define ERR(MSG) xccerror(algorithm, problem, scanner, MSG)
#define CALL(FUNC, args...) \
    if(!FUNC) \
    { ERR("Function " str(FUNC) " undefined! Cannot call."); return 0;} \
			else if(!FUNC(args)) \
			{ ERR("Function " str(FUNC) " returned unsuccessfully!"); return 0;}
}

%token <num> NUM
%token <str> ID
%token <item> ITEM

%token UNKNOWN
%token LPRIMLIST
%token RPRIMLIST
%token LSECLIST
%token RSECLIST
%token COLORSEP
%token ENDOPTION

%type	start
%type	problem
%type primary_items
%type primary_item
%type secondary_items
%type secondary_item
%type options
%type option

%%

start :
  problem
;

problem:	{ *problem = xcc_problem_allocate(); }
		LPRIMLIST primary_items RPRIMLIST
		LSECLIST secondary_items RSECLIST
		options
	;

primary_items:	primary_item
	|	primary_item primary_items
	;

primary_item: 	ID { CALL(algorithm->define_primary_item, algorithm, *problem, $1); }
        |	ID ':' '[' NUM ';' NUM ']'
        	{ CALL(algorithm->define_primary_item_with_range, algorithm, *problem, $1, $4, $6); }
	;

secondary_items:
		secondary_item
	|	secondary_item secondary_items
	;

secondary_item:	ID { algorithm->define_secondary_item(algorithm, *problem, $1); }
	;

options:	option ENDOPTION { CALL(algorithm->end_option, algorithm, *problem); }
	|	options option
	;

option:		ID { CALL(algorithm->add_item,
		    algorithm,
		    *problem,
		    xcc_item_from_ident(*problem, $1));
		}
	|	ID ':' NUM { CALL(algorithm->add_item_with_color,
				  algorithm,
				  *problem,
				  xcc_item_from_ident(*problem, $1),
				  $3);
		}
	;

%%

int xccerror(void *algorithm,
	     void *problem,
	     const void* scanner,
	     char const *msg)
{
    (void)algorithm;
    (void)problem;
    (void)scanner;
    return fprintf(stderr, "[XCC] Parser Error: %s\n", msg);
}
