//-------------------------------------------------------------------------------------------------
// <copyright file="sceutil.cpp" company="Microsoft">
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
//    SQL Compact Edition helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// This conflicts with some of SQLUtil's includes, so best to keep it out of precomp
#include <sqlce_err.h>

// Limit is documented as 4 GB, but for some reason the API's don't let us specify anything above 4091 MB.
#define MAX_SQLCE_DATABASE_SIZE 4091

// structs
struct SCE_DATABASE_INTERNAL
{
    volatile LONG dwTransactionRefcount;
    IDBInitialize *pIDBInitialize;
    IDBCreateSession *pIDBCreateSession;
    ITransactionLocal *pITransactionLocal;
    IDBProperties *pIDBProperties;
    IOpenRowset *pIOpenRowset;
    ISessionProperties *pISessionProperties;
};

struct SCE_ROW
{
    SCE_TABLE_SCHEMA *pTableSchema;
    IRowset *pIRowset;
    HROW hRow;
    BOOL fInserting;

    DWORD dwBindingIndex;
    DBBINDING *rgBinding;
    SIZE_T cbOffset;
    BYTE *pbData;
};

struct SCE_QUERY
{
    SCE_TABLE_SCHEMA *pTableSchema;
    SCE_INDEX_SCHEMA *pIndexSchema;
    SCE_DATABASE_INTERNAL *pDatabaseInternal;

    // Accessor build-up members
    DWORD dwBindingIndex;
    DBBINDING *rgBinding;
    SIZE_T cbOffset;
    BYTE *pbData;
};

struct SCE_QUERY_RESULTS
{
    IRowset *pIRowset;
    SCE_TABLE_SCHEMA *pTableSchema;
};

// internal function declarations
static HRESULT RunQuery(
    __in BOOL fRange,
    __in SCE_QUERY_HANDLE psqhHandle,
    __out SCE_QUERY_RESULTS **ppsqrhHandle
    );
static HRESULT EnsureSchema(
    __in SCE_DATABASE *pDatabase,
    __in SCE_DATABASE_SCHEMA *pDatabaseSchema
    );
static HRESULT SetColumnValue(
    __in SCE_TABLE_SCHEMA *pTableSchema,
    __in DWORD dwColumnIndex,
    __in const BYTE *pbData,
    __in SIZE_T cbSize,
    __inout DBBINDING *pBinding,
    __inout SIZE_T *cbOffset,
    __inout BYTE **ppbBuffer
    );
static HRESULT GetColumnValue(
    __in SCE_ROW *pRow,
    __in DWORD dwColumnIndex,
    __out_opt BYTE **ppbData,
    __out SIZE_T *cbSize
    );
static HRESULT GetColumnValueFixed(
    __in SCE_ROW *pRow,
    __in DWORD dwColumnIndex,
    __in DWORD cbSize,
    __out BYTE *pbData
    );
static HRESULT SetDataSourceProperties(
    __in ISessionProperties *pISessionProperties
    );
static HRESULT SetSessionProperties(
    __in ISessionProperties *pISessionProperties
    );
static void ReleaseDatabase(
    SCE_DATABASE *pDatabase
    );
static void ReleaseDatabaseInternal(
    SCE_DATABASE_INTERNAL *pDatabaseInternal
    );

// function definitions
extern "C" HRESULT DAPI SceCreateDatabase(
    __in_z LPCWSTR sczFile,
    __out SCE_DATABASE **ppDatabase
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDirectory = NULL;
    SCE_DATABASE *pNewSceDatabase = NULL;
    SCE_DATABASE_INTERNAL *pNewSceDatabaseInternal = NULL;
    IUnknown *pIUnknownSession = NULL;
    IDBDataSourceAdmin *pIDBDataSourceAdmin = NULL; 
    DBPROPSET rgdbpDataSourcePropSet[2];
    DBPROP rgdbpDataSourceProp[1];
    DBPROP rgdbpDataSourceSsceProp[1];

    pNewSceDatabase = reinterpret_cast<SCE_DATABASE *>(MemAlloc(sizeof(SCE_DATABASE), TRUE));
    ExitOnNull(pNewSceDatabase, hr, E_OUTOFMEMORY, "Failed to allocate SCE_DATABASE struct");

    pNewSceDatabaseInternal = reinterpret_cast<SCE_DATABASE_INTERNAL *>(MemAlloc(sizeof(SCE_DATABASE_INTERNAL), TRUE));
    ExitOnNull(pNewSceDatabaseInternal, hr, E_OUTOFMEMORY, "Failed to allocate SCE_DATABASE_INTERNAL struct");

    pNewSceDatabase->sdbHandle = reinterpret_cast<void *>(pNewSceDatabaseInternal);

    hr = CoCreateInstance(CLSID_SQLSERVERCE_3_5, 0, CLSCTX_INPROC_SERVER, IID_IDBInitialize, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIDBInitialize));
    ExitOnFailure(hr, "Failed to get IDBInitialize interface");

    hr = pNewSceDatabaseInternal->pIDBInitialize->QueryInterface(IID_IDBDataSourceAdmin, reinterpret_cast<void **>(&pIDBDataSourceAdmin));
    ExitOnFailure(hr, "Failed to get IDBDataSourceAdmin interface");

    hr = PathGetDirectory(sczFile, &sczDirectory);
    ExitOnFailure1(hr, "Failed to get directory portion of path: %ls", sczFile);

    hr = DirEnsureExists(sczDirectory, NULL);
    ExitOnFailure1(hr, "Failed to ensure directory exists: %ls", sczDirectory);

    rgdbpDataSourceProp[0].dwPropertyID = DBPROP_INIT_DATASOURCE;
    rgdbpDataSourceProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
    rgdbpDataSourceProp[0].vValue.vt = VT_BSTR;
    rgdbpDataSourceProp[0].vValue.bstrVal = ::SysAllocString(sczFile);

    // SQL CE doesn't seem to allow us to specify DBPROP_INIT_PROMPT if we include any properties from DBPROPSET_SSCE_DBINIT 
    rgdbpDataSourcePropSet[0].guidPropertySet = DBPROPSET_DBINIT;
    rgdbpDataSourcePropSet[0].rgProperties = rgdbpDataSourceProp;
    rgdbpDataSourcePropSet[0].cProperties = _countof(rgdbpDataSourceProp);

    rgdbpDataSourceSsceProp[0].dwPropertyID = DBPROP_SSCE_MAX_DATABASE_SIZE;
    rgdbpDataSourceSsceProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
    rgdbpDataSourceSsceProp[0].vValue.vt = VT_I4;
    rgdbpDataSourceSsceProp[0].vValue.intVal = MAX_SQLCE_DATABASE_SIZE;

    rgdbpDataSourcePropSet[1].guidPropertySet = DBPROPSET_SSCE_DBINIT;
    rgdbpDataSourcePropSet[1].rgProperties = rgdbpDataSourceSsceProp;
    rgdbpDataSourcePropSet[1].cProperties = _countof(rgdbpDataSourceSsceProp);

    hr = pIDBDataSourceAdmin->CreateDataSource(_countof(rgdbpDataSourcePropSet), rgdbpDataSourcePropSet, NULL, IID_IUnknown, &pIUnknownSession);
    ExitOnFailure(hr, "Failed to create data source");

    hr = pNewSceDatabaseInternal->pIDBInitialize->QueryInterface(IID_IDBProperties, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIDBProperties));
    ExitOnFailure(hr, "Failed to get IDBProperties interface");

    hr = pNewSceDatabaseInternal->pIDBInitialize->QueryInterface(IID_IDBCreateSession, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIDBCreateSession));
    ExitOnFailure(hr, "Failed to get IDBCreateSession interface");

    hr = pNewSceDatabaseInternal->pIDBCreateSession->CreateSession(NULL, IID_ISessionProperties, reinterpret_cast<IUnknown **>(&pNewSceDatabaseInternal->pISessionProperties));
    ExitOnFailure(hr, "Failed to get ISessionProperties interface");

    hr = SetSessionProperties(pNewSceDatabaseInternal->pISessionProperties);
    ExitOnFailure(hr, "Failed to set session properties");

    hr = pNewSceDatabaseInternal->pISessionProperties->QueryInterface(IID_IOpenRowset, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIOpenRowset));
    ExitOnFailure(hr, "Failed to get IOpenRowset interface");

    hr = pNewSceDatabaseInternal->pISessionProperties->QueryInterface(IID_ITransactionLocal, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pITransactionLocal));
    ExitOnFailure(hr, "Failed to get ITransactionLocal interface");

    *ppDatabase = pNewSceDatabase;
    pNewSceDatabase = NULL;

