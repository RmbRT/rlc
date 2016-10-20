/** @file numberexpression.h
	Contains the definition of the numerical expression type. */
#ifndef __rlc_parser_numberexpression_h_defined
#define __rlc_parser_numberexpression_h_defined

#include "expression.h"
#include "parser.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A numerical expression.
	Numerical expressions can either be numbers or single characters.
@implements RlcParsedExpression */
struct RlcParsedNumberExpression
{
	RLC_DERIVE(struct,RlcParsedExpression);

	/** The number token's index. */
	size_t fNumberToken;
};

/** Initialises a number expression.
@memberof RlcParsedNumberExpression

@param[in] this:
	The number expression to initialise.
@param[in] token_index:
	The number token's index. */
void rlc_parsed_number_expression_create(
	struct RlcParsedNumberExpression * this,
	size_t token_index);

/** Destroys a number expression.
@memberof RlcParsedNumberExpression

@param[in] this:
	The number expression to destroy. */
void rlc_parsed_number_expression_destroy(
	struct RlcParsedNumberExpression * this);

/** Parses a number expression.
@memberof RlcParsedNumberExpression
@param[out] out:
	The number expression to parse.
	@dassert @nonnull
@param[in,out] parser:
	The parser data.
	@dassert @nonnull
@return
	Whether the number expression could be parsed. */
int rlc_parsed_number_expression_parse(
	struct RlcParsedNumberExpression * out,
	struct RlcParserData * parser);

#ifdef __cplusplus
}
#endif

#endif