#include "class.h"
#include "constructor.h"

#include "../assert.h"
#include "../malloc.h"

void rlc_parsed_class_create(
	struct RlcParsedClass * this,
	struct RlcSrcString const * name,
	struct RlcParsedTemplateDecl const * templates)
{
	RLC_DASSERT(this != NULL);

	rlc_parsed_scope_entry_create(
		RLC_BASE_CAST(this, RlcParsedScopeEntry),
		kRlcParsedClass,
		name);

	if(templates)
		this->fTemplateDecl = *templates;
	else
		rlc_parsed_template_decl_create(&this->fTemplateDecl);

	this->fInheritances = NULL;
	this->fInheritanceCount = 0;

	rlc_parsed_member_list_create(&this->fConstructors);
	rlc_parsed_member_list_create(&this->fMembers);

	this->fHasDestructor = 0;
}

void rlc_parsed_class_destroy(
	struct RlcParsedClass * this)
{
	RLC_DASSERT(this != NULL);

	rlc_parsed_scope_entry_destroy_base(RLC_BASE_CAST(this, RlcParsedScopeEntry));

	rlc_parsed_template_decl_destroy(&this->fTemplateDecl);

	rlc_parsed_member_list_destroy(&this->fMembers);
	rlc_parsed_member_list_destroy(&this->fConstructors);

	if(this->fHasDestructor)
	{
		rlc_parsed_destructor_destroy(&this->fDestructor);
		this->fHasDestructor = 0;
	}

	if(this->fInheritanceCount)
	{
		for(RlcSrcSize i = 0; i < this->fInheritanceCount; i++)
			rlc_parsed_symbol_destroy(&this->fInheritances[i].fBase);
		rlc_free((void**)&this->fInheritances);
		this->fInheritanceCount = 0;
	}
}

int rlc_parsed_class_parse(
	struct RlcParsedClass * out,
	struct RlcParser * parser,
	struct RlcParsedTemplateDecl const * templates)
{
	RLC_DASSERT(out != NULL);
	RLC_DASSERT(parser != NULL);
	RLC_DASSERT(templates != NULL);

	struct RlcToken name;
	if((!rlc_parser_is_ahead(parser, kRlcTokBraceOpen)
		&& !rlc_parser_is_ahead(parser, kRlcTokMinusGreater)
		&& !rlc_parser_is_ahead(parser, kRlcTokVirtual))
	|| !rlc_parser_consume(parser, &name, kRlcTokIdentifier))
	{
		return 0;
	}

	struct RlcParserTracer tracer;
	rlc_parser_trace(parser, "class", &tracer);

	rlc_parsed_class_create(out, &name.content, templates);

	out->fIsVirtual = rlc_parser_consume(
		parser,
		NULL,
		kRlcTokVirtual);

	if(kRlcTokMinusGreater == rlc_parser_expect(
		parser,
		NULL,
		2,
		kRlcTokMinusGreater,
		kRlcTokBraceOpen))
	{
		do
		{
			rlc_realloc(
				(void**)&out->fInheritances,
				++out->fInheritanceCount * sizeof(struct RlcParsedInheritance));
			struct RlcParsedInheritance * in =
				&out->fInheritances[out->fInheritanceCount-1];

			int _ = rlc_visibility_parse(&in->fVisibility, parser, kRlcVisibilityPublic);
			(void) _;

			in->fVirtual = rlc_parser_consume(
				parser,
				NULL,
				kRlcTokVirtual);
			if(!rlc_parsed_symbol_parse(&in->fBase, parser, 0))
				rlc_parser_fail(parser, "expected symbol");
		} while(kRlcTokComma == rlc_parser_expect(
			parser,
			NULL,
			2,
			kRlcTokComma,
			kRlcTokBraceOpen));
	}

	struct RlcParsedMemberCommon common;
	rlc_parsed_member_common_create(&common, kRlcVisibilityPublic);