LExit:
    ReleaseStr(sczDirectory);
    ReleaseObject(pIUnknownSession);
    ReleaseObject(pIDBDataSourceAdmin);
    ReleaseVariant(rgdbpDataSourceProp[0].vValue);
    ReleaseDatabase(pNewSceDatabase);

    return hr;
}

extern "C" HRESULT DAPI SceOpenDatabase(
    __in_z LPCWSTR sczFile,
    __out SCE_DATABASE **ppDatabase
    )
{
    HRESULT hr = S_OK;
    SCE_DATABASE *pNewSceDatabase = NULL;
    SCE_DATABASE_INTERNAL *pNewSceDatabaseInternal = NULL;
    DBPROPSET rgdbpDataSourcePropSet[2];
    DBPROP rgdbpDataSourceProp[1];
    DBPROP rgdbpDataSourceSsceProp[1];

    pNewSceDatabase = reinterpret_cast<SCE_DATABASE *>(MemAlloc(sizeof(SCE_DATABASE), TRUE));
    ExitOnNull(pNewSceDatabase, hr, E_OUTOFMEMORY, "Failed to allocate SCE_DATABASE struct");

    pNewSceDatabaseInternal = reinterpret_cast<SCE_DATABASE_INTERNAL *>(MemAlloc(sizeof(SCE_DATABASE_INTERNAL), TRUE));
    ExitOnNull(pNewSceDatabaseInternal, hr, E_OUTOFMEMORY, "Failed to allocate SCE_DATABASE_INTERNAL struct");

    pNewSceDatabase->sdbHandle = reinterpret_cast<void *>(pNewSceDatabaseInternal);

    hr = CoCreateInstance(CLSID_SQLSERVERCE_3_5, 0, CLSCTX_INPROC_SERVER, IID_IDBInitialize, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIDBInitialize));
    ExitOnFailure(hr, "Failed to get IDBInitialize interface");

    hr = pNewSceDatabaseInternal->pIDBInitialize->QueryInterface(IID_IDBProperties, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIDBProperties));
    ExitOnFailure(hr, "Failed to get IDBProperties interface");

    rgdbpDataSourceProp[0].dwPropertyID = DBPROP_INIT_DATASOURCE;
    rgdbpDataSourceProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
    rgdbpDataSourceProp[0].vValue.vt = VT_BSTR;
    rgdbpDataSourceProp[0].vValue.bstrVal = ::SysAllocString(sczFile);

    // SQL CE doesn't seem to allow us to specify DBPROP_INIT_PROMPT if we include any properties from DBPROPSET_SSCE_DBINIT 
    rgdbpDataSourcePropSet[0].guidPropertySet = DBPROPSET_DBINIT;
    rgdbpDataSourcePropSet[0].rgProperties = rgdbpDataSourceProp;
    rgdbpDataSourcePropSet[0].cProperties = _countof(rgdbpDataSourceProp);

    rgdbpDataSourceSsceProp[0].dwPropertyID = DBPROP_SSCE_MAX_DATABASE_SIZE;
    rgdbpDataSourceSsceProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
    rgdbpDataSourceSsceProp[0].vValue.vt = VT_I4;
    rgdbpDataSourceSsceProp[0].vValue.lVal = MAX_SQLCE_DATABASE_SIZE;

    rgdbpDataSourcePropSet[1].guidPropertySet = DBPROPSET_SSCE_DBINIT;
    rgdbpDataSourcePropSet[1].rgProperties = rgdbpDataSourceSsceProp;
    rgdbpDataSourcePropSet[1].cProperties = _countof(rgdbpDataSourceSsceProp);

    hr = pNewSceDatabaseInternal->pIDBProperties->SetProperties(_countof(rgdbpDataSourcePropSet), rgdbpDataSourcePropSet);
    ExitOnFailure(hr, "Failed to set initial properties to open database");

    hr = pNewSceDatabaseInternal->pIDBInitialize->Initialize();
    ExitOnFailure1(hr, "Failed to open database: %ls", sczFile);

    hr = pNewSceDatabaseInternal->pIDBInitialize->QueryInterface(IID_IDBCreateSession, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIDBCreateSession));
    ExitOnFailure(hr, "Failed to get IDBCreateSession interface");

    hr = pNewSceDatabaseInternal->pIDBCreateSession->CreateSession(NULL, IID_ISessionProperties, reinterpret_cast<IUnknown **>(&pNewSceDatabaseInternal->pISessionProperties));
    ExitOnFailure(hr, "Failed to get ISessionProperties interface");

    hr = SetSessionProperties(pNewSceDatabaseInternal->pISessionProperties);
    ExitOnFailure(hr, "Failed to set session properties");

    hr = pNewSceDatabaseInternal->pISessionProperties->QueryInterface(IID_IOpenRowset, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pIOpenRowset));
    ExitOnFailure(hr, "Failed to get IOpenRowset interface");

    hr = pNewSceDatabaseInternal->pISessionProperties->QueryInterface(IID_ITransactionLocal, reinterpret_cast<void **>(&pNewSceDatabaseInternal->pITransactionLocal));
    ExitOnFailure(hr, "Failed to get ITransactionLocal interface");

    *ppDatabase = pNewSceDatabase;
    pNewSceDatabase = NULL;

LExit:
    ReleaseDatabase(pNewSceDatabase);

    return hr;
}

extern "C" HRESULT DAPI SceEnsureDatabase(
    __in_z LPCWSTR sczFile,
    __in SCE_DATABASE_SCHEMA *pdsSchema,
    __out SCE_DATABASE **ppDatabase
    )
{
    HRESULT hr = S_OK;

    if (FileExistsEx(sczFile, NULL))
    {
        hr = SceOpenDatabase(sczFile, ppDatabase);
        ExitOnFailure1(hr, "Failed to open database while ensuring database exists: %ls", sczFile);
    }
    else
    {
        hr = SceCreateDatabase(sczFile, ppDatabase);
        ExitOnFailure1(hr, "Failed to create database while ensuring database exists: %ls", sczFile);
    }

    hr = EnsureSchema(*ppDatabase, pdsSchema);
    ExitOnFailure1(hr, "Failed to ensure schema is correct in database: %ls", sczFile);
    
LExit:
    return hr;
}

extern "C" HRESULT DAPI SceIsTableEmpty(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out BOOL *pfEmpty
    )
{
    HRESULT hr = S_OK;
    SCE_ROW_HANDLE row = NULL;

    hr = SceGetFirstRow(pDatabase, dwTableIndex, &row);
    if (E_NOTFOUND == hr)
    {
        *pfEmpty = TRUE;
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to get first row while checking if table is empty");

    *pfEmpty = FALSE;

LExit:
    ReleaseSceRow(row);

    return hr;
}

extern "C" HRESULT DAPI SceGetFirstRow(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out SCE_ROW_HANDLE *pRowHandle
    )
{
    HRESULT hr = S_OK;
    DBCOUNTITEM cRowsObtained = 0;
    HROW hRow = DB_NULL_HROW;
    HROW *phRow = &hRow;
    SCE_ROW *pRow = NULL;
    SCE_TABLE_SCHEMA *pTable = &(pDatabase->pdsSchema->rgTables[dwTableIndex]);

    hr = pTable->pIRowset->RestartPosition(DB_NULL_HCHAPTER);
    ExitOnFailure(hr, "Failed to reset IRowset position to beginning");

    hr = pTable->pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow);
    if (DB_S_ENDOFROWSET == hr)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }
    ExitOnFailure(hr, "Failed to get next first row");

    pRow = reinterpret_cast<SCE_ROW *>(MemAlloc(sizeof(SCE_ROW), TRUE));
    ExitOnNull(pRow, hr, E_OUTOFMEMORY, "Failed to allocate SCE_ROW struct");

    pRow->hRow = hRow;
    pRow->pTableSchema = pTable;
    pRow->pIRowset = pTable->pIRowset;
    pRow->pIRowset->AddRef();

    *pRowHandle = reinterpret_cast<SCE_ROW_HANDLE>(pRow);

