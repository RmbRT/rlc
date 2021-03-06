#include "tokens.h"

#include "../assert.h"
#include "../malloc.h"

char const * rlc_token_type_name(
	enum RlcTokenType type)
{
	static char const * const names[] = {
		"identifier",
		"number",
		"float number",
		"character",
		"string",

		"'ABSTRACT'",
		"'ASSERT'",
		"'BREAK'",
		"'CASE'",
		"'CATCH'",
		"'CONCEPT'",
		"'CONTINUE'",
		"'DEFAULT'",
		"'DESTRUCTOR'",
		"'DO'",
		"'ELSE'",
		"'ENUM'",
		"'EXTERN'",
		"'FINAL'",
		"'FINALLY'",
		"'FOR'",
		"'IF'",
		"'INCLUDE'",
		"'INLINE'",
		"'NULL'",
		"'NUMBER'",
		"'OPERATOR'",
		"'OVERRIDE'",
		"'PRIVATE'",
		"'PROTECTED'",
		"'PUBLIC'",
		"'RETURN'",
		"'SIZEOF'",
		"'STATIC'",
		"'SWITCH'",
		"'TEST'",
		"'THIS'",
		"'THROW'",
		"'TRY'",
		"'TYPE'",
		"'UNION'",
		"'VIRTUAL'",
		"'VOID'",
		"'WHILE'",

		"+=",
		"++",
		"+",

		"-=",
		"-:",
		"--",
		"->*",
		"->",
		"-",

		"*=",
		"*",

		"\\",

		"/=",
		"/",

		"%=",
		"%",

		"!=",
		"!:",
		"!",

		"^=",
		"^",

		"~:",
		"~",

		"&&&",
		"&&=",
		"&&",
		"&=",
		"&",

		"||=",
		"||",
		"|=",
		"|",

		"?",

		"::=",
		":=",
		"::",
		":",
		"@@",
		"@",
		"...",
		"..!",
		"..?",
		".*",
		".",
		",",
		";",
		"==",

		"[",
		"]",
		"{",
		"}",
		"(",
		")",

		"<<<=",
		"<<<",
		"<<=",
		"<<",
		"<=",
		"<-",
		"<",

		">>>=",
		">>>",
		">>=",
		">>",
		">=",
		">",

		"$",
		"##",
		"#"
	};

	static_assert(RLC_COVERS_ENUM(names, RlcTokenType), "mal-sized table.");

	RLC_DASSERT(RLC_IN_ENUM(type, RlcTokenType));

	return names[type];
}