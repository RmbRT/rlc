#include "sizeofexpression.h"
#include "../assert.h"
#include "../malloc.h"

void rlc_parsed_sizeof_expression_create(
	struct RlcParsedSizeofExpression * this,
	RlcSrcIndex start,
	RlcSrcIndex end)
{
	RLC_DASSERT(this != NULL);

	rlc_parsed_expression_create(
		RLC_BASE_CAST(this, RlcParsedExpression),
		kRlcParsedSizeofExpression,
		start,
		end);
}

void rlc_parsed_sizeof_expression_destroy(
	struct RlcParsedSizeofExpression * this)
{
	RLC_DASSERT(this != NULL);

	rlc_parsed_expression_destroy_base(
		RLC_BASE_CAST(this, RlcParsedExpression));

	if(this->fIsType)
		rlc_parsed_type_name_destroy(&this->fType);
	else if(this->fExpression)
	{
		rlc_parsed_expression_destroy_virtual(this->fExpression);
		rlc_free((void**)&this->fExpression);
	}
}

int rlc_parsed_sizeof_expression_parse(
	struct RlcParsedSizeofExpression * out,
	struct RlcParser * parser)
{
	RLC_DASSERT(out != NULL);
	RLC_DASSERT(parser != NULL);

	size_t const start = rlc_parser_index(parser);

	if(!rlc_parser_consume(
		parser,
		NULL,
		kRlcTokSizeof))
		return 0;


	rlc_parser_expect(
		parser,
		NULL,
		1,
		kRlcTokParentheseOpen);

	if((out->fIsType = !rlc_parser_consume(
		parser,
		NULL,
		kRlcTokHash)))
	{
		if(!rlc_parsed_type_name_parse(
			&out->fType,
			parser,
			0))
		{
			rlc_parser_fail(parser, "expected type name");
		}
	} else
	{
		if(!(out->fExpression = rlc_parsed_expression_parse(
			parser,
			RLC_ALL_FLAGS(RlcParsedExpressionType))))
		{
			rlc_parser_fail(parser, "expected expression");
		}
	}

	struct RlcToken end;
	rlc_parser_expect(
		parser,
		&end,
		1,
		kRlcTokParentheseClose);

	rlc_parsed_sizeof_expression_create(out, start, rlc_src_string_end(&end.content));
	return 1;
}

void rlc_parsed_sizeof_expression_print(
	struct RlcParsedSizeofExpression const * this,
	struct RlcSrcFile const * file,
	FILE * out)
{
	fprintf(out, "sizeof(");
	if(this->fIsType)
		rlc_parsed_type_name_print(&this->fType, file, out);
	else
		rlc_parsed_expression_print(this->fExpression, file, out);
	fputc(')', out);
}