/** @file enum.h
	Contains the enum struct as used by the parser. */
#ifndef __rlc_parser_enum_h_defined
#define __rlc_parser_enum_h_defined

#include "scopeentry.h"
#include "parser.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Holds a single constant of an enum, with an arbitrary number of aliases.
@see `RlcParsedEnum`. */
struct RlcParsedEnumConstant
{
	/** The aliases. */
	struct RlcSrcString * fNameTokens;
	/** The alias count. */
	size_t fNameCount;
};

/** Creates an enum constant.
@param[out] this:
	The enum constant to create. */
void rlc_parsed_enum_constant_create(
	struct RlcParsedEnumConstant * this);

/** Adds an alias to an enum constant.
@param[in,out] this:
	The enum constant to add a name to.
	@dassert @nonnull
@param[in] name:
	The alias. */
void rlc_parsed_enum_constant_add_name(
	struct RlcParsedEnumConstant * this,
	struct RlcSrcString const * name);

/** Destroys an enum constant.
@param[in,out] this:
	The constant to destroy. */
void rlc_parsed_enum_constant_destroy(
	struct RlcParsedEnumConstant * this);

/** Parses an enum constant.
@param[in,out] parser:
	The parser data.
	@dassert @nonnull
@param[out] out:
	The enum constant. */
int rlc_parsed_enum_constant_parse(
	struct RlcParsedEnumConstant * out,
	struct RlcParser * parser);

/** Enum type.
@extends RlcParsedScopeEntry */
struct RlcParsedEnum
{
	RLC_DERIVE(struct,RlcParsedScopeEntry);

	/** The constants. */
	struct RlcParsedEnumConstant * fConstants;
	/** The constant count. */
	size_t fConstantCount;
};

/** Creates an enum.
@param[out] this:
	The enum to create.
	@dassert @nonnull
@param[in] name:
	The enum's name.
	@dassert @nonnull */
void rlc_parsed_enum_create(
	struct RlcParsedEnum * this,
	struct RlcSrcString const * name);

/** Destroys an enum.
@memberof RlcParsedEnum
@param[in,out] this:
	The enum to destroy. */
void rlc_parsed_enum_destroy(
	struct RlcParsedEnum * this);

/** Adds a constant to an enum.
@param[in,out] this:
	The enum to add a constant to.
@param[in] constant:
	The constant to add.
	@pass_ownership
	@dassert @nonnull */
void rlc_parsed_enum_add_constant(
	struct RlcParsedEnum * this,
	struct RlcParsedEnumConstant const * constant);

/** Parses an enum.
@param[in,out] parser:
	The parser.
	@dassert @nonnull
@param[out] out:
	The enum to parse.
	@dassert @nonnull */
_Nodiscard int rlc_parsed_enum_parse(
	struct RlcParsedEnum * out,
	struct RlcParser * parser);

#ifdef __cplusplus
}
#endif


#endif