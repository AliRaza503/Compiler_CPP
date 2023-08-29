#ifndef __PARSER__H__
#define __PARSER__H__

#include <vector>
#include <string>

#include "inputbuf.h"
#include "lexer.h"

class Parser {
	public:
		LexicalAnalyzer lexer;
		Token token;
		TokenType tempTokenType;
		int parse_program();
	private:
		int parse_varlist();
		int parse_unaryOperator();
		int parse_binaryOperator();
		int parse_primary();
		int parse_expression();
		int parse_assstmt();
		int parse_case();
		int parse_caselist();
		int parse_switchstmt();
		int parse_whilestmt();
		int parse_ifstmt();
		int parse_stmt();
		int parse_stmtlist();
		int parse_body();
		int parse_typename();
		int parse_vardecl();
		int parse_vardecllist();
		int parse_globalVars();

    int parse_exp(void);
};

#endif  //__PARSER__H__

