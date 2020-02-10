#include "resolver.h"
#include "../assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static _Noreturn void va_fail(
	struct RlcToken const * token,
	struct RlcSrcFile const * file,
	char const * msg,
	va_list ap)
{
	RLC_DASSERT(token != NULL);
	RLC_DASSERT(file != NULL);
	RLC_DASSERT(msg != NULL);

	struct RlcSrcPosition pos;
	rlc_src_file_position(
		file,
		&pos,
		token->content.start);

	fprintf(stderr, "%s:%u:%u: error: ",
		file->fName,
		pos.line,
		pos.column);
	vfprintf(stderr, msg, ap);
	fprintf(stderr, ".\n");
	fflush(stderr);

	exit(EXIT_FAILURE);
}

_Noreturn void rlc_resolver_fail(
	struct RlcToken const * token,
	struct RlcSrcFile const * file,
	char const * msg,
	...)
{
	va_list ap;
	va_start(ap, msg);
	va_fail(token, file, msg, ap);
}

_Noreturn void rlc_resolver_fail_ctx(
	struct RlcResolverFailContext const * ctx,
	char const * msg,
	...)
{
	va_list ap;
	va_start(ap, msg);
	va_fail(ctx->token, ctx->file, msg, ap);
}