/*
 * oracle_query_fuzzer: feeds arbitrary bytes to IvorySQL's Oracle-compatible SQL grammar
 * (liboracle_parser: ora_scan.l / ora_gram.y), the parser the backend uses when
 * ivorysql.compatible_mode = oracle.  The module's _PG_init installs oracle_raw_parser as
 * ora_raw_parser, exactly as loading the shared library does; the harness then parses the input
 * as a semicolon-separated list of commands (RAW_PARSE_DEFAULT) and runs the node support
 * functions (nodeToString, copyObject, equal) over the raw parse tree.  Parse errors are handled
 * with PG_TRY, as the backend itself does.
 *
 * Nothing here needs a database: semantic analysis reads the system catalogs, which do not exist
 * in this process, so it is out of scope.
 */
#include "postgres.h"

#include "miscadmin.h"
#include "tcop/tcopprot.h"
#include "nodes/nodes.h"
#include "nodes/pg_list.h"
#include "parser/parser.h"
#include "utils/guc.h"
#include "utils/memutils.h"

extern void _PG_init(void);

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
	MemoryContextInit();
	InitializeGUCOptions();
	whereToSendOutput = DestNone;
	Log_destination = 0;
	_PG_init();
	return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	MemoryContext ctx;
	MemoryContext oldctx;
	volatile bool parsed = false;
	char	   *query;

	set_stack_base();

	ctx = AllocSetContextCreate(TopMemoryContext, "fuzz input", ALLOCSET_DEFAULT_SIZES);
	oldctx = MemoryContextSwitchTo(ctx);

	query = palloc(size + 1);
	memcpy(query, data, size);
	query[size] = '\0';

	PG_TRY();
	{
		List	   *tree = (*ora_raw_parser) (query, RAW_PARSE_DEFAULT);
		List	   *copy = copyObject(tree);

		(void) nodeToString(tree);
		(void) equal(tree, copy);
		parsed = true;
	}
	PG_CATCH();
	{
		FlushErrorState();
	}
	PG_END_TRY();

	MemoryContextSwitchTo(oldctx);
	MemoryContextDelete(ctx);

	return parsed ? 0 : -1;
}
