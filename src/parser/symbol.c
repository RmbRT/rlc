#include "symbol.h"
#include "typename.h"

#include "../malloc.h"
#include "../assert.h"

void rlc_parsed_symbol_child_create(
	struct RlcParsedSymbolChild * this)
{
	RLC_DASSERT(this != NULL);

	this->fNameToken = 0;
	this->fTemplates = NULL;
	this->fTemplateCount = 0;
}

int rlc_parsed_symbol_child_parse(
	struct RlcParsedSymbolChild * out,
	struct RlcParserData * parser)
{
	RLC_DASSERT(out != NULL);
	RLC_DASSERT(parser != NULL);

	enum RlcParseError error_code;

	rlc_parsed_symbol_child_create(out);

	if(rlc_parser_data_consume(
		parser,
		kRlcTokIdentifier))
	{
		out->fType = kRlcParsedSymbolChildTypeIdentifier;
	} else if(rlc_parser_data_consume(
		parser,
		kRlcTokConstructor))
	{
		out->fType = kRlcParsedSymbolChildTypeConstructor;
	} else if(rlc_parser_data_consume(
		parser,
		kRlcTokDestructor))
	{
		out->fType = kRlcParsedSymbolChildTypeDestructor;
	} else
		goto nonfatal_failure;

	out->fNameToken = rlc_parser_data_consumed_index(parser);

	if(rlc_parser_data_consume(
		parser,
		kRlcTokBraceOpen))
	{
		if(out->fType == kRlcParsedSymbolChildTypeDestructor)
		{
			error_code = kRlcParseErrorDestructorTemplate;
			goto failure;
		}

		struct RlcParsedSymbolChildTemplate template;
		do {
			if((template.fIsExpression = rlc_parser_data_consume(
				parser,
				kRlcTokHash)))
			{
				if(template.fExpression = rlc_parsed_expression_parse(
					parser,
					RLC_ALL_FLAGS(RlcParsedExpressionType)))
				{
					rlc_parsed_symbol_child_add_template(
						out,
						&template);
				} else
				{
					error_code = kRlcParseErrorExpectedExpression;
					goto failure;
				}
			} else
			{
				template.fTypeName = NULL;
				rlc_malloc(
					(void**)&template.fTypeName,
					sizeof(struct RlcParsedTypeName));

				if(rlc_parsed_type_name_parse(
					template.fTypeName,
					parser))
				{
					rlc_parsed_symbol_child_add_template(
						out,
						&template);
				} else
				{
					rlc_free((void**)&template.fTypeName);
					error_code = kRlcParseErrorExpectedTypeNameExpression;
					goto failure;
				}
			}
		} while(rlc_parser_data_consume(
			parser,
			kRlcTokComma));

		if(!rlc_parser_data_consume(
			parser,
			kRlcTokBraceClose))
		{
			error_code = kRlcParseErrorExpectedBraceClose;
			goto failure;
		}
	}

	return 1;

failure:
	rlc_parser_data_add_error(
		parser,
		error_code);
nonfatal_failure:
	rlc_parsed_symbol_child_destroy(out);

	return 0;
}

void rlc_parsed_symbol_child_add_template(
	struct RlcParsedSymbolChild * this,
	struct RlcParsedSymbolChildTemplate * template_argument)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(template_argument != NULL);

	rlc_realloc(
		(void**)&this->fTemplates,
		sizeof(struct RlcParsedSymbolChildTemplate) * ++ this->fTemplateCount);

	this->fTemplates[this->fTemplateCount-1] = *template_argument;
}

void rlc_parsed_symbol_child_destroy(
	struct RlcParsedSymbolChild * this)
{
	RLC_DASSERT(this != NULL);

	this->fNameToken = 0;
	if(this->fTemplates)
	{
		for(size_t i = 0; i < this->fTemplateCount; i++)
		{
			if(this->fTemplates[i].fIsExpression)
			{
				if(this->fTemplates[i].fExpression)
				{
					rlc_parsed_expression_destroy_virtual(this->fTemplates[i].fExpression);
					rlc_free((void**)&this->fTemplates[i].fExpression);
				}
			}
			else
			{
				if(this->fTemplates[i].fTypeName)
				{
					rlc_parsed_type_name_destroy(this->fTemplates[i].fTypeName);
					rlc_free((void**)&this->fTemplates[i].fTypeName);
				}
			}
		}
		rlc_free((void**)&this->fTemplates);
	}
	this->fTemplateCount = 0;
}

void rlc_parsed_symbol_destroy(
	struct RlcParsedSymbol * this)
{
	RLC_DASSERT(this != NULL);

	if(this->fChildren)
	{
		for(size_t i = 0; i < this->fChildCount; i++)
			rlc_parsed_symbol_child_destroy(&this->fChildren[i]);
		rlc_free((void**)&this->fChildren);
	}

	this->fChildCount = 0;
	this->fIsRoot = 0;
}


void rlc_parsed_symbol_add_child(
	struct RlcParsedSymbol * this,
	struct RlcParsedSymbolChild * child)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(child != NULL);

	rlc_realloc(
		(void**)&this->fChildren,
		sizeof (struct RlcParsedSymbolChild) * ++ this->fChildCount);

	this->fChildren[this->fChildCount-1] = *child;
#ifdef RLC_DEBUG
	// invalidate the old data.
	rlc_parsed_symbol_child_create(child);
#endif
}

void rlc_parsed_symbol_create(
	struct RlcParsedSymbol * this)
{
	RLC_DASSERT(this != NULL);

	this->fChildren = NULL;
	this->fChildCount = 0;
	this->fIsRoot = 0;
}

int rlc_parsed_symbol_parse(
	struct RlcParsedSymbol * out,
	struct RlcParserData * parser)
{
	RLC_DASSERT(out != NULL);
	RLC_DASSERT(parser != NULL);


	enum RlcParseError error_code;

	rlc_parsed_symbol_create(out);

	out->fIsRoot = rlc_parser_data_consume(
		parser,
		kRlcTokDoubleColon)
	? 1
	: 0;

	int parsed_any = 0;

	do {
		struct RlcParsedSymbolChild child;
		if(rlc_parsed_symbol_child_parse(
			&child,
			parser))
		{
			parsed_any = 1;
			rlc_parsed_symbol_add_child(
				out,
				&child);
		} else if(parsed_any || out->fIsRoot)
		{
			error_code = kRlcParseErrorExpectedSymbol;
			goto failure;
		}
	} while(rlc_parser_data_consume(
		parser,
		kRlcTokDoubleColon));

	if(!parsed_any)
		goto nonfatal_failure;

success:
	return 1;
failure:
	rlc_parser_data_add_error(
		parser,
		error_code);
nonfatal_failure:
	rlc_parsed_symbol_destroy(
		out);
	return 0;
}