	struct RlcParsedMember * member;
	while((member = rlc_parsed_member_parse(
		parser,
		&common,
		RLC_ALL_FLAGS(RlcParsedMemberType)
		& (out->fHasDestructor ? ~RLC_FLAG(kRlcParsedDestructor) : ~0))))
	{
		if(RLC_DERIVING_TYPE(member) == kRlcParsedDestructor)
		{
			out->fHasDestructor = 1;
			out->fDestructor = *RLC_DERIVE_CAST(
				member,
				RlcParsedMember,
				struct RlcParsedDestructor);
			rlc_free((void**)&member);
		} else if(RLC_DERIVING_TYPE(member) == kRlcParsedConstructor)
		{
			rlc_parsed_member_list_add(
				&out->fConstructors,
				member);
		} else
		{
			// add the member to the members list.
			rlc_parsed_member_list_add(
				&out->fMembers,
				member);
		}
	}

	rlc_parser_expect(
		parser,
		NULL,
		1,
		kRlcTokBraceClose);

	rlc_parser_untrace(parser, &tracer);

	return 1;
}

static void rlc_parsed_inheritance_print(
	struct RlcParsedInheritance const * this,
	struct RlcSrcFile const * file,
	FILE * out)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(file != NULL);
	RLC_DASSERT(out != NULL);

	rlc_visibility_print(this->fVisibility, 0, out);

	if(this->fVirtual)
		fprintf(out, " virtual ");

	rlc_parsed_symbol_print_no_template(&this->fBase, file, out);
}