LExit:
    return hr;
}

HRESULT DAPI SceGetNextRow(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out SCE_ROW_HANDLE *pRowHandle
    )
{
    HRESULT hr = S_OK;
    DBCOUNTITEM cRowsObtained = 0;
    HROW hRow = DB_NULL_HROW;
    HROW *phRow = &hRow;
    SCE_ROW *pRow = NULL;
    SCE_TABLE_SCHEMA *pTable = &(pDatabase->pdsSchema->rgTables[dwTableIndex]);

    hr = pTable->pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow);
    if (DB_S_ENDOFROWSET == hr)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }
    ExitOnFailure(hr, "Failed to get next first row");

    pRow = reinterpret_cast<SCE_ROW *>(MemAlloc(sizeof(SCE_ROW), TRUE));
    ExitOnNull(pRow, hr, E_OUTOFMEMORY, "Failed to allocate SCE_ROW struct");

    pRow->hRow = hRow;
    pRow->pTableSchema = pTable;
    pRow->pIRowset = pTable->pIRowset;
    pRow->pIRowset->AddRef();

    *pRowHandle = reinterpret_cast<SCE_ROW_HANDLE>(pRow);

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceBeginTransaction(
    __in SCE_DATABASE *pDatabase
    )
{
    HRESULT hr = S_OK;
    SCE_DATABASE_INTERNAL *pDatabaseInternal = reinterpret_cast<SCE_DATABASE_INTERNAL *>(pDatabase->sdbHandle);

    ::InterlockedIncrement(&pDatabaseInternal->dwTransactionRefcount);

    if (1 == pDatabaseInternal->dwTransactionRefcount)
    {
        hr = pDatabaseInternal->pITransactionLocal->StartTransaction(ISOLATIONLEVEL_SERIALIZABLE, 0, NULL, NULL);
        ExitOnFailure(hr, "Failed to start transaction");
    }

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceCommitTransaction(
    __in SCE_DATABASE *pDatabase
    )
{
    HRESULT hr = S_OK;
    SCE_DATABASE_INTERNAL *pDatabaseInternal = reinterpret_cast<SCE_DATABASE_INTERNAL *>(pDatabase->sdbHandle);

    ::InterlockedDecrement(&pDatabaseInternal->dwTransactionRefcount);

    if (0 == pDatabaseInternal->dwTransactionRefcount)
    {
        hr = pDatabaseInternal->pITransactionLocal->Commit(FALSE, XACTTC_SYNC, 0);
        ExitOnFailure(hr, "Failed to commit transaction");
    }

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceRollbackTransaction(
    __in SCE_DATABASE *pDatabase
    )
{
    HRESULT hr = S_OK;
    SCE_DATABASE_INTERNAL *pDatabaseInternal = reinterpret_cast<SCE_DATABASE_INTERNAL *>(pDatabase->sdbHandle);

    ::InterlockedDecrement(&pDatabaseInternal->dwTransactionRefcount);

    if (0 == pDatabaseInternal->dwTransactionRefcount)
    {
        hr = pDatabaseInternal->pITransactionLocal->Abort(NULL, FALSE, FALSE);
        ExitOnFailure(hr, "Failed to abort transaction");
    }

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceDeleteRow(
    __in SCE_ROW_HANDLE *pRowHandle
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(*pRowHandle);
    IRowsetChange *pIRowsetChange = NULL;
    DBROWSTATUS rowStatus = DBROWSTATUS_S_OK;

    hr = pRow->pIRowset->QueryInterface(IID_IRowsetChange, reinterpret_cast<void **>(&pIRowsetChange));
    ExitOnFailure(hr, "Failed to get IRowsetChange interface");

    hr = pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER, 1, &pRow->hRow, &rowStatus);
    ExitOnFailure1(hr, "Failed to delete row with status: %u", rowStatus);

    ReleaseNullSceRow(*pRowHandle);

LExit:
    ReleaseObject(pIRowsetChange);

    return hr;
}

extern "C" HRESULT DAPI ScePrepareInsert(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __out SCE_ROW_HANDLE *pRowHandle
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = NULL;

    pRow = reinterpret_cast<SCE_ROW *>(MemAlloc(sizeof(SCE_ROW), TRUE));
    ExitOnNull(pRow, hr, E_OUTOFMEMORY, "Failed to allocate SCE_ROW struct");

    pRow->hRow = DB_NULL_HROW;
    pRow->pTableSchema = &(pDatabase->pdsSchema->rgTables[dwTableIndex]);
    pRow->pIRowset = pRow->pTableSchema->pIRowset;
    pRow->pIRowset->AddRef();
    pRow->fInserting = TRUE;

    *pRowHandle = reinterpret_cast<SCE_ROW_HANDLE>(pRow);
    pRow = NULL;

LExit:
    ReleaseMem(pRow);

    return hr;
}

extern "C" HRESULT DAPI SceFinishUpdate(
    __in SCE_ROW_HANDLE rowHandle
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);
    IAccessor *pIAccessor = NULL;
    IRowsetChange *pIRowsetChange = NULL;
    HACCESSOR hAccessor = DB_NULL_HACCESSOR;
    HROW hRow = DB_NULL_HROW;

    hr = pRow->pIRowset->QueryInterface(IID_IAccessor, reinterpret_cast<void **>(&pIAccessor));
    ExitOnFailure(hr, "Failed to get IAccessor interface");

    hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, pRow->dwBindingIndex, pRow->rgBinding, 0, &hAccessor, NULL);
    ExitOnFailure(hr, "Failed to create accessor");

    hr = pRow->pIRowset->QueryInterface(IID_IRowsetChange, reinterpret_cast<void **>(&pIRowsetChange));
    ExitOnFailure(hr, "Failed to get IRowsetChange interface");

    if (pRow->fInserting)
    {
        hr = pIRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, pRow->pbData, &hRow);
        ExitOnFailure(hr, "Failed to insert new row");

        pRow->hRow = hRow;
        ReleaseNullObject(pRow->pIRowset);
        pRow->pIRowset = pRow->pTableSchema->pIRowset;
        pRow->pIRowset->AddRef();
    }
    else
    {
        hr = pIRowsetChange->SetData(pRow->hRow, hAccessor, pRow->pbData);
        ExitOnFailure(hr, "Failed to update existing row");
    }

LExit:
    if (DB_NULL_HACCESSOR != hAccessor)
    {
        pIAccessor->ReleaseAccessor(hAccessor, NULL);
    }
    ReleaseObject(pIAccessor);
    ReleaseObject(pIRowsetChange);

    return hr;
}

