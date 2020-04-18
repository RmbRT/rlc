#ifndef __rlc_scoper_scope_h_defined
#define __rlc_scoper_scope_h_defined
#pragma once

#include "../parser/file.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RlcScopedMember;
struct RlcScopedScopeEntry;
struct RlcScopedSymbolChild;
struct RlcScopedScopeItem;
struct RlcScopedScopeItemGroup;
struct RlcScopedStatement;

struct RlcParsedMember;
struct RlcParsedScopeEntry;

/** A scoped scope.
	Scopes have a parent scope, into which they are embedded. Scopes can also have siblings (i.e., multiple namespaces with the same name, or the global namespace in multiple files), which are also included in the scope. */
struct RlcScopedScope
{
	/** Whether the owner is a scope item, as opposed to a statement. */
	int ownerIsItem;
	union {
		/** The scope's associated scope item, or NULL if global scope. */
		struct RlcScopedScopeItem * ownerItem;
		/** The scope's associated statement, or NULL if global scope. */
		struct RlcScopedStatement * ownerStatement;
	};

	/** The file this scope belongs to. */
	struct RlcSrcFile const * file;

	/** The scope's siblings. */
	struct RlcScopedScope ** siblings;
	/** The scope's sibling count. */
	RlcSrcSize siblingCount;

	/** The scope's item goups. */
	struct RlcScopedScopeItemGroup ** groups;
	/** The scope's item group count. */
	RlcSrcSize groupCount;
};

struct RlcScopedScope * rlc_scoped_scope_new_for_item(
	struct RlcScopedScopeItem * owner);

struct RlcScopedScope * rlc_scoped_scope_new_for_statement(
	struct RlcScopedStatement * owner);

struct RlcScopedScope * rlc_scoped_scope_new_root(
	struct RlcSrcFile const * file);

void rlc_scoped_scope_delete(
	struct RlcScopedScope * this);

/** Returns a scope's parent scope, or NULL if it is the global scope.
@memberof RlcScopedScope */
struct RlcScopedScope * rlc_scoped_scope_parent(
	struct RlcScopedScope * this);

/** Returns the root scope containing the current scope.
@memberof RlcScopedScope */
struct RlcScopedScope * rlc_scoped_scope_root(
	struct RlcScopedScope * this);

/** A filter loop callback.
@return
	Whether to continue looking. */
typedef int (*rlc_scoped_scope_filter_fn_t)(
	struct RlcScopedScopeItem *,
	void *);
/** Calls a callback on all scope entries with the requested name.
	Also looks in all sibling and parent scopes if requested.
@memberof RlcScopedScope
@param[in,out] this:
	The scope to search.
	@dassert @nonnull
@param[in] name:
	The name to look for.
@param[in] callback:
	The callback to execute on any found entries.
@param[in,out] userdata:
	Additional data to pass to the callback.
@param[in] check_parents:
	Whether to check parent scopes.
@param[in] check_siblings:
	Whether to check sibling scopes.
@return
	Whether any scope entries were found. */
int rlc_scoped_scope_filter(
	struct RlcScopedScope * this,
	struct RlcScopedSymbolChild const * name,
	rlc_scoped_scope_filter_fn_t callback,
	void * userdata,
	int check_parents,
	int check_siblings);

/** Adds an existing scoped scope entry to a scope.
	Does not reference the entry.
@param[in,out] this:
	The scope to add an entry to.
	@dassert @nonnull
@param[in] item:
	The item to add to the scope
	@dassert @nonnull. */
void rlc_scoped_scope_add_item(
	struct RlcScopedScope * this,
	struct RlcScopedScopeItem * entry);

/** Retrieves or creates a scope item group with the requested name.
@memberof RlcScopedScope
@param[in,out] this:
	The scope whose group to access.
	@dassert @nonnull
@param[in] name:
	The group's name.
	@dassert @nonnull
@param[in] file:
	The file containing the name.
	@dassert @nonnull */
struct RlcScopedScopeItemGroup * rlc_scoped_scope_group(
	struct RlcScopedScope * this,
	struct RlcToken const * name,
	struct RlcSrcFile const * file);

/** Creates a scoped scope entry from a parsed scope entry and adds it to a scope.
@memberof RlcScopedScope
@param[in,out] this:
	The scope to add an entry to.
	@dassert @nonnull
@param[in] file:
	The scope entry's source file.
	@dassert @nonnull
@param[in] entry:
	The scope entry to add.
	@dassert @nonnull */
struct RlcScopedScopeEntry * rlc_scoped_scope_add_entry(
	struct RlcScopedScope * this,
	struct RlcSrcFile const * file,
	struct RlcParsedScopeEntry const * entry);

/** Adds a member to a scope.
@memberof RlcScopedScope
@param[in,out] this:
	The scope to add a member to.
	@dassert @nonnull
@param[in] member:
	The member to add.
	@dassert @nonnull
@param[in] parent:
	The parent scope item.
	@dassert @nonnull */
struct RlcScopedMember * rlc_scoped_scope_add_member(
	struct RlcScopedScope * this,
	struct RlcSrcFile const * file,
	struct RlcParsedMember const * parsed);

void rlc_scoped_scope_populate(
	struct RlcScopedScope * this,
	struct RlcParsedFile const * file);

#ifdef __cplusplus
}
#endif


#endif