static void rlc_parsed_class_print_decl(
	struct RlcParsedClass const * this,
	struct RlcSrcFile const * file,
	FILE * out)
{
	rlc_parsed_template_decl_print(&this->fTemplateDecl, file, out);
	fputs("class ", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	fputs(";\n", out);
}

static void rlc_parsed_class_print_impl(
	struct RlcParsedClass const * this,
	struct RlcSrcFile const * file,
	struct RlcPrinter * printer)
{
	struct RlcPrinterCtx ctx;
	rlc_printer_add_ctx(
		printer,
		&ctx,
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		&this->fTemplateDecl);

	FILE* out = printer->fTypesImpl;
	rlc_parsed_template_decl_print(&this->fTemplateDecl, file, out);
	fputs("class ", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	if(this->fInheritanceCount)
	{
		fputc(':', out);
		for(RlcSrcIndex i = 0; i < this->fInheritanceCount; i++)
		{
			if(i)
				fputs(", ", out);

			rlc_parsed_inheritance_print(
				&this->fInheritances[i],
				file,
				out);
		}
	}

	fprintf(out, " { ");

	rlc_parsed_member_list_print(&this->fMembers, file, printer);

	fputs("public: struct __rl_identifier {};\n", out);

	if(this->fIsVirtual)
		fputs("virtual void const * __rl_get_derived(__rl_identifier const *) const = 0;\n", out);
	else
		fputs("inline void const * __rl_get_derived(__rl_identifier const *) const { return this; }\n", out);

	for(RlcSrcIndex i = 0; i < this->fInheritanceCount; i++)
	{
		fputs("void const * __rl_get_derived(", out);
		rlc_parsed_symbol_print_no_template(
			&this->fInheritances[i].fBase,
			file,
			out);

		fputs("::__rl_identifier const *) const { return __rl::real_addr(*this); }\n", out);
	}


	if(this->fHasDestructor)
	{
		RLC_ASSERT(this->fDestructor.fIsDefinition);

		rlc_visibility_print(
			RLC_BASE_CAST(&this->fDestructor, RlcParsedMember)->fVisibility,
			1,
			out);

		if(this->fDestructor.fIsInline)
			fputs("inline ", out);

		if(this->fIsVirtual)
			fputs("virtual ", out);
		fputc('~', out);
		rlc_src_string_print(
			&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
			file,
			out);
		fputs("();\n", out);

		FILE * out = printer->fFuncsImpl;
		rlc_printer_print_ctx_tpl(printer, file, out);
		rlc_printer_print_ctx_symbol(printer, file, out);
		fputs("::~", out);
		rlc_src_string_print(
			&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
			file,
			out);
		fputs("()\n", out);
		fputs("#define _return return\n", out);
		rlc_parsed_block_statement_print(
			&this->fDestructor.fBody,
			file,
			out);
		fputs("#undef _return\n", out);
	}
	else
	{
		fputs("public:", out);
		if(this->fIsVirtual)
		{
			fputs("virtual ",out);
			fputs(" ~", out);
				rlc_src_string_print(
					&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
					file,
					out);
				fputs("() = default;\n", out);
		}
	}
	fputs("inline void __rl_destructor() const { this->~", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	fputs("(); }", out);


	////////////////
	// tuple ctor //
	////////////////

	fputs(
		"\n// tuple ctor helper: applies a tuple to any existing ctors.\n"
		"template<class...__RL_Types, std::size_t ...__RL_Indices>\n"
		"inline ", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	fputs("(::__rl::TupleCtorHelper,"
		"::__rl::Tuple<__RL_Types...> && __rl_tuple,"
		"::std::index_sequence<__RL_Indices...>):\n", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	fputs("(::std::forward<__RL_Types>(::std::get<__RL_Indices>(__rl_tuple))...)\n"
		"{\n"
		"}\n", out);


	fputs("// tuple ctor: calls tuple ctor helper to apply tuples to ctors.\n"
		"template<class ...__RL_Types>\n"
		"inline ", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	fputs("(::__rl::Tuple<__RL_Types...> &&__rl_tuple,\n"
		"::std::enable_if_t<::std::is_constructible_v<", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	fputs(", __RL_Types...>,\n"
		"\t::__rl::TupleCtorHelper> = __rl::tupleCtorHelper):\n", out);
	rlc_src_string_print(
		&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
		file,
		out);
	fputs("(::__rl::tupleCtorHelper,"
		"::std::move(__rl_tuple),"
		"::std::make_index_sequence<sizeof...(__RL_Types)>{})\n"
		"{\n"
		"}\n", out);

	// Add manual default ctor.
	if(!this->fConstructors.fEntryCount)
	{
		rlc_src_string_print(
			&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
			file,
			out);
		fputs("() = default;\n", out);
	}

	// Default ctors and assignments for virtual classes.
	if(this->fIsVirtual)
	{
		fputs("typedef ", out);
		rlc_printer_print_ctx_symbol(printer, file, out);
		fputs(" __rl_MY_T;\n", out);


		rlc_src_string_print(
			&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
			file,
			out);
		fputs("(__rl_MY_T&&) = default;\n", out);

		rlc_src_string_print(
			&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
			file,
			out);
		fputs("(__rl_MY_T const&) = default;\n", out);

		fputs("__rl_MY_T& operator=(__rl_MY_T&&) = default;\n", out);
		fputs("__rl_MY_T& operator=(__rl_MY_T const&) = default;\n", out);
	}

	////////////////////
	// tuple ctor end //
	////////////////////



	for(RlcSrcIndex i = 0; i < this->fConstructors.fEntryCount; i++)
	{
		struct RlcParsedConstructor * ctor = RLC_DERIVE_CAST(
			this->fConstructors.fEntries[i],
			RlcParsedMember,
			struct RlcParsedConstructor);

		rlc_visibility_print(
			this->fConstructors.fEntries[i]->fVisibility,
			1,
			out);

		rlc_parsed_template_decl_print(&ctor->fTemplates, file, out);
		rlc_src_string_print(
			&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
			file,
			out);
		fputc('(', out);

		for(RlcSrcIndex j = 0; j < ctor->fArgumentCount; j++)
		{
			if(j)
				fputs(", ", out);
			rlc_parsed_variable_print_argument(
				&ctor->fArguments[j],
				file,
				out,
				1);
		}
		fputc(')', out);
		if(!ctor->fIsDefinition
		&& !ctor->fArgumentCount
		&& !ctor->fInitialiserCount)
		{
			fputs(" = default;\n", out);
			continue;
		}

		fputs(";\n", out);

		FILE * out = printer->fFuncsImpl;
		rlc_printer_print_ctx_tpl(printer, file, out);
		rlc_parsed_template_decl_print(&ctor->fTemplates, file, out);
		rlc_printer_print_ctx_symbol(printer, file, out);
		fputs("::", out);
		rlc_src_string_print(
			&RLC_BASE_CAST(this, RlcParsedScopeEntry)->fName,
			file,
			out);
		fputc('(', out);

		for(RlcSrcIndex j = 0; j < ctor->fArgumentCount; j++)
		{
			if(j)
				fputs(", ", out);
			rlc_parsed_variable_print_argument(
				&ctor->fArguments[j],
				file,
				out,
				1);
		}
		fputs(")\n", out);
		for(RlcSrcIndex j = 0; j < ctor->fInitialiserCount; j++)
		{
			struct RlcParsedInitialiser * init = &ctor->fInitialisers[j];
			fputs(j ? ", " : ": ", out);
			rlc_parsed_symbol_print(&init->fMember, file, out);
			fputc('(', out);
			for(RlcSrcIndex k = 0; k < init->fArgumentCount; k++)
			{
				if(k)
					fputs(", ", out);
				rlc_parsed_expression_print(init->fArguments[k], file, out);
			}
			fputs(")\n", out);
		}

		if(ctor->fIsDefinition)
		{
			fputs("\n#define _return return\n", out);
			rlc_parsed_block_statement_print(&ctor->fBody, file, out);
			fputs("\n#undef _return\n", out);
		} else fputs("{;}\n", out);
	}

	fprintf(out, " };\n");
	rlc_printer_pop_ctx(printer);
}

void rlc_parsed_class_print(
	struct RlcParsedClass const * this,
	struct RlcSrcFile const * file,
	struct RlcPrinter * printer)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(printer != NULL);


	rlc_parsed_class_print_decl(
		this,
		file,
		printer->fTypes);

	rlc_parsed_class_print_impl(
		this,
		file,
		printer);
}

void rlc_parsed_member_class_create(
	struct RlcParsedMemberClass * this,
	struct RlcParsedMemberCommon const * member)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(member != NULL);
	RLC_DASSERT(member->attribute == kRlcMemberAttributeNone);

	rlc_parsed_member_create(
		RLC_BASE_CAST(this, RlcParsedMember),
		kRlcParsedMemberClass,
		member);
}

void rlc_parsed_member_class_destroy(
	struct RlcParsedMemberClass * this)
{
	RLC_DASSERT(this != NULL);

	rlc_parsed_class_destroy(
		RLC_BASE_CAST(this, RlcParsedClass));
	rlc_parsed_member_destroy_base(
		RLC_BASE_CAST(this, RlcParsedMember));
}

int rlc_parsed_member_class_parse(
	struct RlcParsedMemberClass * out,
	struct RlcParser * parser,
	struct RlcParsedMemberCommon const * member)
{
	RLC_DASSERT(out != NULL);
	RLC_DASSERT(parser != NULL);
	RLC_DASSERT(member != NULL);

	if(member->attribute != kRlcMemberAttributeNone
	|| !rlc_parsed_class_parse(
		RLC_BASE_CAST(out, RlcParsedClass),
		parser,
		&member->templates))
		return 0;

	rlc_parsed_member_class_create(
		out,
		member);

	return 1;
}

void rlc_parsed_member_class_print(
	struct RlcParsedMemberClass const * this,
	struct RlcSrcFile const * file,
	struct RlcPrinter * printer)
{
	rlc_visibility_print(
		RLC_BASE_CAST(this, RlcParsedMember)->fVisibility,
		1,
		printer->fTypesImpl);
	rlc_parsed_class_print_impl(
		RLC_BASE_CAST(this, RlcParsedClass),
		file,
		printer);
}