extern "C" HRESULT DAPI SceSetColumnBinary(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in_bcount(cbBuffer) const BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);

    if (NULL == pRow->rgBinding)
    {
        pRow->rgBinding = static_cast<DBBINDING *>(MemAlloc(sizeof(DBBINDING) * pRow->pTableSchema->cColumns, TRUE));
        ExitOnNull(pRow->rgBinding, hr, E_OUTOFMEMORY, "Failed to allocate DBBINDINGs for sce row writer");
    }

    hr = SetColumnValue(pRow->pTableSchema, dwColumnIndex, pbBuffer, cbBuffer, &pRow->rgBinding[pRow->dwBindingIndex++], &pRow->cbOffset, &pRow->pbData);
    ExitOnFailure(hr, "Failed to set column value as binary");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceSetColumnDword(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in const DWORD dwValue
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);

    if (NULL == pRow->rgBinding)
    {
        pRow->rgBinding = static_cast<DBBINDING *>(MemAlloc(sizeof(DBBINDING) * pRow->pTableSchema->cColumns, TRUE));
        ExitOnNull(pRow->rgBinding, hr, E_OUTOFMEMORY, "Failed to allocate DBBINDINGs for sce row writer");
    }

    hr = SetColumnValue(pRow->pTableSchema, dwColumnIndex, reinterpret_cast<const BYTE *>(&dwValue), 4, &pRow->rgBinding[pRow->dwBindingIndex++], &pRow->cbOffset, &pRow->pbData);
    ExitOnFailure(hr, "Failed to set column value as binary");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceSetColumnBool(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in const BOOL fValue
    )
{
    HRESULT hr = S_OK;
    short int sValue = fValue ? 0xFFFF : 0x0000;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);

    if (NULL == pRow->rgBinding)
    {
        pRow->rgBinding = static_cast<DBBINDING *>(MemAlloc(sizeof(DBBINDING) * pRow->pTableSchema->cColumns, TRUE));
        ExitOnNull(pRow->rgBinding, hr, E_OUTOFMEMORY, "Failed to allocate DBBINDINGs for sce row writer");
    }

    hr = SetColumnValue(pRow->pTableSchema, dwColumnIndex, reinterpret_cast<const BYTE *>(&sValue), 2, &pRow->rgBinding[pRow->dwBindingIndex++], &pRow->cbOffset, &pRow->pbData);
    ExitOnFailure(hr, "Failed to set column value as binary");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceSetColumnString(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in_z LPCWSTR pszValue
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);
    SIZE_T cbSize = (lstrlenW(pszValue) + 1) * sizeof(WCHAR);

    if (NULL == pRow->rgBinding)
    {
        pRow->rgBinding = static_cast<DBBINDING *>(MemAlloc(sizeof(DBBINDING) * pRow->pTableSchema->cColumns, TRUE));
        ExitOnNull(pRow->rgBinding, hr, E_OUTOFMEMORY, "Failed to allocate DBBINDINGs for sce row writer");
    }

    hr = SetColumnValue(pRow->pTableSchema, dwColumnIndex, reinterpret_cast<const BYTE *>(pszValue), cbSize, &pRow->rgBinding[pRow->dwBindingIndex++], &pRow->cbOffset, &pRow->pbData);
    ExitOnFailure1(hr, "Failed to set column value as string: %ls", pszValue);

LExit:
    return hr;
}

HRESULT DAPI SceSetColumnEmpty(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);

    if (NULL == pRow->rgBinding)
    {
        pRow->rgBinding = static_cast<DBBINDING *>(MemAlloc(sizeof(DBBINDING) * pRow->pTableSchema->cColumns, TRUE));
        ExitOnNull(pRow->rgBinding, hr, E_OUTOFMEMORY, "Failed to allocate DBBINDINGs for sce row writer");
    }

    hr = SetColumnValue(pRow->pTableSchema, dwColumnIndex, NULL, 0, &pRow->rgBinding[pRow->dwBindingIndex++], &pRow->cbOffset, &pRow->pbData);
    ExitOnFailure(hr, "Failed to set column value as binary");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceSetColumnSystemTime(
    __in SCE_ROW_HANDLE rowHandle,
    __in DWORD dwColumnIndex,
    __in const SYSTEMTIME *pst
    )
{
    HRESULT hr = S_OK;
    DBTIMESTAMP dbTimeStamp = { };

    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);

    if (NULL == pRow->rgBinding)
    {
        pRow->rgBinding = static_cast<DBBINDING *>(MemAlloc(sizeof(DBBINDING) * pRow->pTableSchema->cColumns, TRUE));
        ExitOnNull(pRow->rgBinding, hr, E_OUTOFMEMORY, "Failed to allocate DBBINDINGs for sce row writer");
    }

    dbTimeStamp.year = pst->wYear;
    dbTimeStamp.month = pst->wMonth;
    dbTimeStamp.day = pst->wDay;
    dbTimeStamp.hour = pst->wHour;
    dbTimeStamp.minute = pst->wMinute;
    dbTimeStamp.second = pst->wSecond;
    // fraction represents nanoseconds (millionths of a second) - so multiply milliseconds by 1 million to get there
    dbTimeStamp.fraction = pst->wMilliseconds * 1000000;

    hr = SetColumnValue(pRow->pTableSchema, dwColumnIndex, reinterpret_cast<BYTE *>(&dbTimeStamp), sizeof(dbTimeStamp), &pRow->rgBinding[pRow->dwBindingIndex++], &pRow->cbOffset, &pRow->pbData);
    ExitOnFailure(hr, "Failed to set column value as DBTIMESTAMPOFFSET");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceGetColumnBinary(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out_opt BYTE **ppbBuffer,
    __inout SIZE_T *pcbBuffer
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowReadHandle);

    hr = GetColumnValue(pRow, dwColumnIndex, ppbBuffer, pcbBuffer);
    if (E_NOTFOUND == hr)
    {
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to get binary data out of column");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceGetColumnDword(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out DWORD *pdwValue
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowReadHandle);

    hr = GetColumnValueFixed(pRow, dwColumnIndex, 4, reinterpret_cast<BYTE *>(pdwValue));
    if (E_NOTFOUND == hr)
    {
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to get dword data out of column");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceGetColumnBool(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out BOOL *pfValue
    )
{
    HRESULT hr = S_OK;
    short int sValue = 0;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowReadHandle);

    hr = GetColumnValueFixed(pRow, dwColumnIndex, 2, reinterpret_cast<BYTE *>(&sValue));
    if (E_NOTFOUND == hr)
    {
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to get data out of column");

    if (sValue == 0x0000)
    {
        *pfValue = FALSE;
    }
    else
    {
        *pfValue = TRUE;
    }

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceGetColumnString(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out LPWSTR *ppszValue
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowReadHandle);
    SIZE_T cbSize = 0;

    hr = GetColumnValue(pRow, dwColumnIndex, reinterpret_cast<BYTE **>(ppszValue), &cbSize);
    if (E_NOTFOUND == hr)
    {
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to get string data out of column");

LExit:
    return hr;
}

extern "C" HRESULT DAPI SceGetColumnSystemTime(
    __in SCE_ROW_HANDLE rowReadHandle,
    __in DWORD dwColumnIndex,
    __out SYSTEMTIME *pst
    )
{
    HRESULT hr = S_OK;
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowReadHandle);
    DBTIMESTAMP dbTimeStamp = { };

    hr = GetColumnValueFixed(pRow, dwColumnIndex, sizeof(dbTimeStamp), reinterpret_cast<BYTE *>(&dbTimeStamp));
    if (E_NOTFOUND == hr)
    {
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to get string data out of column");

    pst->wYear = dbTimeStamp.year;
    pst->wMonth = dbTimeStamp.month;
    pst->wDay = dbTimeStamp.day;
    pst->wHour = dbTimeStamp.hour;
    pst->wMinute = dbTimeStamp.minute;
    pst->wSecond = dbTimeStamp.second;
    // fraction represents nanoseconds (millionths of a second) - so divide fraction by 1 million to get to milliseconds
    pst->wMilliseconds = static_cast<WORD>(dbTimeStamp.fraction / 1000000);

LExit:
    return hr;
}

extern "C" void DAPI SceCloseTable(
    __in SCE_TABLE_SCHEMA *pTable
    )
{
    ReleaseObject(pTable->pIRowset);
    ReleaseObject(pTable->pIRowsetChange);
}

extern "C" HRESULT DAPI SceCloseDatabase(
    __in SCE_DATABASE *pDatabase
    )
{
    HRESULT hr = S_OK;

    ReleaseDatabase(pDatabase);

    return hr;
}

extern "C" HRESULT DAPI SceBeginQuery(
    __in SCE_DATABASE *pDatabase,
    __in DWORD dwTableIndex,
    __in DWORD dwIndex,
    __out SCE_QUERY_HANDLE *psqhHandle
    )
{
    HRESULT hr = S_OK;
    SCE_QUERY *psq = static_cast<SCE_QUERY*>(MemAlloc(sizeof(SCE_QUERY), TRUE));
    ExitOnNull(psq, hr, E_OUTOFMEMORY, "Failed to allocate new sce query");

    psq->pTableSchema = &(pDatabase->pdsSchema->rgTables[dwTableIndex]);
    psq->pIndexSchema = &(psq->pTableSchema->rgIndexes[dwIndex]);
    psq->pDatabaseInternal = reinterpret_cast<SCE_DATABASE_INTERNAL *>(pDatabase->sdbHandle);

    psq->rgBinding = static_cast<DBBINDING *>(MemAlloc(sizeof(DBBINDING) * psq->pTableSchema->cColumns, TRUE));
    ExitOnNull(psq, hr, E_OUTOFMEMORY, "Failed to allocate DBBINDINGs for new sce query");

    *psqhHandle = static_cast<SCE_QUERY_HANDLE>(psq);
    psq = NULL;

LExit:
    if (psq != NULL)
    {
        ReleaseMem(psq->rgBinding);
        ReleaseMem(psq);
    }

    return hr;
}

HRESULT DAPI SceSetQueryColumnDword(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in const DWORD dwValue
    )
{
    HRESULT hr = S_OK;
    SCE_QUERY *pQuery = reinterpret_cast<SCE_QUERY *>(sqhHandle);

    hr = SetColumnValue(pQuery->pTableSchema, pQuery->pIndexSchema->rgColumns[pQuery->dwBindingIndex], reinterpret_cast<const BYTE *>(&dwValue), 4, &pQuery->rgBinding[pQuery->dwBindingIndex], &pQuery->cbOffset, &pQuery->pbData);
    ExitOnFailure(hr, "Failed to set column value as binary");

    ++(pQuery->dwBindingIndex);

LExit:
    return hr;
}

HRESULT DAPI SceSetQueryColumnString(
    __in SCE_QUERY_HANDLE sqhHandle,
    __in_z LPCWSTR pszString
    )
{
    HRESULT hr = S_OK;
    SCE_QUERY *pQuery = reinterpret_cast<SCE_QUERY *>(sqhHandle);
    SIZE_T cbSize = (lstrlenW(pszString) + 1) * sizeof(WCHAR);

    hr = SetColumnValue(pQuery->pTableSchema, pQuery->pIndexSchema->rgColumns[pQuery->dwBindingIndex], reinterpret_cast<const BYTE *>(pszString), cbSize, &pQuery->rgBinding[pQuery->dwBindingIndex], &pQuery->cbOffset, &pQuery->pbData);
    ExitOnFailure(hr, "Failed to set column value as binary");

    ++(pQuery->dwBindingIndex);

LExit:
    return hr;
}

HRESULT DAPI SceRunQueryExact(
    __in SCE_QUERY_HANDLE *psqhHandle,
    __out SCE_ROW_HANDLE *pRowHandle
    )
{
    HRESULT hr = S_OK;
    SCE_QUERY_RESULTS *pQueryResults = NULL;

    hr = RunQuery(FALSE, *psqhHandle, &pQueryResults);
    ExitOnFailure(hr, "Failed to run query exact");

    hr = SceGetNextResultRow(reinterpret_cast<SCE_QUERY_RESULTS_HANDLE>(pQueryResults), pRowHandle);
    if (E_NOTFOUND == hr)
    {
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to get next row out of results");

LExit:
    ReleaseNullSceQuery(*psqhHandle);
    ReleaseSceQueryResults(pQueryResults);

    return hr;
}

extern "C" HRESULT DAPI SceRunQueryRange(
    __in SCE_QUERY_HANDLE *psqhHandle,
    __out SCE_QUERY_RESULTS_HANDLE *psqrhHandle
    )
{
    HRESULT hr = S_OK;
    SCE_QUERY_RESULTS **ppQueryResults = reinterpret_cast<SCE_QUERY_RESULTS **>(psqrhHandle);

    hr = RunQuery(TRUE, *psqhHandle, ppQueryResults);
    ExitOnFailure(hr, "Failed to run query for range");

LExit:
    ReleaseNullSceQuery(*psqhHandle);

    return hr;
}

extern "C" HRESULT DAPI SceGetNextResultRow(
    __in SCE_QUERY_RESULTS_HANDLE sqrhHandle,
    __out SCE_ROW_HANDLE *pRowHandle
    )
{
    HRESULT hr = S_OK;
    HROW hRow = DB_NULL_HROW;
    HROW *phRow = &hRow;
    DBCOUNTITEM cRowsObtained = 0;
    SCE_ROW *pRow = NULL;
    SCE_QUERY_RESULTS *pQueryResults = reinterpret_cast<SCE_QUERY_RESULTS *>(sqrhHandle);

    Assert(pRowHandle && (*pRowHandle == NULL));

    hr = pQueryResults->pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow);
    if (DB_S_ENDOFROWSET == hr)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }
    ExitOnFailure(hr, "Failed to get next first row");

    pRow = reinterpret_cast<SCE_ROW *>(MemAlloc(sizeof(SCE_ROW), TRUE));
    ExitOnNull(pRow, hr, E_OUTOFMEMORY, "Failed to allocate SCE_ROW struct");

    pRow->hRow = hRow;
    pRow->pTableSchema = pQueryResults->pTableSchema;
    pRow->pIRowset = pQueryResults->pIRowset;
    pRow->pIRowset->AddRef();

    *pRowHandle = reinterpret_cast<SCE_ROW_HANDLE>(pRow);
    pRow = NULL;
    hRow = DB_NULL_HROW;

LExit:
    if (DB_NULL_HROW != hRow)
    {
        pQueryResults->pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
    }
    ReleaseMem(pRow);

    return hr;
}

