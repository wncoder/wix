#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="sceutil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    Header for SQL Compact Edition helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef void* SCE_DATABASE_HANDLE;
typedef void* SCE_ROW_HANDLE;
typedef void* SCE_QUERY_HANDLE;
typedef void* SCE_QUERY_RESULTS_HANDLE;

#define ReleaseSceRow(prrh) if (prrh) { SceFreeRow(prrh); }
#define ReleaseNullSceRow(prrh) if (prrh) { SceFreeRow(prrh); prrh = NULL; }
#define ReleaseSceQuery(pqh) if (pqh) { SceFreeQuery(pqh); }
#define ReleaseNullSceQuery(pqh) if (pqh) { SceFreeQuery(pqh); pqh = NULL; }
#define ReleaseSceQueryResults(pqh) if (pqh) { SceFreeQueryResults(pqh); }
#define ReleaseNullSceQueryResults(pqh) if (pqh) { SceFreeQueryResults(pqh); pqh = NULL; }

struct SCE_COLUMN_SCHEMA
{
    LPCWSTR wzName;
    DBTYPE dbtColumnType;
    DWORD dwLength;
    BOOL fPrimaryKey; // If this column is the primary key
    BOOL fNullable;
    BOOL fAutoIncrement;

    LPWSTR wzRelationName;
    DWORD dwForeignKeyTable;
    DWORD dwForeignKeyColumn;
};

struct SCE_INDEX_SCHEMA
{
    LPWSTR wzName;

    DWORD *rgColumns;
    DWORD cColumns;
};

struct SCE_TABLE_SCHEMA
{
    LPCWSTR wzName;
    DWORD cColumns;
    SCE_COLUMN_SCHEMA *rgColumns;

    DWORD cIndexes;
    SCE_INDEX_SCHEMA *rgIndexes;

    // Internal to SCEUtil - consumers shouldn't access or modify
    // TODO: enforce / hide in a handle of some sort?
    IRowset *pIRowset;
    IRowsetChange *pIRowsetChange;
};

struct SCE_DATABASE_SCHEMA
{
    DWORD cTables;
    SCE_TABLE_SCHEMA *rgTables;
};

struct SCE_DATABASE
{
    SCE_DATABASE_HANDLE sdbHandle;
    SCE_DATABASE_SCHEMA *pdsSchema;
};

HRESULT DAPI SceCreateDatabase(
    __in_z LPCWSTR sczFile,
    __out SCE_DATABASE **ppDatabase
    );
HRESULT DAPI SceOpenDatabase(
    __in_z LPCWSTR sczFile,
    __out SCE_DATABASE **ppDatabase
    );
HRESULT DAPI SceEnsureDatabase(
    __in_z LPCWSTR sczFile,
    __in SCE_DATABASE_SCHEMA *pdsSchema,
    __out SCE_DATABASE **ppDatabase
    );
HRESULT DAPI SceIsTableEmpty(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out BOOL *pfEmpty
    );
HRESULT DAPI SceGetFirstRow(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out SCE_ROW_HANDLE *pRowHandle
    );
HRESULT DAPI SceGetNextRow(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out SCE_ROW_HANDLE *pRowHandle
    );
HRESULT DAPI SceBeginTransaction(
    __in SCE_DATABASE *pDatabase
    );
HRESULT DAPI SceCommitTransaction(
    __in SCE_DATABASE *pDatabase
    );
HRESULT DAPI SceRollbackTransaction(
    __in SCE_DATABASE *pDatabase
    );
HRESULT DAPI SceDeleteRow(
    __in SCE_ROW_HANDLE *pRowHandle
    );
HRESULT DAPI ScePrepareInsert(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out SCE_ROW_HANDLE *pRowHandle
    );
HRESULT DAPI SceFinishUpdate(
    __in SCE_ROW_HANDLE rowHandle
    );
HRESULT DAPI SceSetColumnBinary(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in_bcount(cbBuffer) const BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    );
HRESULT DAPI SceSetColumnDword(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in const DWORD dwValue
    );
HRESULT DAPI SceSetColumnQword(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in const DWORD64 qwValue
    );
HRESULT DAPI SceSetColumnBool(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in const BOOL fValue
    );
HRESULT DAPI SceSetColumnString(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in_z LPCWSTR pszValue
    );
HRESULT DAPI SceSetColumnSystemTime(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in const SYSTEMTIME *pst
    );
HRESULT DAPI SceSetColumnEmpty(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex
    );
HRESULT DAPI SceGetColumnBinary(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out_opt BYTE **ppbBuffer,
    __inout SIZE_T *pcbBuffer
    );
HRESULT DAPI SceGetColumnDword(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out DWORD *pdwValue
    );
HRESULT DAPI SceGetColumnQword(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out DWORD64 *pqwValue
    );
HRESULT DAPI SceGetColumnBool(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out BOOL *pfValue
    );
HRESULT DAPI SceGetColumnString(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out LPWSTR *ppszValue
    );
HRESULT DAPI SceGetColumnSystemTime(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out SYSTEMTIME *pst
    );
HRESULT DAPI SceBeginQuery(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __in DWORD dwIndex,
    __out SCE_QUERY_HANDLE *psqhHandle
    );
HRESULT DAPI SceSetQueryColumnBinary(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in_bcount(cbBuffer) const BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    );
HRESULT DAPI SceSetQueryColumnDword(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in const DWORD dwValue
    );
HRESULT DAPI SceSetQueryColumnQword(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in const DWORD64 qwValue
    );
HRESULT DAPI SceSetQueryColumnBool(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in const BOOL fValue
    );
HRESULT DAPI SceSetQueryColumnString(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in_z LPCWSTR pszString
    );
HRESULT DAPI SceSetQueryColumnSystemTime(
    __in SCE_ROW_HANDLE rowHandle,
    __in const SYSTEMTIME *pst
    );
HRESULT DAPI SceSetQueryColumnEmpty(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in_z LPCWSTR pszString
    );
HRESULT DAPI SceRunQueryExact(
    __in SCE_QUERY_HANDLE *psqhHandle,
    __out SCE_ROW_HANDLE *pRowHandle
    );
HRESULT DAPI SceRunQueryRange(
    __in SCE_QUERY_HANDLE *psqhHandle,
    __out SCE_QUERY_RESULTS_HANDLE *psqrhHandle
    );
HRESULT DAPI SceGetNextResultRow(
    __in SCE_QUERY_RESULTS_HANDLE sqrhHandle,
    __out SCE_ROW_HANDLE *pRowHandle
    );
void DAPI SceCloseTable(
    __in SCE_TABLE_SCHEMA *pTable
    );
HRESULT DAPI SceCloseDatabase(
    __in SCE_DATABASE *pDatabase
    );
void DAPI SceFreeRow(
    __in SCE_ROW_HANDLE rowReadHandle
    );
void DAPI SceFreeQuery(
    __in SCE_QUERY_HANDLE sqhHandle
    );
void DAPI SceFreeQueryResults(
    __in SCE_QUERY_RESULTS_HANDLE sqrhHandle
    );

#ifdef __cplusplus
}
#endif
