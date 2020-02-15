#include "scope.h"
#include "scopeitem.h"
#include "scopeentry.h"
#include "member.h"
#include "symbol.h"
#include "../assert.h"
#include "../malloc.h"

struct RlcScopedScope * rlc_scoped_scope_new(
	struct RlcScopedScopeItem * owner)
{
	struct RlcScopedScope * ret = NULL;
	rlc_malloc(
		(void**)&ret,
		sizeof(struct RlcScopedScope));

	ret->owner = owner;
	ret->siblings = NULL;
	ret->siblingCount = 0;
	ret->entries = NULL;
	ret->entryCount = 0;

	return ret;
}

void rlc_scoped_scope_delete(
	struct RlcScopedScope * this)
{
	RLC_DASSERT(this != NULL);

	if(this->siblings)
	{
		rlc_free((void**)&this->siblings);
		this->siblingCount = 0;
	}

	if(this->entries)
	{
		for(RlcSrcIndex i = 0; i < this->entryCount; i++)
			rlc_scoped_scope_item_delete(this->entries[i]);
		rlc_free((void**)&this->entries);
		this->entryCount = 0;
	}

	rlc_free((void**)&this);
}

static int rlc_scoped_scope_filter_impl(
	struct RlcScopedScope * this,
	struct RlcScopedSymbolChild const * name,
	rlc_scoped_scope_filter_fn_t callback,
	void * userdata,
	int parents,
	int siblings,
	int * abort)
{
	int found = 0;

	for(RlcSrcIndex i = 0; i < this->entryCount; i++)
	{
		if(0 == rlc_scoped_identifier_compare(
			&this->entries[i]->name,
			&name->name))
		{
			found = 1;
			if(!callback(this->entries[i], userdata))
			{
				*abort = 1;
				return 1;
			}
		}
	}

	if(siblings)
	{
		for(RlcSrcIndex i = 0; i < this->siblingCount; i++)
		{
			found |= rlc_scoped_scope_filter_impl(
				this->siblings[i],
				name,
				callback,
				userdata,
				0, 0, // Don't look into siblings' relatives.
				abort);

			if(*abort)
				return 1;
		}
	}
	if(parents)
	{
		if(this->owner)
		{
			found |= rlc_scoped_scope_filter_impl(
				this->owner->parent,
				name,
				callback,
				userdata,
				1, siblings, // Look into parent's relatives.
				abort);
		}
	}

	return found;
}

int rlc_scoped_scope_filter(
	struct RlcScopedScope * this,
	struct RlcScopedSymbolChild const * name,
	rlc_scoped_scope_filter_fn_t callback,
	void * userdata,
	int check_parents,
	int check_siblings)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(name != NULL);
	RLC_DASSERT(callback != NULL);

	int abort = 0;
	return rlc_scoped_scope_filter_impl(
		this,
		name,
		callback,
		userdata,
		check_parents,
		check_siblings,
		&abort);
}


void rlc_scoped_scope_add_item(
	struct RlcScopedScope * this,
	struct RlcScopedScopeItem * item)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(item != NULL);

	rlc_realloc(
		(void **)&this->entries,
		sizeof(struct RlcScopedScopeItem *) * ++this->entryCount);

	this->entries[this->entryCount-1] = item;
}

struct RlcScopedScopeEntry * rlc_scoped_scope_add_entry(
	struct RlcScopedScope * this,
	struct RlcSrcFile const * file,
	struct RlcParsedScopeEntry * entry,
	struct RlcScopedScope * parent)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(file != NULL);
	RLC_DASSERT(entry != NULL);

	struct RlcScopedScopeEntry * res = rlc_scoped_scope_entry_new(
		file,
		entry,
		parent);

	rlc_scoped_scope_add_item(
		this,
		RLC_BASE_CAST(res, RlcScopedScopeItem));
	return res;
}

struct RlcScopedMember * rlc_scoped_scope_add_member(
	struct RlcScopedScope * this,
	struct RlcSrcFile const * file,
	struct RlcParsedMember const * parsed,
	struct RlcScopedScopeItem * parent)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(file != NULL);
	RLC_DASSERT(parsed != NULL);

	struct RlcScopedMember * res = rlc_scoped_member_new(file, parsed, parent);
	rlc_scoped_scope_add_item(
		this,
		RLC_BASE_CAST(res, RlcScopedScopeItem));
	return res;

}

void rlc_scoped_scope_populate(
	struct RlcScopedScope * this,
	struct RlcParsedFile const * file)
{
	RLC_DASSERT(this != NULL);
	RLC_DASSERT(file != NULL);

	for(RlcSrcIndex i = 0; i < file->fScopeEntries.fEntryCount; i++)
	{
		rlc_scoped_scope_add_entry(
			this,
			&file->fSource,
			file->fScopeEntries.fEntries[i],
			NULL);
	}
}