extern "C" void DAPI SceFreeRow(
    __in SCE_ROW_HANDLE rowHandle
    )
{
    SCE_ROW *pRow = reinterpret_cast<SCE_ROW *>(rowHandle);

    if (DB_NULL_HROW != pRow->hRow)
    {
        pRow->pIRowset->ReleaseRows(1, &pRow->hRow, NULL, NULL, NULL);
    }
    ReleaseObject(pRow->pIRowset);
    ReleaseMem(pRow->rgBinding);
    ReleaseMem(pRow->pbData);
    ReleaseMem(pRow);
}

void DAPI SceFreeQuery(
    __in SCE_QUERY_HANDLE sqhHandle
    )
{
    SCE_QUERY *pQuery = reinterpret_cast<SCE_QUERY *>(sqhHandle);

    ReleaseMem(pQuery->rgBinding);
    ReleaseMem(pQuery->pbData);
    ReleaseMem(pQuery);
}

void DAPI SceFreeQueryResults(
    __in SCE_QUERY_RESULTS_HANDLE sqrhHandle
    )
{
    SCE_QUERY_RESULTS *pQueryResults = reinterpret_cast<SCE_QUERY_RESULTS *>(sqrhHandle);

    ReleaseObject(pQueryResults->pIRowset);
    ReleaseMem(pQueryResults);
}

// internal function definitions
static HRESULT RunQuery(
    __in BOOL fRange,
    __in SCE_QUERY_HANDLE psqhHandle,
    __out SCE_QUERY_RESULTS **ppQueryResults
    )
{
    HRESULT hr = S_OK;
    DBID tableID = { };
    DBID indexID = { };
    IAccessor *pIAccessor = NULL;
    IRowsetIndex *pIRowsetIndex = NULL;
    IRowset *pIRowset = NULL;
    HACCESSOR hAccessor = DB_NULL_HACCESSOR;
    SCE_QUERY *pQuery = reinterpret_cast<SCE_QUERY *>(psqhHandle);
    SCE_QUERY_RESULTS *pQueryResults = NULL;
    DBPROPSET rgdbpIndexPropSet[1];
    DBPROP rgdbpIndexProp[1];

    rgdbpIndexPropSet[0].cProperties     = 1;
    rgdbpIndexPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
    rgdbpIndexPropSet[0].rgProperties    = rgdbpIndexProp;

    rgdbpIndexProp[0].dwPropertyID       = DBPROP_IRowsetIndex;
    rgdbpIndexProp[0].dwOptions          = DBPROPOPTIONS_REQUIRED;
    rgdbpIndexProp[0].colid              = DB_NULLID;
    rgdbpIndexProp[0].vValue.vt          = VT_BOOL;
    rgdbpIndexProp[0].vValue.boolVal     = VARIANT_TRUE;

    tableID.eKind = DBKIND_NAME;
    tableID.uName.pwszName = const_cast<WCHAR *>(pQuery->pTableSchema->sczName);

    indexID.eKind = DBKIND_NAME;
    indexID.uName.pwszName = const_cast<WCHAR *>(pQuery->pIndexSchema->sczName);

    hr = pQuery->pDatabaseInternal->pIOpenRowset->OpenRowset(NULL, &tableID, &indexID, IID_IRowsetIndex, _countof(rgdbpIndexPropSet), rgdbpIndexPropSet, (IUnknown**) &pIRowsetIndex);
    ExitOnFailure(hr, "Failed to open IRowsetIndex");

    hr = pIRowsetIndex->QueryInterface(IID_IRowset, reinterpret_cast<void **>(&pIRowset));
    ExitOnFailure(hr, "Failed to get IRowset interface from IRowsetIndex");

    hr = pIRowset->QueryInterface(IID_IAccessor, reinterpret_cast<void **>(&pIAccessor));
    ExitOnFailure(hr, "Failed to get IAccessor interface");

    hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, pQuery->dwBindingIndex, pQuery->rgBinding, 0, &hAccessor, NULL);
    ExitOnFailure(hr, "Failed to create accessor");

    if (!fRange)
    {
        hr = pIRowsetIndex->Seek(hAccessor, pQuery->dwBindingIndex, pQuery->pbData, DBSEEK_FIRSTEQ);
        if (DB_E_NOTFOUND == hr)
        {
            ExitFunction1(hr = E_NOTFOUND);
        }
        ExitOnFailure(hr, "Failed to seek to record");
    }
    else
    {
        hr = pIRowsetIndex->SetRange(hAccessor, pQuery->dwBindingIndex, pQuery->pbData, 0, NULL, DBRANGE_MATCH);
        if (DB_E_NOTFOUND == hr)
        {
            ExitFunction1(hr = E_NOTFOUND);
        }
        ExitOnFailure(hr, "Failed to set range to find records");
    }

    pQueryResults = reinterpret_cast<SCE_QUERY_RESULTS *>(MemAlloc(sizeof(SCE_QUERY_RESULTS), TRUE));
    ExitOnNull(pQueryResults, hr, E_OUTOFMEMORY, "Failed to allocate query results struct");

    pQueryResults->pTableSchema = pQuery->pTableSchema;
    pQueryResults->pIRowset = pIRowset;
    pIRowset = NULL;

    *ppQueryResults = pQueryResults;
    pQueryResults = NULL;

