//-------------------------------------------------------------------------------------------------
// <copyright file="WatsonDefinition.h" company="Microsoft">
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

#include "common\operation.h"
#include "ModuleUtils.h"
#include "common\SystemUtil.h"
#include "ux\uxenum.h"

#define DW_APPNAME_LENGTH   56

namespace IronMan
{

class PathCollection
{
    struct CPathEqual
    {
        static bool IsEqual(const CPath& path1, const CPath& path2)
        {
            return CString(path1) == CString(path2);
        }
    };
    CSimpleArray<CPath, CPathEqual> m_array;

public:
    PathCollection() {}
    virtual ~PathCollection() {}

    //Ensure that the condition is true before adding to the array
    // - It is not an empty string.
    // - It does not already exist in the arry.
    void Add(const CPath& path)
    {
        if (!CString(path).IsEmpty() && m_array.Find(path) == -1)
            m_array.Add(path);
    }
    void Remove(const CPath& path)
    {
        m_array.Remove(path);
    }

    CString ConvertToWatsonString() const
    {
        if (m_array.GetSize() == 0)
            return L"";

        CString cs(m_array[0]);
        for(int i=1; i<m_array.GetSize(); ++i)
        {
            cs += L"|";
            cs += m_array[i];
        }
        return cs;
    }

    int Find(const CPath& path) const
    {
        return m_array.Find(path);
    }
};

// Control structure for our LaunchWatson call.
struct WatsonParams
{
    WatsonParams()
    {}

    bool fQueueReport;                              //Queue the report to admin queue

    LCID lcidUI;                                    // will try this UI langauge if non-zero
                                                    // next DW will use the system LCID, 
                                                    // and if it can't find an intl dll for that, 
                                                    // will fall back on US English (1033)	

    WCHAR szLocGeneral_AppName[DW_APPNAME_LENGTH];  // The name of the application.  
                                                    // THIS ITEM IS REQUIRED. 
                                                    // We do not go find your executable name to use instead.

    WCHAR szGeneral_AppName[DW_APPNAME_LENGTH];     // The ENU name of the application.  
                                                    // THIS ITEM IS REQUIRED. 
                                                    // We do not go find your executable name to use instead.
                                                    // This value is used to write to the eventlog so it must match what is register 
                                                    // with the event source.  Otherwise, you will have malformed events in event viewer.
                                                    // I am expecting this to match the ProductName MSI property minus the (language) suffix.

    WCHAR szPackageVersion[DW_APPNAME_LENGTH];      // The package version of the package being installed
                                                    // THIS ITEM IS REQUIRED. 

    WCHAR szFilesToDelete[1024];                    // File list, seperated by DW_FILESEP each of these files gets added to the
                                                    // cab at upload time.  These are files that we will delete if fDwrDeleteFiles is sent.

    PathCollection pcFilesToKeep;                   // File list, seperated by DW_FILESEP each of these files gets added to the
                                                    // cab at upload time.  These are files that we will NOT delete if fDwrDeleteFiles is sent.

    WCHAR szPackageId[16];                          //For Microsoft, this is the KB number
    Operation::euiOperation operation;              //Install | Uninstall | Demo
    WCHAR szArchitecture[8];                        //X86 | A64 | I64
    WCHAR szOperatingSystem[32];                    //XPSP2 | W2K3SP2 | Vista
    WCHAR szItemName[255];                          //Item name
    UxEnum::phaseEnum currentPhase;                 //I for Install | D for Download
    int iMsiErrorMessage;                           //The Error message from External UI Handler
    UxEnum::actionEnum currentAction;
    UxEnum::uiModeEnum uiMode;
    CString csWatsonDialogText;                             //Set the text that appears on the WER dialogbox.
    CString csCurrentItemStep;                      //The extra information that provide more information of the failure.
};

//Singleton class
class WatsonData
{
protected:
    WatsonParams m_WatsonParams;
    ILogger* m_pILogger;

public:
    static WatsonData*& WatsonDataStatic()
    {
        static WatsonData* s_WatsonData = NULL;
        return s_WatsonData;
    }

