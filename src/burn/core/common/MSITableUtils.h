//-------------------------------------------------------------------------------------------------
// <copyright file="MSITableUtils.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IProgressObserver.h"
#include "interfaces\ILogger.h"

namespace IronMan
{
//------------------------------------------------------------------------------
// MsiTableUtils
//
// Class for accessing the values of tables in an MSI or MSP
//-------------------------------------------------------------------------------
class MsiTableUtils
{
public:
    enum DatabaseFileType
    {
        MsiFile = 0,
        MspFile = 1,
    };

private:
    MSIHANDLE m_hDatabase;
    const CPath m_databasePath;
    const MsiTableUtils::DatabaseFileType m_databaseFileType;
    ILogger& m_logger;

public:
    //------------------------------------------------------------------------------
    // Database Path Constructor
    //------------------------------------------------------------------------------
    MsiTableUtils(const CString& databasePath
                , MsiTableUtils::DatabaseFileType databaseFileType
                , ILogger& logger)
        : m_hDatabase(0)
        , m_databasePath(databasePath)
        , m_databaseFileType(databaseFileType)
        , m_logger(logger)
    {}

    //------------------------------------------------------------------------------
    // Copy Constructor
    //------------------------------------------------------------------------------
    MsiTableUtils(const MsiTableUtils& rhs)
        : m_hDatabase(0)
        , m_databasePath(rhs.m_databasePath)
        , m_databaseFileType(rhs.m_databaseFileType)
        , m_logger(rhs.m_logger)
    {}

    //------------------------------------------------------------------------------
    // static functiont to create class
    //------------------------------------------------------------------------------
    static MsiTableUtils CreateMsiTableUtils(const CString& databasePath
                                            , MsiTableUtils::DatabaseFileType databaseFileType
                                            , ILogger& logger)
    {
        return MsiTableUtils(databasePath, databaseFileType, logger);
    }

    //------------------------------------------------------------------------------
    // Destructor
    //------------------------------------------------------------------------------
    ~MsiTableUtils()
    {
        if ( 0 != m_hDatabase )
        {
            MsiCloseHandle(m_hDatabase);
        }
    }

    //------------------------------------------------------------------------------
    // Uses a SQL query to get a value from the Database
    //------------------------------------------------------------------------------
    UINT ExecuteScalar( const CString& columnNameOfValueToGet
        , const CString& tableName
        , const CString& whereClause
        , CString& value)
    {
        UINT err = ERROR_SUCCESS;
        // If the database has not been open yet, open it now
        if (0 == m_hDatabase)
        {
            err = OpenDatabase();
        }
        if ( ERROR_SUCCESS == err )
        {
            PMSIHANDLE hView;

            CString queryString( L"SELECT `" + columnNameOfValueToGet + L"` "
                L"FROM `" + tableName + L"` "
                L"WHERE " + whereClause );

            err = MsiDatabaseOpenView(m_hDatabase, queryString, &hView);
            if ( ERROR_SUCCESS == err )
            {
                err = MsiViewExecute(hView, NULL);
                if ( ERROR_SUCCESS == err )
                {
                    PMSIHANDLE hRecord;
                    err = MsiViewFetch(hView, &hRecord);
                    if ( ERROR_SUCCESS == err )
                    {
                        DWORD dwfieldSize = 0;
                        err = MsiRecordGetString(hRecord, 1, value.GetBuffer(dwfieldSize), &dwfieldSize);
                        value._ReleaseBuffer();
                        if ( ERROR_MORE_DATA  == err )
                        {
                            dwfieldSize++;
                            err = MsiRecordGetString(hRecord, 1, value.GetBuffer(dwfieldSize), &dwfieldSize);
                            value._ReleaseBuffer();
                        }
                    }
                }
            }
        }
        return err;
    }

private:
    UINT OpenDatabase()
    {
        LPCTSTR szPersist = MSIDBOPEN_READONLY;
        if (MsiTableUtils::MspFile == m_databaseFileType)
        {
            szPersist += MSIDBOPEN_PATCHFILE;
        }
        return MsiOpenDatabase(m_databasePath, szPersist, &m_hDatabase);
    }

private: // hooks for unit tests
    virtual UINT WINAPI MsiOpenDatabase( LPCTSTR      szDatabasePath  // path to database, 0 to create temporary database
                                , LPCTSTR      szPersist       // output database path or one of predefined values
                                , MSIHANDLE*   phDatabase)     // location to return database handle
    {
        return ::MsiOpenDatabase(szDatabasePath, szPersist, phDatabase);
    }

    virtual UINT WINAPI MsiDatabaseOpenView(MSIHANDLE hDatabase
                                                , LPCTSTR     szQuery            // SQL query to be prepared
                                                , MSIHANDLE*  phView)            // returned view if TRUE
    {
        return ::MsiDatabaseOpenView(hDatabase, szQuery, phView);
    }

    virtual UINT WINAPI MsiViewExecute(MSIHANDLE hView
                                        , MSIHANDLE hRecord)             // optional parameter record, or 0 if none
    {
        return ::MsiViewExecute(hView, hRecord);
    }

    virtual UINT WINAPI MsiViewFetch(MSIHANDLE hView
                                    , MSIHANDLE  *phRecord)          // returned data record if fetch succeeds
    {
        return ::MsiViewFetch(hView, phRecord);
    }

    virtual UINT WINAPI MsiRecordGetString(MSIHANDLE hRecord
                                            ,UINT iField
            ,__out_ecount_opt(*pcchValueBuf) LPTSTR  szValueBuf      // buffer for returned value
            ,__inout_opt                     LPDWORD  pcchValueBuf)   // in/out buffer character count
    {
        return ::MsiRecordGetString(hRecord, iField, szValueBuf, pcchValueBuf);
    }

    virtual UINT WINAPI MsiCloseHandle(MSIHANDLE hAny)
    {
        return ::MsiCloseHandle(hAny);
    }
};
}