LExit:
    if (DB_NULL_HACCESSOR != hAccessor)
    {
        pIAccessor->ReleaseAccessor(hAccessor, NULL);
    }
    ReleaseObject(pIAccessor);
    ReleaseObject(pIRowset);
    ReleaseObject(pIRowsetIndex);
    ReleaseMem(pQueryResults);
    ReleaseSceQueryResults(pQueryResults);

    return hr;
}

static HRESULT EnsureSchema(
    __in SCE_DATABASE *pDatabase,
    __in SCE_DATABASE_SCHEMA *pdsSchema
    )
{
    HRESULT hr = S_OK;
    BOOL fInTransaction = FALSE;
    DBID tableID = { };
    DBID indexID = { };
    DBPROPSET rgdbpRowSetPropset[1];
    DBPROP rgdbpRowSetProp[1];
    DBPROPSET rgdbpIndexPropset[1];
    DBPROP rgdbpIndexProp[1];
    DWORD dwColumnProperties = 0;
    DWORD dwColumnPropertyIndex = 0;
    DBCOLUMNDESC *rgColumnDescriptions = NULL;
    DBINDEXCOLUMNDESC *rgIndexColumnDescriptions = NULL;
    SCE_DATABASE_INTERNAL *pDatabaseInternal = reinterpret_cast<SCE_DATABASE_INTERNAL *>(pDatabase->sdbHandle);
    ITableDefinition *pTableDefinition = NULL;
    IIndexDefinition *pIndexDefinition = NULL;

    // Keep a pointer to the schema in the SCE_DATABASE object for future reference
    pDatabase->pdsSchema = pdsSchema;

    rgdbpRowSetPropset[0].cProperties = 1;
    rgdbpRowSetPropset[0].guidPropertySet = DBPROPSET_ROWSET;
    rgdbpRowSetPropset[0].rgProperties = rgdbpRowSetProp;

    rgdbpRowSetProp[0].dwPropertyID = DBPROP_IRowsetChange;
    rgdbpRowSetProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
    rgdbpRowSetProp[0].colid = DB_NULLID;
    rgdbpRowSetProp[0].vValue.vt = VT_BOOL;
    rgdbpRowSetProp[0].vValue.boolVal = VARIANT_TRUE;

    rgdbpIndexPropset[0].cProperties = 1;
    rgdbpIndexPropset[0].guidPropertySet = DBPROPSET_INDEX;
    rgdbpIndexPropset[0].rgProperties = rgdbpIndexProp;

    rgdbpIndexProp[0].dwPropertyID = DBPROP_INDEX_NULLS;
    rgdbpIndexProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
    rgdbpIndexProp[0].colid = DB_NULLID;
    rgdbpIndexProp[0].vValue.vt = VT_I4;
    rgdbpIndexProp[0].vValue.lVal = DBPROPVAL_IN_DISALLOWNULL;

    hr = pDatabaseInternal->pISessionProperties->QueryInterface(IID_ITableDefinition, reinterpret_cast<void **>(&pTableDefinition));
    ExitOnFailure(hr, "Failed to get ITableDefinition for table creation");

    hr = pDatabaseInternal->pISessionProperties->QueryInterface(IID_IIndexDefinition, reinterpret_cast<void **>(&pIndexDefinition));
    ExitOnFailure(hr, "Failed to get IIndexDefinition for index creation");

    hr = SceBeginTransaction(pDatabase);
    ExitOnFailure(hr, "Failed to start transaction for ensuring schema");
    fInTransaction = TRUE;

    for (DWORD dwTable = 0; dwTable < pdsSchema->cTables; ++dwTable)
    {
        // Don't free this pointer - it's just a shortcut to the current table's name within the struct
        LPCWSTR pwzTableName = pdsSchema->rgTables[dwTable].sczName;

        tableID.eKind = DBKIND_NAME;
        tableID.uName.pwszName = const_cast<WCHAR *>(pwzTableName);

        // First try to open the table - or if it doesn't exist, create it
        hr = pDatabaseInternal->pIOpenRowset->OpenRowset(NULL, &tableID, NULL, IID_IRowset, _countof(rgdbpRowSetPropset), rgdbpRowSetPropset, reinterpret_cast<IUnknown **>(&pdsSchema->rgTables[dwTable].pIRowset));
        if (DB_E_NOTABLE == hr)
        {
            // The table doesn't exist, so let's create it
            rgColumnDescriptions = static_cast<DBCOLUMNDESC *>(MemAlloc(sizeof(DBCOLUMNDESC) * pdsSchema->rgTables[dwTable].cColumns, TRUE));
            ExitOnNull(rgColumnDescriptions, hr, E_OUTOFMEMORY, "Failed to allocate column description array while creating table");

            // Fill out each column description struct as appropriate
            for (DWORD i = 0; i < pdsSchema->rgTables[dwTable].cColumns; ++i)
            {
                rgColumnDescriptions[i].dbcid.eKind = DBKIND_NAME;
                rgColumnDescriptions[i].dbcid.uName.pwszName = (WCHAR *)pdsSchema->rgTables[dwTable].rgColumns[i].sczName;
                rgColumnDescriptions[i].wType = pdsSchema->rgTables[dwTable].rgColumns[i].dbtColumnType;
                rgColumnDescriptions[i].ulColumnSize = pdsSchema->rgTables[dwTable].rgColumns[i].dwLength;
                if (0 == rgColumnDescriptions[i].ulColumnSize && (DBTYPE_WSTR == rgColumnDescriptions[i].wType || DBTYPE_BYTES == rgColumnDescriptions[i].wType))
                {
                    // TODO: what exactly is the right size here?
                    rgColumnDescriptions[i].ulColumnSize = 255;
                }

                dwColumnProperties = 0;
                if (pdsSchema->rgTables[dwTable].rgColumns[i].fAutoIncrement)
                {
                    ++dwColumnProperties;
                }
                if (!pdsSchema->rgTables[dwTable].rgColumns[i].fNullable)
                {
                    ++dwColumnProperties;
                }

                if (0 < dwColumnProperties)
                {
                    rgColumnDescriptions[i].cPropertySets = 1;
                    rgColumnDescriptions[i].rgPropertySets = reinterpret_cast<DBPROPSET *>(MemAlloc(sizeof(DBPROPSET), TRUE));
                    ExitOnNull(rgColumnDescriptions[i].rgPropertySets, hr, E_OUTOFMEMORY, "Failed to allocate propset object while setting up column parameters");

                    rgColumnDescriptions[i].rgPropertySets[0].cProperties = dwColumnProperties;
                    rgColumnDescriptions[i].rgPropertySets[0].guidPropertySet = DBPROPSET_COLUMN;
                    rgColumnDescriptions[i].rgPropertySets[0].rgProperties = reinterpret_cast<DBPROP *>(MemAlloc(sizeof(DBPROP) * dwColumnProperties, TRUE));

                    dwColumnPropertyIndex = 0;
                    if (pdsSchema->rgTables[dwTable].rgColumns[i].fAutoIncrement)
                    {
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].dwPropertyID = DBPROP_COL_AUTOINCREMENT;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].dwOptions = DBPROPOPTIONS_REQUIRED;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].colid = DB_NULLID;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].vValue.vt = VT_BOOL;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].vValue.boolVal = VARIANT_TRUE;
                        ++dwColumnPropertyIndex;
                    }
                    if (!pdsSchema->rgTables[dwTable].rgColumns[i].fNullable)
                    {
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].dwPropertyID = DBPROP_COL_NULLABLE;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].dwOptions = DBPROPOPTIONS_REQUIRED;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].colid = DB_NULLID;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].vValue.vt = VT_BOOL;
                        rgColumnDescriptions[i].rgPropertySets[0].rgProperties[dwColumnPropertyIndex].vValue.boolVal = VARIANT_FALSE;
                        ++dwColumnPropertyIndex;
                    }
                }
            }

            hr = pTableDefinition->CreateTable(NULL, &tableID, pdsSchema->rgTables[dwTable].cColumns, rgColumnDescriptions, IID_IRowset, _countof(rgdbpRowSetPropset), rgdbpRowSetPropset, NULL, reinterpret_cast<IUnknown **>(&pdsSchema->rgTables[dwTable].pIRowset));
            ExitOnFailure1(hr, "Failed to create table: %ls", pdsSchema->rgTables[dwTable].sczName);

            for (DWORD i = 0; i < pdsSchema->rgTables[dwTable].cColumns; ++i)
            {
                if (NULL != rgColumnDescriptions[i].rgPropertySets)
                {
                    ReleaseMem(rgColumnDescriptions[i].rgPropertySets[0].rgProperties);
                    ReleaseMem(rgColumnDescriptions[i].rgPropertySets);
                }
            }

            ReleaseNullMem(rgColumnDescriptions);
        }
        else
        {
            ExitOnFailure(hr, "Failed to open table while ensuring schema");
        }

        if (0 < pdsSchema->rgTables[dwTable].cIndexes)
        {
            // Can't create an index while the table is open
            ReleaseNullObject(pdsSchema->rgTables[dwTable].pIRowset);

            // Now create indexes for the table
            for (DWORD dwIndex = 0; dwIndex < pdsSchema->rgTables[dwTable].cIndexes; ++dwIndex)
            {
                indexID.eKind = DBKIND_NAME;
                indexID.uName.pwszName = pdsSchema->rgTables[dwTable].rgIndexes[dwIndex].sczName;

                rgIndexColumnDescriptions = reinterpret_cast<DBINDEXCOLUMNDESC *>(MemAlloc(sizeof(DBINDEXCOLUMNDESC) * pdsSchema->rgTables[dwTable].rgIndexes[dwIndex].cColumns, TRUE));
                ExitOnNull(rgIndexColumnDescriptions, hr, E_OUTOFMEMORY, "Failed to allocate structure to hold index column descriptions");

                for (DWORD dwColumnIndex = 0; dwColumnIndex < pdsSchema->rgTables[dwTable].rgIndexes[dwIndex].cColumns; ++dwColumnIndex)
                {
                    rgIndexColumnDescriptions[dwColumnIndex].pColumnID = reinterpret_cast<DBID *>(MemAlloc(sizeof(DBID), TRUE));
                    rgIndexColumnDescriptions[dwColumnIndex].pColumnID->eKind = DBKIND_NAME;
                    rgIndexColumnDescriptions[dwColumnIndex].pColumnID->uName.pwszName = const_cast<LPOLESTR>(pdsSchema->rgTables[dwTable].rgColumns[pdsSchema->rgTables[dwTable].rgIndexes[dwIndex].rgColumns[dwColumnIndex]].sczName);
                    rgIndexColumnDescriptions[dwColumnIndex].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
                }

                hr = pIndexDefinition->CreateIndex(&tableID, &indexID, static_cast<DBORDINAL>(pdsSchema->rgTables[dwTable].rgIndexes[dwIndex].cColumns), rgIndexColumnDescriptions, 1, rgdbpIndexPropset, NULL);
                if (DB_E_DUPLICATEINDEXID == hr)
                {
                    // If the index already exists, no worries
                    hr = S_OK;
                }
                ExitOnFailure2(hr, "Failed to create index named %ls into table named %ls", pdsSchema->rgTables[dwTable].rgIndexes[dwIndex].sczName, pwzTableName);

                ReleaseNullMem(rgIndexColumnDescriptions);
            }

            hr = pDatabaseInternal->pIOpenRowset->OpenRowset(NULL, &tableID, NULL, IID_IRowset, _countof(rgdbpRowSetPropset), rgdbpRowSetPropset, reinterpret_cast<IUnknown **>(&pdsSchema->rgTables[dwTable].pIRowset));
            ExitOnFailure(hr, "Failed to re-open table after ensuring all indexes are created");
        }

        hr = pdsSchema->rgTables[dwTable].pIRowset->QueryInterface(IID_IRowsetChange, reinterpret_cast<void **>(&pdsSchema->rgTables[dwTable].pIRowsetChange));
        ExitOnFailure1(hr, "Failed to get IRowsetChange object for table: %ls", pdsSchema->rgTables[dwTable].sczName);
    }

    hr = SceCommitTransaction(pDatabase);
    ExitOnFailure(hr, "Failed to commit transaction for ensuring schema");
    fInTransaction = FALSE;