    WatsonData()
        : m_pILogger(NULL)
    {
        m_WatsonParams.iMsiErrorMessage = 0;
        m_WatsonParams.szFilesToDelete[0] = L'\0';
        InitalizeIronMan();
    }

    void InitalizeIronMan()
    {
        //Initialize m_watsonParams
        //Queue by default because we have not parse the command line switches yet.
        SetQueueMode(true);

        SetLcidUi(1033);

        //The default is no error;
        SetInternalErrorState(0, L"");

        SetLocApplicationName(ModuleUtils::GetProgramName());
        SetApplicationName(ModuleUtils::GetProgramName());
        SetPackageVersion(L"Unknown");
        SetOperatingSystem(CSystemUtil::GetOSAbbr());
        SetArchitecture(CSystemUtil::GetCPUArchitecture());
        SetOperation(IronMan::Operation::uioUninitalized);
        SetCurrentItemName(L" ");  //Cannot be a NULL string because Watson not take it as a parameter value.
        SetCurrentPhase(UxEnum::pUI); 
        SetCurrentAction(UxEnum::aNone);
    }

    //------------------------------------------------------------------------------
    // Logger
    //
    // For Watson to upload a file, the file must be closed.  
    // We need to cache the logger so that we can lose the log file.
    //------------------------------------------------------------------------------
    void SetLogger(ILogger& logger)
    {        
         m_pILogger = &logger;
    }

    ILogger* GetLogger()
    {   
       return m_pILogger;
    }    

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    // SetDisplayUIMode
    //
    // The accessor for setting the Queue mode for struct m_WatsonParams
    //------------------------------------------------------------------------------
    void SetQueueMode(bool fQueue)
    {
        m_WatsonParams.fQueueReport = fQueue;
    }

    void SetUiMode(bool bPassive, bool bSilent)
    {
        m_WatsonParams.uiMode = UxEnum::smInteractive;

        if (bPassive)
            m_WatsonParams.uiMode = UxEnum::smPassive;

        if (bSilent)
            m_WatsonParams.uiMode = UxEnum::smSilent;
    }

    const UxEnum::uiModeEnum GetUiMode() const
    {
        return m_WatsonParams.uiMode;
    }

    const CString GetUiModeString() const
    {
        return UxEnum::GetUiModeString(m_WatsonParams.uiMode);
    }

    //------------------------------------------------------------------------------
    // IsQueue
    //
    // The accessor for getting the Queue status from struct m_WatsonParams.
    //------------------------------------------------------------------------------
    const bool IsQueue() const
    {
        return m_WatsonParams.fQueueReport;
    }

    //------------------------------------------------------------------------------
    // SetLcid
    //
    // The accessor for setting the LCID from struct m_WatsonParams.
    //------------------------------------------------------------------------------
    LCID SetLcidUi(LCID lcid)
    {
        return m_WatsonParams.lcidUI = lcid;
    }

    //------------------------------------------------------------------------------
    // GetLcidUi
    //
    // The accessor for getting the LCID from struct m_WatsonParams.
    //------------------------------------------------------------------------------
    const LCID GetLcidUi() const
    {
        return m_WatsonParams.lcidUI;
    }

    //------------------------------------------------------------------------------
    // SetLocApplicationName
    //
    // The accessor for setting szLocGeneral_AppName for struct m_WatsonParams.
    //------------------------------------------------------------------------------
    void SetLocApplicationName(LPCWSTR lpName)
    {
        m_WatsonParams.szLocGeneral_AppName[DW_APPNAME_LENGTH - 1] = 0;
        StringCchCopyW( m_WatsonParams.szLocGeneral_AppName, 
                        _countof(m_WatsonParams.szLocGeneral_AppName),
                        lpName);
    }

    //------------------------------------------------------------------------------
    // GetLocGeneral_AppName
    //
    // The accessor for getting the szLocGeneral_AppName from struct m_WatsonParams.
    //------------------------------------------------------------------------------
    LPCWSTR GetLocGeneralAppName() const
    {
        return m_WatsonParams.szLocGeneral_AppName;
    }

