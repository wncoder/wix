//-------------------------------------------------------------------------------------------------
// <copyright file="ILogger.h" company="Microsoft">
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

namespace IronMan
{

class ILogger
{
public:
    enum LoggingLevel
    {
        Error,
        Warning,
        Information,	// Informative messages about major oprations.
        UX,				// Watson/UX/usability-related
        Verbose,
        Debug,			// ? stuff from asserts, perhaps
        InternalUseOnly,	// double-secret: do not use
        Result,
        FinalResult
    };
    virtual ~ILogger() {}
    virtual void Log(LoggingLevel, LPCWSTR szLogMessage) = 0;

    virtual void PushSection(LPCWSTR strAction, LPCWSTR strDescription) = 0;
    virtual void PopSection(LPCWSTR str) = 0;
    
    virtual CPath GetFilePath() = 0;
    virtual bool RenameLog(const CPath & pthNewName) = 0;
    
    //Re-open the file handle and append to the end. 
    virtual void OpenForAppend() = 0;

    //Close the file handle.  This is for main for Watson upload. 
    virtual void Close() = 0;

    // Used to log data in special format
    virtual void BeginLogAsIs(LoggingLevel lvl, CString szLogStart) = 0; 
    virtual void LogLine(LPCWSTR szLine) = 0;
    virtual void LogStartList() = 0;
    virtual void LogListItem(LPCWSTR item) = 0;
    virtual void LogEndList() = 0;
    virtual void EndLogAsIs() = 0;

    // fork and merge methods
    virtual ILogger* Fork() = 0;
    virtual void Merge(ILogger*) = 0;
    virtual void DeleteFork(ILogger*) = 0;
    virtual CPath GetForkedName() = 0;
    virtual void LogFinalResult(LPCWSTR szLogMessage)  {}  //Make it an abstract method so that we don't have to declare it everywhere.
};

class NullLogger : private ILogger
{
    virtual void Log(LoggingLevel, LPCWSTR szLogMessage) {}
    
    virtual void PushSection(LPCWSTR strAction, LPCWSTR strDescription) {}
    virtual void PopSection(LPCWSTR str) {}

    virtual CPath GetFilePath() {return CPath();}

    virtual bool RenameLog(const CPath & pthNewName) {return true;}

    //Re-open the file handle and append to the end.  
    virtual void OpenForAppend() {}

    //Close the file handle.  This is for main for Watson upload. 
    virtual void Close() {}

    // LogAsIs
    virtual void BeginLogAsIs(LoggingLevel lvl, CString szLogStart) {}
    virtual void LogLine(LPCWSTR szLine) {}
    virtual void LogStartList() {}
    virtual void LogListItem(LPCWSTR item) {}
    virtual void LogEndList() {}
    virtual void EndLogAsIs() {}

    // fork and merge
    virtual ILogger* Fork() { return &GetNullLogger(); }
    virtual void Merge(ILogger*) {}
    virtual void DeleteFork(ILogger*) {}
    virtual CPath GetForkedName() { return GetFilePath(); }
    virtual void LogFinalResult(LPCWSTR szLogMessage)  {}  //Make it an abstract method so that we don't have to declare it everywhere.

public:
    static ILogger& GetNullLogger()
    {
        static NullLogger nl;
        return nl;
    }
};

}