LExit:
    ReleaseObject(pTableDefinition);
    ReleaseObject(pIndexDefinition);

    if (fInTransaction)
    {
        SceRollbackTransaction(pDatabase);
    }

    ReleaseMem(rgIndexColumnDescriptions);
    ReleaseMem(rgColumnDescriptions);

    return hr;
}

static HRESULT SetColumnValue(
    __in SCE_TABLE_SCHEMA *pTableSchema,
    __in DWORD dwColumnIndex,
    __in const BYTE *pbData,
    __in SIZE_T cbSize,
    __inout DBBINDING *pBinding,
    __inout SIZE_T *cbOffset,
    __inout BYTE **ppbBuffer
    )
{
    HRESULT hr = S_OK;
    SIZE_T cbNewOffset = *cbOffset;

    pBinding->iOrdinal = dwColumnIndex + 1; // Skip bookmark column
    pBinding->dwMemOwner = DBMEMOWNER_CLIENTOWNED;
    pBinding->dwPart = DBPART_VALUE | DBPART_LENGTH;
    pBinding->obLength = cbNewOffset;
    cbNewOffset += sizeof(DBBYTEOFFSET);
    pBinding->obValue = cbNewOffset;
    cbNewOffset += cbSize;
    pBinding->wType = pTableSchema->rgColumns[dwColumnIndex].dbtColumnType;
    pBinding->cbMaxLen = 4 * 1024 * 1024;

    if (NULL == *ppbBuffer)
    {
        *ppbBuffer = reinterpret_cast<BYTE *>(MemAlloc(cbNewOffset, TRUE));
        ExitOnNull(*ppbBuffer, hr, E_OUTOFMEMORY, "Failed to allocate buffer while setting row string");
    }
    else
    {
        *ppbBuffer = reinterpret_cast<BYTE *>(MemReAlloc(*ppbBuffer, cbNewOffset, TRUE));
        ExitOnNull(*ppbBuffer, hr, E_OUTOFMEMORY, "Failed to reallocate buffer while setting row string");
    }

    *(reinterpret_cast<DBBYTEOFFSET *>(*ppbBuffer + *cbOffset)) = cbSize;
    *cbOffset += sizeof(DBBYTEOFFSET);
    memcpy(*ppbBuffer + *cbOffset, pbData, cbSize);
    *cbOffset += cbSize;

LExit:
    return hr;
}