    //------------------------------------------------------------------------------
    // SetApplicationName
    //
    // The accessor for setting szGeneral_AppName for struct m_WatsonParams.
    //------------------------------------------------------------------------------
    void SetApplicationName(LPCWSTR lpName)
    {
        m_WatsonParams.szGeneral_AppName[DW_APPNAME_LENGTH - 1] = 0;
        StringCchCopyW(	m_WatsonParams.szGeneral_AppName, 
                        _countof(m_WatsonParams.szGeneral_AppName),
                        lpName);
    }

    //------------------------------------------------------------------------------
    // GetGeneral_AppName
    //
    // The accessor for getting the szGeneral_AppName from struct m_WatsonParams.
    //------------------------------------------------------------------------------
    LPCWSTR GetGeneralAppName() const
    {
        return m_WatsonParams.szGeneral_AppName;
    }

    //------------------------------------------------------------------------------
    // SetPackageVersion
    //
    // The accessor for setting szPackageVersion for struct m_WatsonParams.
    //------------------------------------------------------------------------------
    void SetPackageVersion(LPCWSTR szPackageVersion)
    {
        StringCchCopyW( m_WatsonParams.szPackageVersion, 
                        _countof(m_WatsonParams.szPackageVersion),
                        szPackageVersion);

        m_WatsonParams.szPackageVersion[DW_APPNAME_LENGTH - 1] = 0;
    }
    
    //------------------------------------------------------------------------------
    // GetPackageVersion
    //
    // The accessor for getting szPackageVersion from struct m_WatsonParams.
    //------------------------------------------------------------------------------
    LPCWSTR GetPackageVersion() const
    {
        return m_WatsonParams.szPackageVersion;
    }

    void AddFilesToDelete(LPCWSTR pszFileNames)
    {
        m_WatsonParams.szFilesToDelete[1024 - 1] = 0;
        StringCchCopyW(	m_WatsonParams.szFilesToDelete, 
                        _countof(m_WatsonParams.szFilesToDelete),
                        pszFileNames);
    }

    LPCWSTR GetFilesToDelete() const
    {
        return m_WatsonParams.szFilesToDelete;
    }

    //------------------------------------------------------------------------------
    // AddFileToKeep
    //
    // Add the file to keep after Watson upload.
    // Ensure that it is unique.
    //------------------------------------------------------------------------------
    void AddFileToKeep(LPCWSTR szFilePath)
    {
        if (-1 == m_WatsonParams.pcFilesToKeep.Find(szFilePath))
        {
            m_WatsonParams.pcFilesToKeep.Add(szFilePath);
        }
    }

    //------------------------------------------------------------------------------
    // GetFilesToKeep
    //
    // The accessor for getting the szFilesToKeep from struct m_WatsonParams.
    //------------------------------------------------------------------------------
    PathCollection& GetFilesToKeep() 
    {
        return m_WatsonParams.pcFilesToKeep;
    }

    void SetPackageId(LPCWSTR location)
    {
        m_WatsonParams.szPackageId[16 - 1] = 0;
        StringCchCopyW(	m_WatsonParams.szPackageId, 
                        _countof(m_WatsonParams.szPackageId),
                        location);
    }

    LPCWSTR GetPackageId() const
    {
        return m_WatsonParams.szPackageId;
    }

    void SetOperation(Operation::euiOperation operation)
    {
        m_WatsonParams.operation = operation;
    }

    const Operation::euiOperation GetOperation() const
    {
        return m_WatsonParams.operation;
    }

    void SetArchitecture(LPCWSTR architecture)
    {
        m_WatsonParams.szArchitecture[8 - 1] = 0;
        StringCchCopyW(	m_WatsonParams.szArchitecture, 
                        _countof(m_WatsonParams.szArchitecture),
                        architecture);
    }

    LPCWSTR GetArchitecture() const
    {
        return m_WatsonParams.szArchitecture;
    }

