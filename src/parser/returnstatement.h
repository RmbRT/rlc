#ifndef __rlc_parser_returnstatement_h_defined
#define __rlc_parser_returnstatement_h_defined

#include "expression.h"
#include "statement.h"
#include "parser.h"
#include "../macros.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RlcParsedReturnStatement
{
	RLC_DERIVE(struct,RlcParsedStatement);
	struct RlcParsedExpression * fExpression;
};

void rlc_parsed_return_statement_create(
	struct RlcParsedReturnStatement * this);

void rlc_parsed_return_statement_destroy(
	struct RlcParsedReturnStatement * this);

int rlc_parsed_return_statement_parse(
	struct RlcParsedReturnStatement * out,
	struct RlcParser * parser);

#ifdef __cplusplus
}
#endif

#endif