static HRESULT GetColumnValue(
    __in SCE_ROW *pRow,
    __in DWORD dwColumnIndex,
    __out_opt BYTE **ppbData,
    __out SIZE_T *cbSize
    )
{
    HRESULT hr = S_OK;
    SCE_TABLE_SCHEMA *pTable = pRow->pTableSchema;
    IAccessor *pIAccessor = NULL;
    HACCESSOR hAccessorLength = DB_NULL_HACCESSOR;
    HACCESSOR hAccessorValue = DB_NULL_HACCESSOR;
    DWORD dwDataSize = 0;
    void *pvRawData = NULL;
    DBBINDING dbBinding = { };
    DBBINDSTATUS dbBindStatus = DBBINDSTATUS_OK;

    dbBinding.iOrdinal = dwColumnIndex + 1;
    dbBinding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
    dbBinding.dwPart = DBPART_LENGTH;
    dbBinding.wType = pTable->rgColumns[dwColumnIndex].dbtColumnType;
    dbBinding.cbMaxLen = 4;

    pRow->pIRowset->QueryInterface(IID_IAccessor, reinterpret_cast<void **>(&pIAccessor));
    ExitOnFailure(hr, "Failed to get IAccessor interface");

    hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &dbBinding, 0, &hAccessorLength, &dbBindStatus);
    ExitOnFailure(hr, "Failed to create accessor");

    hr = pRow->pIRowset->GetData(pRow->hRow, hAccessorLength, reinterpret_cast<void *>(&dwDataSize));
    ExitOnFailure(hr, "Failed to get size of data");

    // For variable-length columns, zero data returned means NULL
    if (0 == dwDataSize)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    if (NULL != ppbData)
    {
        dbBinding.dwPart = DBPART_VALUE;
        dbBinding.cbMaxLen = dwDataSize;

        hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &dbBinding, 0, &hAccessorValue, &dbBindStatus);
        ExitOnFailure(hr, "Failed to create accessor");

        if (DBBINDSTATUS_OK != dbBindStatus)
        {
            hr = E_INVALIDARG;
            ExitOnFailure(hr, "Bad bind status while creating accessor to get value");
        }

        if (DBTYPE_WSTR == dbBinding.wType)
        {
            hr = StrAlloc(reinterpret_cast<LPWSTR *>(&pvRawData), dwDataSize);
            ExitOnFailure1(hr, "Failed to allocate space for string data while reading column %u", dwColumnIndex);
        }
        else
        {
            pvRawData = MemAlloc(dwDataSize, TRUE);
            ExitOnNull1(pvRawData, hr, E_OUTOFMEMORY, "Failed to allocate space for data while reading column %u", dwColumnIndex);
        }

        hr = pRow->pIRowset->GetData(pRow->hRow, hAccessorValue, pvRawData);
        ExitOnFailure(hr, "Failed to read data value");

        *ppbData = reinterpret_cast<BYTE *>(pvRawData);
        pvRawData = NULL;
    }

    *cbSize = dwDataSize;

LExit:
    ReleaseMem(pvRawData);

    if (DB_NULL_HACCESSOR != hAccessorLength)
    {
        pIAccessor->ReleaseAccessor(hAccessorLength, NULL);
    }
    if (DB_NULL_HACCESSOR != hAccessorValue)
    {
        pIAccessor->ReleaseAccessor(hAccessorValue, NULL);
    }
    ReleaseObject(pIAccessor);

    return hr;
}

static HRESULT GetColumnValueFixed(
    __in SCE_ROW *pRow,
    __in DWORD dwColumnIndex,
    __in DWORD cbSize,
    __out BYTE *pbData
    )
{
    HRESULT hr = S_OK;
    SCE_TABLE_SCHEMA *pTable = pRow->pTableSchema;
    IAccessor *pIAccessor = NULL;
    HACCESSOR hAccessorLength = DB_NULL_HACCESSOR;
    HACCESSOR hAccessorValue = DB_NULL_HACCESSOR;
    DWORD dwDataSize = 0;
    DBBINDSTATUS dbBindStatus = DBBINDSTATUS_OK;
    DBBINDING dbBinding = { };

    dbBinding.iOrdinal = dwColumnIndex + 1;
    dbBinding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
    dbBinding.dwPart = DBPART_LENGTH;
    dbBinding.wType = pTable->rgColumns[dwColumnIndex].dbtColumnType;

    pRow->pIRowset->QueryInterface(IID_IAccessor, reinterpret_cast<void **>(&pIAccessor));
    ExitOnFailure(hr, "Failed to get IAccessor interface");

    hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &dbBinding, 0, &hAccessorLength, &dbBindStatus);
    ExitOnFailure(hr, "Failed to create accessor");

    if (DBBINDSTATUS_OK != dbBindStatus)
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, "Bad bind status while creating accessor to get length of value");
    }

    hr = pRow->pIRowset->GetData(pRow->hRow, hAccessorLength, reinterpret_cast<void *>(&dwDataSize));
    ExitOnFailure(hr, "Failed to get size of data");

    if (0 == dwDataSize)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    dbBinding.dwPart = DBPART_VALUE;
    dbBinding.cbMaxLen = cbSize;

    hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &dbBinding, 0, &hAccessorValue, &dbBindStatus);
    ExitOnFailure(hr, "Failed to create accessor");

    if (DBBINDSTATUS_OK != dbBindStatus)
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, "Bad bind status while creating accessor to get value");
    }

    hr = pRow->pIRowset->GetData(pRow->hRow, hAccessorValue, reinterpret_cast<void *>(pbData));
    ExitOnFailure(hr, "Failed to read data value");

LExit:
    if (DB_NULL_HACCESSOR != hAccessorLength)
    {
        pIAccessor->ReleaseAccessor(hAccessorLength, NULL);
    }
    if (DB_NULL_HACCESSOR != hAccessorValue)
    {
        pIAccessor->ReleaseAccessor(hAccessorValue, NULL);
    }
    ReleaseObject(pIAccessor);

    return hr;
}

static HRESULT SetSessionProperties(
    __in ISessionProperties *pISessionProperties
    )
{
    HRESULT hr = S_OK;
    DBPROP rgdbpDataSourceProp[1];
    DBPROPSET rgdbpDataSourcePropSet[1];

    rgdbpDataSourceProp[0].dwPropertyID = DBPROP_SSCE_TRANSACTION_COMMIT_MODE;
    rgdbpDataSourceProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
    rgdbpDataSourceProp[0].vValue.vt = VT_I4;
    rgdbpDataSourceProp[0].vValue.lVal = DBPROPVAL_SSCE_TCM_FLUSH; 
        
    rgdbpDataSourcePropSet[0].guidPropertySet = DBPROPSET_SSCE_SESSION;
    rgdbpDataSourcePropSet[0].rgProperties = rgdbpDataSourceProp;
    rgdbpDataSourcePropSet[0].cProperties = 1;

    hr = pISessionProperties->SetProperties(1, rgdbpDataSourcePropSet);
    ExitOnFailure(hr, "Failed to set session properties");

LExit:
    return hr;
}

static void ReleaseDatabase(
    SCE_DATABASE *pDatabase
    )
{
    if (NULL != pDatabase && NULL != pDatabase->sdbHandle)
    {
        ReleaseDatabaseInternal(reinterpret_cast<SCE_DATABASE_INTERNAL *>(pDatabase->sdbHandle));
    }
    ReleaseMem(pDatabase);
}

static void ReleaseDatabaseInternal(
    SCE_DATABASE_INTERNAL *pDatabaseInternal
    )
{
    HRESULT hr = S_OK;

    if (NULL != pDatabaseInternal)
    {
        ReleaseObject(pDatabaseInternal->pITransactionLocal);
        ReleaseObject(pDatabaseInternal->pIOpenRowset);
        ReleaseObject(pDatabaseInternal->pISessionProperties);
        ReleaseObject(pDatabaseInternal->pIDBCreateSession);
        ReleaseObject(pDatabaseInternal->pIDBProperties);

        if (NULL != pDatabaseInternal->pIDBInitialize)
        {
            hr = pDatabaseInternal->pIDBInitialize->Uninitialize();
            if (FAILED(hr))
            {
                TraceError(hr, "Failed to call uninitialize on IDBInitialize");
            }
            ReleaseObject(pDatabaseInternal->pIDBInitialize);
        }
    }
    ReleaseMem(pDatabaseInternal);
}
