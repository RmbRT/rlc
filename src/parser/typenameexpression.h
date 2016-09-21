/** @file typenameexpression.h
	Contains the type name expression type. */
#ifndef __rlc_parser_expression_typenameexpression_h_defined
#define __rlc_parser_expression_typenameexpression_h_defined

#include "expression.h"
#include "typename.h"
#include "../macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A type name expression.
@extends RlcParsedExpression
@extends RlcParsedTypeName */
struct RlcParsedTypeNameExpression
{
	RLC_DERIVE(struct, RlcParsedExpression);
	RLC_DERIVE(struct, RlcParsedTypeName);
};

/** Creates a type name expression.
@param[out] this:
	The type name expression to create.
	@dassert @nonnull */
void rlc_parsed_type_name_expression_create(
	struct RlcParsedTypeNameExpression * this);

/** Destroys a parsed type name expression.
@param[in,out] this:
	The type name expression to destroy.
	@dassert @nonnull */
void rlc_parsed_type_name_expression_destroy(
	struct RlcParsedTypeNameExpression * this);

#ifdef __cplusplus
}
#endif


#endif