
#include "main.h"

#include <sqlite/sqlite3.h>

//--------------------------------------------------------------------------------------

typedef struct _SQLiteResult
{
	int nRows;
	int nColumns;
	char **pResults;
	char *szErrMsg;
	int nCurrentRow;

} SQLiteResult;

SAMPMAP mapDbResults;
SAMPMAP mapDbHandles;

//--------------------------------------------------------------------------------------


static cell AMX_NATIVE_CALL  n_open(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_close(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_query(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_free_result(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_num_rows(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_next_row(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_num_fields(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_field_name(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_get_field(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_get_field_int(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_get_field_float(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_get_field_assoc(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_get_field_assoc_int(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_get_field_assoc_float(AMX* amx, cell* params)
{
	return 0;
}

static cell AMX_NATIVE_CALL n_debug_openfiles(AMX* amx, cell* params)
{
	return mapDbHandles.m_dwSize;
}

static cell AMX_NATIVE_CALL n_debug_openresults(AMX* amx, cell* params)
{
	return mapDbResults.m_dwSize;
}

static cell AMX_NATIVE_CALL n_get_mem_handle(AMX* amx, cell* params)
{
	DWORD index = params[1] - 1;

	if (index < mapDbHandles.m_dwSize)
	{
		return (cell)mapDbHandles.m_pdwPtrs[index];
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_get_result_mem_handle(AMX* amx, cell* params)
{
	DWORD index = params[1] - 1;

	if (index < mapDbResults.m_dwSize)
	{
		return (cell)mapDbResults.m_pdwPtrs[index];
	}
	return 0;
}

/*
 native DB:db_open(name[]);
 native db_close(DB:db);
 native DBResult:db_query(DB:db, query[]);
 native db_free_result(DBResult:dbresult);
 native db_num_rows(DBResult:dbresult);
 native db_next_row(DBResult:dbresult);
 native db_num_fields(DBResult:dbresult);
 native db_field_name(DBResult:dbresult, field, result[], maxlength);
 native db_get_field(DBResult:dbresult, field, result[], maxlength);
 native db_get_field_int(DBResult:result, field = 0);
 native Float:db_get_field_float(DBResult:result, field = 0);
 native db_get_field_assoc(DBResult:dbresult, const field[], result[], maxlength);
 native db_get_field_assoc_int(DBResult:result, const field[]);
 native Float:db_get_field_assoc_float(DBResult:result, const field[]);
 native db_get_mem_handle(DB:db);
 native db_get_result_mem_handle(DBResult:result);
 native db_debug_openfiles();
 native db_debug_openresults();
*/

#if defined __cplusplus
extern "C"
#endif
AMX_NATIVE_INFO sampDb_Natives[] = {
  { "db_open",				n_open },
  { "db_close",				n_close },
  { "db_query",				n_query },
  { "db_free_result",		n_free_result },
  { "db_num_rows",			n_num_rows },
  { "db_next_row",			n_next_row },
  { "db_num_fields",		n_num_fields },
  { "db_field_name",		n_field_name },
  { "db_get_field",			n_get_field },
  { "db_get_field_int",		n_get_field_int },
  { "db_get_field_float",	n_get_field_float },
  { "db_get_field_assoc",	n_get_field_assoc },
  { "db_get_field_assoc_int",n_get_field_assoc_int },
  { "db_get_field_assoc_float",n_get_field_assoc_float },
  { "db_debug_openfiles",	n_debug_openfiles },
  { "db_debug_openresults",	n_debug_openresults },
  { "db_get_mem_handle",	n_get_mem_handle },
  { "db_get_result_mem_handle",n_get_result_mem_handle },
  { NULL, NULL }        /* terminator */
};

int AMXEXPORT amx_sampDbInit(AMX* amx)
{
	return amx_Register(amx, sampDb_Natives, -1);
}

int AMXEXPORT amx_sampDbCleanup(AMX* amx)
{
	return AMX_ERR_NONE;
}
