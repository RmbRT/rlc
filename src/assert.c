#include "assert.h"

#include <stdio.h>
#include <stdlib.h>

void rlc_assertion_failure(
	char const * expression,
	char const * file,
	int line,
	char const * function)
{
	printf("assertion failure: \"%s\" in %s:%i (%s).\n", expression, file, line, function);
	abort();
}