    void SetOperatingSystem(LPCWSTR operatingSystem)
    {
        size_t cchDest = _countof(m_WatsonParams.szOperatingSystem);
        m_WatsonParams.szOperatingSystem[cchDest - 1] = 0;
        StringCchCopyW(	m_WatsonParams.szOperatingSystem, 
                        cchDest,
                        operatingSystem);
    }

    LPCWSTR GetOperatingSystem() const
    {
        return m_WatsonParams.szOperatingSystem;
    }

    //------------------------------------------------------------------------------
    // SetInternalError
    //
    // There is a need to store the last result as it represent the current state
    // of the application at error/crash time.
    // It is intentional that we want to capture the last set error.  This is to handle 
    // the OnFailureBehaviour and Error Retry scenarios.
    //------------------------------------------------------------------------------
    void SetInternalError(int internalMsiError)
    {        
        m_WatsonParams.iMsiErrorMessage = internalMsiError;
    }

    //------------------------------------------------------------------------------
    // SetCurrentStep
    //
    // The current step in an item execution.  For MSI, thisis the action name.
    //------------------------------------------------------------------------------
    void SetCurrentStep(const CString csCurrentItemStep)
    {
        m_WatsonParams.csCurrentItemStep = csCurrentItemStep;
    }

    //------------------------------------------------------------------------------
    // SetError
    //
    // Set both the Internal Error and the CurrentItemStep to ensure that are in sync.
    //------------------------------------------------------------------------------
    void SetInternalErrorState(int internalMsiError, const CString& csCurrentItemStep)
    {
        SetInternalError(internalMsiError);
        SetCurrentStep(csCurrentItemStep);
    }

    const CString GetInternalErrorString() const
    {
        CString strMsiErrorMessage;
        strMsiErrorMessage.Format(L"%u", m_WatsonParams.iMsiErrorMessage);
        return strMsiErrorMessage;
    }

    int GetInternalError() const
    {
        return m_WatsonParams.iMsiErrorMessage;
    }

    void SetCurrentItemName(LPCWSTR pszItemName)
    {        
        StringCchCopyW( m_WatsonParams.szItemName, 
                        _countof(m_WatsonParams.szItemName),
                        pszItemName);
        m_WatsonParams.szItemName[_countof(m_WatsonParams.szItemName) - 1] = 0;
    }

    LPCWSTR GetCurrentItemName() const
    {
        return m_WatsonParams.szItemName;
    }

    //------------------------------------------------------------------------------
    // GetCurrentStep
    //
    // The getter for Current Item step.
    //------------------------------------------------------------------------------
    const CString GetCurrentStep() const
    {
        if (L"" == m_WatsonParams.csCurrentItemStep)
        {
            return L" ";  //Returning as Watson will ignore "" as a value for parameter.
        }
        return m_WatsonParams.csCurrentItemStep;
    }

    void SetCurrentPhase(UxEnum::phaseEnum phase)
    {        
        m_WatsonParams.currentPhase = phase;
        SetCurrentItemName(L" ");  //Reset the item since we are changing phase.
    }

    const UxEnum::phaseEnum GetCurrentPhase() const
    {
        return m_WatsonParams.currentPhase;
    }

    const CString GetCurrentPhaseString() const
    {
        return UxEnum::GetPhaseString(m_WatsonParams.currentPhase);
    }

    void SetCurrentAction(UxEnum::actionEnum action)
    {
        m_WatsonParams.currentAction = action;
    }

    const CString GetCurrentActionString() const
    {
        return UxEnum::GetActionString(m_WatsonParams.currentAction);
    }

    const UxEnum::actionEnum GetCurrentAction() const
    {
        return m_WatsonParams.currentAction;
    }

    //This is the getter/setter for the WER header text 
    //Overriding the original "Application has stopped working" 
    //message because it is misleading.
    const void SetWatsonHeader(const CString csWatsonText)
    {
        m_WatsonParams.csWatsonDialogText = csWatsonText;
    }

    const CString GetWatsonHeader() const
    {
        return m_WatsonParams.csWatsonDialogText;
    }
};
}
