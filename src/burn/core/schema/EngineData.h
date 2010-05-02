//-------------------------------------------------------------------------------------------------
// <copyright file="EngineData.h" company="Microsoft">
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

#include "Feature.h"
#include "common\XmlUtils.h"
#include "interfaces\ILogger.h"
#include "Blockers.h"
#include "ConfigurationElement.h"
#include "CustomError.h"
#include "BlockerElement.h"
#include "common\ParamSubstitutter.h"
#include "schema\LocalizedData.h"

namespace IronMan
{

const LPCWSTR c_pszSetupVersion = L"1.0";

//------------------------------------------------------------------------------
// Class: IronMan::ItemBase
// Used as the base for the IronMan item types, and contains information on
// what type of item it is, e.g. an MSI, and MSP, and EXE or a plain File.
//------------------------------------------------------------------------------
class ItemBase
{
public:
    enum ItemType
    {
        Mock, // for mocking only
        Patches,
        Msi,
        Msp,
        Exe,
        File,
        AgileMsi,
        AgileMsp,
        ServiceControl,
        CleanupBlockType,
        RelatedProductsType
    };
    ItemBase(ItemType type, ULONGLONG estimatedInstallTime, bool isPerMachine = false, const CString& id = L"") 
        : m_itemType(type)
        , m_estimatedInstallTime(estimatedInstallTime)
        , m_hFile(NULL) 
        , m_perMachine(isPerMachine)
        , m_id(id)
        , m_bApplicable(false)
        , m_detectState(PACKAGE_STATE_UNKNOWN)
        , m_actionState(ACTION_STATE_NONE)
        , m_requestState(REQUEST_STATE_NONE)
        , m_bActionStateUpdated(false)
    {
        this->m_sczInstallCondition = NULL;
        this->m_sczRepairCondition = NULL;
    }
    ItemBase(ItemType type, CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_itemType(type)
        , m_hFile(NULL) 
        , m_perMachine(ElementUtils::EvaluateToBoolValue(L"PerMachine", false, ElementUtils::GetOptionalAttributeByName(spElement, L"PerMachine", logger), logger))
        , m_id(ElementUtils::GetOptionalAttributeByName(spElement, L"Id", logger, true))
        , m_bApplicable(false)
        , m_detectState(PACKAGE_STATE_UNKNOWN)
        , m_actionState(ACTION_STATE_NONE)
        , m_requestState(REQUEST_STATE_NONE)
        , m_bActionStateUpdated(false)
    {
        HRESULT hr = S_OK;

        this->m_sczInstallCondition = NULL;
        this->m_sczRepairCondition = NULL;

        // @EstimatedInstallTime
        CString itemName = ElementUtils::GetName(spElement);
        if ( L"File" == itemName )
        {
            m_estimatedInstallTime = ElementUtils::GetOptionalAttributeULONGLONGByName(spElement, L"EstimatedInstallTime", logger);
        }
        else
        {
            m_estimatedInstallTime = ElementUtils::GetAttributeULONGLONGByName(spElement, L"EstimatedInstallTime", logger);
        }


        // @InstallCondition
        hr = XmlGetAttributeEx((IXMLDOMElement*)spElement, L"InstallCondition", &this->m_sczInstallCondition);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @InstallCondition.");
        }

        // @RepairCondition
        hr = XmlGetAttributeEx((IXMLDOMElement*)spElement, L"RepairCondition", &this->m_sczRepairCondition);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @RepairCondition.");
        }

        // FileRef/@Id
        AddReferencedFiles(spElement, m_referencedFiles, logger);

    LExit:
        return;
    }
    ItemBase(const ItemBase& rhs)
        : m_itemType(rhs.m_itemType)
        , m_estimatedInstallTime(rhs.m_estimatedInstallTime)
        , m_hFile(rhs.m_hFile) 
        , m_perMachine(rhs.m_perMachine)
        , m_id(rhs.m_id)
        , m_bApplicable(rhs.m_bApplicable)
        , m_detectState(rhs.m_detectState)
        , m_actionState(rhs.m_actionState)
        , m_requestState(rhs.m_requestState)
        , m_bActionStateUpdated(rhs.m_bActionStateUpdated)
    {
        HRESULT hr = S_OK;

        this->m_sczInstallCondition = NULL;
        this->m_sczRepairCondition = NULL;

        if (rhs.m_sczInstallCondition)
        {
            hr = StrAllocString(&this->m_sczInstallCondition, rhs.m_sczInstallCondition, 0);
            ExitOnFailure(hr, "Failed to copy InstallCondition.");
        }

        if (rhs.m_sczRepairCondition)
        {
            hr = StrAllocString(&this->m_sczRepairCondition, rhs.m_sczRepairCondition, 0);
            ExitOnFailure(hr, "Failed to copy RepairCondition.");
        }

        // FileRef/@Id
        for( int i = 0; i < rhs.m_referencedFiles.GetSize(); ++i)
        {
            m_referencedFiles.Add(rhs.m_referencedFiles[i]);
        }

    LExit:
        return;
    }
    ItemBase(ItemType type, CComPtr<IXMLDOMElement> spElement, ULONGLONG estimatedInstallTime, ILogger& logger, bool isPerMachine = false) 
        : m_itemType(type)
        , m_estimatedInstallTime(estimatedInstallTime)
        , m_hFile(NULL) 
        , m_perMachine(isPerMachine)
        , m_id(ElementUtils::GetOptionalAttributeByName(spElement, L"Id", logger, true))
        , m_bApplicable(false)
        , m_detectState(PACKAGE_STATE_UNKNOWN)
        , m_actionState(ACTION_STATE_NONE)
        , m_requestState(REQUEST_STATE_NONE)
        , m_bActionStateUpdated(false)
    {
        this->m_sczInstallCondition = NULL;
        this->m_sczRepairCondition = NULL;

        // FileRef/@Id
        AddReferencedFiles(spElement, m_referencedFiles, logger);
    }
    virtual ~ItemBase()
    {
        ReleaseStr(m_sczInstallCondition);
        ReleaseStr(m_sczRepairCondition);
        CloseHandle();
    }
    ItemType GetType() const 
    {
        return m_itemType;
    }

    ULONGLONG GetEstimatedInstallTime() const
    {
        return m_estimatedInstallTime;
    }

    // Creates a copy of the item, similar to a copy constructor
    virtual ItemBase* Clone() const = 0;
    void SetHandle(HANDLE hFile)
    {
        if (m_hFile)
            ::CloseHandle(m_hFile);
        m_hFile = hFile;
    }

    // Check if need after SetupUI.dll refactoring.
    void SetApplicability(bool bApplicable)
    {
        m_bApplicable = bApplicable;
    }

    void SetDetectState(__in PACKAGE_STATE detectState)
    {
        m_detectState = detectState;
    }

    void SetRequestState(__in REQUEST_STATE requestState)
    {
        m_requestState = requestState;
    }

    void SetActionState(__in ACTION_STATE actionState)
    {
        m_actionState = actionState;
        m_bActionStateUpdated = true;
    }

    void GetPackageState(__out bool* pApplicable, __out PACKAGE_STATE* pDetectState, __out REQUEST_STATE* pRequestState, __out ACTION_STATE* pActionState) const
    {
        if (pApplicable)
        {
            *pApplicable = m_bApplicable;
        }

        if (pDetectState)
        {
            *pDetectState = m_detectState;
        }

        if (pRequestState)
        {
            *pRequestState = m_requestState;
        }

        if (pActionState)
        {
            *pActionState = m_actionState;
        }
    }

    bool ActionStateUpdated() const
    {
        return m_bActionStateUpdated;
    }

    virtual void CloseHandle()
    {
        if (m_hFile)
            ::CloseHandle(m_hFile);
        m_hFile = NULL;
    }

    virtual bool IsPerMachine() const
    {
        return m_perMachine;
    }

    LPCWSTR GetInstallCondition() const
    {
        return this->m_sczInstallCondition;
    }

    LPCWSTR GetRepairCondition() const
    {
        return this->m_sczRepairCondition;
    }

    virtual CString GetId() const 
    {
        return m_id;
    }

    //-------------------------------------------------------------------------
    // GetReferencedFiles
    // Returns a reference to an array of FileRef Ids that this item references
    //-------------------------------------------------------------------------
    const CSimpleArray<CString>& GetReferencedFiles() const
    {
        return m_referencedFiles;
    }

protected:
    void ResetType(ItemType type) 
    { 
        m_itemType = type; 
    }

private:
    static void AddReferencedFiles(CComPtr<IXMLDOMElement> spElement, CSimpleArray<CString>& referencedFiles, ILogger& logger)
    {
        CComPtr<IXMLDOMNode> spChild;

        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            do {
                CComPtr<IXMLDOMNode> spSibling = spChild;
                if ((!ElementUtils::IsNodeComment(spSibling)) && 
                    ( (L"File" == ElementUtils::GetName(spSibling)) || (L"FileRef" == ElementUtils::GetName(spSibling)) ) )
                {
                    // File or FileRef Element, store Id
                    CString csFileRefId = ElementUtils::GetAttributeByName(CComQIPtr<IXMLDOMElement>(spSibling), L"Id", logger);
                    if ( -1 == referencedFiles.Find(csFileRefId) )
                    {
                        referencedFiles.Add(csFileRefId);
                    }
                }
                // Get next Child Element
                spChild = NULL;
                hr = spSibling->get_nextSibling(&spChild);
                if (S_OK != hr)
                {
                    // No more child elements
                    break;
                }
            } while (NULL != spChild);
        }
    }

private:
    ItemType m_itemType;
    HANDLE m_hFile;
    ULONGLONG m_estimatedInstallTime;
    bool m_perMachine;
    LPWSTR m_sczInstallCondition;
    LPWSTR m_sczRepairCondition;
    CString m_id;
    bool m_bApplicable;
    PACKAGE_STATE m_detectState;
    ACTION_STATE m_actionState;
    REQUEST_STATE m_requestState;
    bool m_bActionStateUpdated;
    CSimpleArray<CString> m_referencedFiles;
};



struct ItemArray : public CSimpleArray<ItemBase*>
{
    ItemArray() {}
    ItemArray(const ItemArray& rhs)
    {
        for(int i=0; i<rhs.GetSize(); ++i)
            Add(rhs[i]->Clone());
    }
    ItemArray(const CSimpleArray<ItemBase*>& rhs)
    {
        for(int i=0; i<rhs.GetSize(); ++i)
            Add(rhs[i]->Clone());
    }

    virtual ~ItemArray()
    {
        for (int i=0; i<GetSize(); ++i)
        {
            delete (*this)[i];
        }
    }
};

/*
    This class can be used to represent vairos Failure Actions (Failure Behaviours)
    that are supported. 

    When a failure action is not authored, one can designate one of the three as 
    default failure actions. In this case m_bActionSpecified will be set to faluse 
    as the action was not specified in the authoring but is the default one.

    When current item fails, based on authoring of the failure aciton, the following can happen:
        STOP - CompositePerformer stops the sequence of items that it is performing and returns.
        CONTINUE - CompositePerformer goes on to performing next item in the sequence.
        ROLLBACK - CompsitePerformer starts a Rollback on items "installed" by it, so far.
*/
class FailureActionEnum
{
    enum FailureActionEnum_ {Stop_, Continue_, Rollback_};
    FailureActionEnum_ m_action;
    bool m_bActionSpecified;

    FailureActionEnum(FailureActionEnum_ action) 
        : m_action(action)
    {}

public:
    FailureActionEnum(const FailureActionEnum& rhs)
        : m_action(rhs.m_action)
        , m_bActionSpecified(rhs.m_bActionSpecified)               
    {}

    static const FailureActionEnum& GetStopAction(bool actionSpecified = true)
    {
        static  FailureActionEnum stop(Stop_);
        stop.m_bActionSpecified = actionSpecified;
        return stop;
    }
    static const FailureActionEnum& GetContinueAction(bool actionSpecified = true)
    {
        static FailureActionEnum _continue(Continue_);
        _continue.m_bActionSpecified = actionSpecified;
        return _continue;
    }
    static const FailureActionEnum& GetRollbackAction(bool actionSpecified = true)
    {
        static FailureActionEnum rollback(Rollback_);
        rollback.m_bActionSpecified = actionSpecified;
        return rollback;
    }

    bool IsFailureActionStop(void) const
    {
        return (m_action == Stop_);
    }
    bool IsFailureActionContinue(void) const
    {
        return (m_action == Continue_);
    }
    bool IsFailureActionRollback(void) const
    {
        return (m_action == Rollback_);
    }

    // Returns:
    //   FALSE
    //     The FailureAction described here is the default one, picked by the engine.
    //     This will happen when author doesn't specify any action in the authoring 
    //     (in ParameterInfo.xml).
    //   TRUE
    ///    The action was authored in.
    bool IsFailureActionSpecified(void) const
    {
        return m_bActionSpecified;
    }
private:
    FailureActionEnum(); // Hide constructor
};

//------------------------------------------------------------------------------
// Class: IronMan::DownloadPath
// This is the "remote" Url of the item we need to acquire, e.g.
// http://download.microsoft.com/download/6/1/c/61c3c8f1-f8bf-434c-8897-8093cd7cfcc4/xispatch.msi
//------------------------------------------------------------------------------
class DownloadPath
{
    CString m_url;
    CString m_hashValue; // This is the hash value of the item being downloaded
    ULONGLONG m_totalNumberOfBytes; // This is the download size of the item

    CString m_compressedHashValue; // This is the hash value of the compressed item being downloaded
    ULONGLONG m_totalNumberOfCompressedBytes; // This is the download size of the compressed item
public:
    DownloadPath(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_url(ElementUtils::GetOptionalAttributeByName(spElement, L"URL", logger, true))
        , m_totalNumberOfBytes(ElementUtils::GetOptionalAttributeULONGLONGByName(spElement, L"DownloadSize", logger))
        , m_hashValue(ElementUtils::GetOptionalAttributeByName(spElement, L"Sha1HashValue", logger))
        , m_totalNumberOfCompressedBytes(ElementUtils::GetOptionalAttributeULONGLONGByName(spElement, L"CompressedDownloadSize", logger))
        , m_compressedHashValue(ElementUtils::GetOptionalAttributeByName(spElement, L"CompressedHashValue", logger))
    {
        // If URL is not empty, then the ItemSize must be > 0, otherwise the XML is invalid
        if ( !m_url.IsEmpty() && 0 == m_totalNumberOfBytes )
        {
            CInvalidXmlException ixe(L"schema validation failure:  If URL is present then there must be a DownloadSize");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }

    DownloadPath(const DownloadPath& rhs) 
        : m_url(rhs.m_url)
        , m_totalNumberOfBytes(rhs.m_totalNumberOfBytes)
        , m_hashValue(rhs.m_hashValue) 
        , m_compressedHashValue(rhs.m_compressedHashValue)
        , m_totalNumberOfCompressedBytes(rhs.m_totalNumberOfCompressedBytes)
    {
    }

    DownloadPath(const CString& url, const ULONGLONG itemSize, CString hashValue=L"", const ULONGLONG itemSizeCompressed = 0, CString hashValueCompressed=L"") 
        : m_url(url)
        , m_totalNumberOfBytes(itemSize)
        , m_hashValue(hashValue) 
        , m_compressedHashValue(hashValueCompressed)
        , m_totalNumberOfCompressedBytes(itemSizeCompressed)
    {
    }
    virtual ~DownloadPath() {}
    CString GetUrl() const { return m_url; }
    ULONGLONG GetItemSize() const 
    {
        return m_totalNumberOfBytes;
    } 
    CString GetHashValue() const 
    { 
        return m_hashValue; 
    }

    ULONGLONG GetCompressedItemSize() const 
    {
        return m_totalNumberOfCompressedBytes;
    }

    CString GetCompressedHashValue() const 
    { 
        return m_compressedHashValue; 
    }

};

//------------------------------------------------------------------------------
// Class: IronMan::LocalPath
// This is the "local" path of the item, i.e. the place the downloader puts it
//------------------------------------------------------------------------------
class LocalPath
{
    mutable CString m_name;
    CString m_originalName; // authored and unaltered value of Name attribute.
    ULONGLONG m_systemDriveSize;
    ULONGLONG m_installedProductSize;
    bool m_lockFileAfterVerification;
    bool m_itemCompressed;
    bool m_bCache;  // Indicates whether the payload should be cached on the machine
    bool m_bPerMachine;
    CPath m_pathCachedDirectoryName;  // Unique directory name for payload, Hash for exe, ProductCodevProductVersion for MSI, PatchCode for MSP

protected:
    LocalPath(const LocalPath& rhs, const CPath& layoutFolder) 
        : m_name(CombinePath(layoutFolder, rhs.GetName()))
        , m_originalName(rhs.m_originalName)
        , m_systemDriveSize(rhs.m_systemDriveSize)
        , m_installedProductSize(rhs.m_installedProductSize)
        , m_lockFileAfterVerification(rhs.m_lockFileAfterVerification)
        , m_itemCompressed(rhs.m_itemCompressed)
        , m_bCache(rhs.m_bCache)
        , m_bPerMachine(rhs.m_bPerMachine)
        , m_pathCachedDirectoryName(rhs.m_pathCachedDirectoryName)
    {}

    virtual void SetLockFileAfterVerification(bool lockValue)
    {
        m_lockFileAfterVerification = lockValue;
    }

    virtual void SetItemCompressedFlag(bool flag)
    {
        m_itemCompressed = flag;
    }

private:
    static CString CombinePath(const CPath& layout, const CPath& name)
    {
        if (name.IsRelative())
        {
            CPath path;
            path.Combine(layout, name);
            return CString(path);
        }
        else
            return name;
    }

public:
    //  Constructor (ParameterInfo)
    LocalPath(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_name(ElementUtils::GetAttributeByName(spElement, L"Name", logger))
        , m_originalName(m_name)
        , m_systemDriveSize(ElementUtils::GetAttributeULONGLONGByName(spElement, L"SystemDriveSize", logger))
        , m_installedProductSize(ElementUtils::GetAttributeULONGLONGByName(spElement, L"InstalledProductSize", logger))
        , m_lockFileAfterVerification(true)
        , m_itemCompressed(false)
        , m_bCache(ElementUtils::EvaluateToBoolValue(L"Cache", true, ElementUtils::GetOptionalAttributeByName(spElement, L"Cache", logger), logger))
        , m_bPerMachine(ElementUtils::EvaluateToBoolValue(L"PerMachine", false, ElementUtils::GetOptionalAttributeByName(spElement, L"PerMachine", logger), logger))

    {
        // Read LOCK attribute
        // Not needed for Beta1 - items are always locked.
        // if (0 == ElementUtils::GetOptionalAttributeByName(spElement, L"Lock", logger).CompareNoCase(L"false"))
        //    m_lockFileAfterVerification = false;
        // 

        // Sum of SystemDriveSize and InstalledProductSize must be less than or equal to MaxULONGLONG
        if ( (m_systemDriveSize + m_installedProductSize) < m_systemDriveSize
            || (m_systemDriveSize + m_installedProductSize) < m_installedProductSize)
        {
            CInvalidXmlException ixe(L"schema validation failure: Sum of SystemDriveSize and InstalledProductSize must be less than or equal to MaxULONGLONG.");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
        // Sum of SystemDriveSize and InstalledProductSize cannot be zero due to the weighted progress bar, so set m_systemDriveSize to 1
        if ( 0 == m_systemDriveSize + m_installedProductSize)
        {
            m_systemDriveSize = 1;
        }

        // Get Cache Directory Name.  Required for MSP, MSI and all Exe except Local type
        m_pathCachedDirectoryName = CPath(ElementUtils::GetOptionalAttributeByName(spElement, L"CacheId", logger));
    }

    LocalPath(const LocalPath& rhs) 
        : m_name(rhs.m_name) 
        , m_originalName(rhs.m_originalName)
        , m_systemDriveSize(rhs.m_systemDriveSize)
        , m_installedProductSize(rhs.m_installedProductSize)
        , m_lockFileAfterVerification(rhs.m_lockFileAfterVerification)
        , m_itemCompressed(rhs.m_itemCompressed)
        , m_bCache(rhs.m_bCache)
        , m_bPerMachine(rhs.m_bPerMachine)
        , m_pathCachedDirectoryName(rhs.m_pathCachedDirectoryName)
    {}

    LocalPath(const CString& name
            , const ULONGLONG systemDriveSize
            , const ULONGLONG installedProductSize
            , bool itemCompressed = false
            , bool bCache = true
            , bool bPerMachine = false
            , const CPath& pathCachedDirectoryName = L"")
        : m_name(name)
        , m_originalName(m_name)
        , m_systemDriveSize(systemDriveSize)
        , m_installedProductSize(installedProductSize)
        , m_itemCompressed(itemCompressed)
        , m_bCache(bCache)
        , m_bPerMachine(bPerMachine)
        , m_pathCachedDirectoryName(pathCachedDirectoryName)
    {}

    virtual ~LocalPath() {}

    void UpdateName(const CString& name) const
    {
        m_name = name;
    }

    const CPath GetName() const
    {
        return CPath(m_name);
    }

    // Returns the authored Name attribute value unaltered.
    const CPath GetOriginalName() const
    {
        return CPath(m_originalName);
    }

    ULONGLONG GetSystemDriveSize() const 
    {
        return m_systemDriveSize;
    } 

    ULONGLONG GetInstalledProductSize() const 
    {
        return m_installedProductSize;
    } 

    bool LockFileAfterVerification() const
    {
        return m_lockFileAfterVerification;
    }

    bool IsItemCompressed() const
    {
        return m_itemCompressed;
    }

    bool DoesItemNeedToBeCached(CPath& pathCachedFileName, bool& bPerMachine) const
    {
        pathCachedFileName = m_pathCachedDirectoryName;
        CPath pathCachedFile = CPath(m_originalName);
        pathCachedFile.StripPath();
        pathCachedFileName.Append(pathCachedFile);
        bPerMachine = m_bPerMachine;
        return m_bCache;
    }

    // Creates a copy of the item with new 'layout' path.
    virtual ItemBase* Twin(const CPath& layout) const = 0; // like clone, but with updated path
};

class CanonicalTargetName
{
    CString m_name;
public:
    CanonicalTargetName(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_name(ElementUtils::GetAttributeByName(spElement, L"CanonicalTargetName", logger).Trim())
    {}
    CanonicalTargetName(const CanonicalTargetName& rhs)   : m_name(rhs.m_name) {}
    CanonicalTargetName(const CString& name): m_name(name) {}
    virtual ~CanonicalTargetName() {}
    const CString& GetCanonicalTargetName() const { return m_name; }
};

// class IgnoreDownloadFailure: Supports auhtoring of attribute that specifies that 
// the item/child item's  download failure can be ignored.
// -----------------------------------------------------------------------------
// Note that now, the old TreatFailureAsSuccess is deprecated and is replaced with IgnoreDownloadFailure.
// This per item flag (except for Patches element, where each msp can have its own IgnoreDownloadFailure flag
// controls only download failure response of the installer (to ignore or FAIL).
// Also note that an Item's Failure behavior is now controlled by FailureAction attribute in the action table.
class IgnoreDownloadFailure
{
    enum IgnoreDownloadFailureState
    {
        Unspecified,
        True,
        False
    };
    const IgnoreDownloadFailureState m_state;

public:
    IgnoreDownloadFailure(const CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_state(EvaluateIgnoreDownloadFailure(ElementUtils::GetOptionalAttributeByName(spElement, L"IgnoreDownloadFailure", logger), logger))
    {}
    IgnoreDownloadFailure(const IgnoreDownloadFailure& rhs)   : m_state(rhs.m_state) {}
    IgnoreDownloadFailure(const bool m_bIgnoreDownloadFailure) 
        : m_state(EvaluateIgnoreDownloadFailure(CString(m_bIgnoreDownloadFailure ? L"True" : L"False"), NullLogger::GetNullLogger()))
    {}
    virtual ~IgnoreDownloadFailure() {}
    const bool ShouldIgnoreDownloadFailure() const 
    { 
        return m_state == IgnoreDownloadFailure::True; 
    }

protected:
    bool IsIgnoreDownloadFailureUnspecified() const 
    { 
        return m_state == IgnoreDownloadFailure::Unspecified; 
    }

private:
    //This is a static class because we are using it in the constuctor
    static IgnoreDownloadFailureState EvaluateIgnoreDownloadFailure(const CString& csState, ILogger& logger)
    {
        if (csState.IsEmpty())
            return IgnoreDownloadFailure::Unspecified;
        if (0 == csState.CompareNoCase(L"true"))
            return IgnoreDownloadFailure::True;
        if (0 == csState.CompareNoCase(L"false"))
            return IgnoreDownloadFailure::False;        

        CInvalidXmlException ixe(L"schema validation failure:  invalid IgnoreDownloadFailure attribute value");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }
};

class RollbackOnPackageInstallFailure
{   
    const bool m_state;

public:
    RollbackOnPackageInstallFailure(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_state(ElementUtils::EvaluateToBoolValue(L"RollBack", true, ElementUtils::GetOptionalAttributeByName(spElement, L"Rollback", logger), logger))
    {}
    RollbackOnPackageInstallFailure(const RollbackOnPackageInstallFailure& rhs)   
        : m_state(rhs.m_state) 
    {}
    RollbackOnPackageInstallFailure(const bool m_bRollback) 
        : m_state(ElementUtils::EvaluateToBoolValue(L"Rollback", true, CString(m_bRollback ? L"True" : L"False"), NullLogger::GetNullLogger()))
    {}
    virtual ~RollbackOnPackageInstallFailure() 
    {}
    const bool ShouldRollBack() const 
    { 
        return m_state; 
    }
};

class ActionTable
{
public:
    enum Actions
    {
        Install,
        Uninstall,
        Repair,
        Noop
    };

private:
    class Action
    {
        Actions m_ifPresent, m_ifAbsent;
        FailureActionEnum m_failureAction;
    public:
        Action(Actions ifPresent, Actions ifAbsent, const FailureActionEnum& failureAction)
            : m_ifPresent(ifPresent)
            , m_ifAbsent (ifAbsent)
            , m_failureAction (failureAction)
        {}
        Action(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : m_ifPresent(GetAction(L"IfPresent", ElementUtils::GetAttributeByName(spElement, L"IfPresent", logger), logger))
            , m_ifAbsent (GetAction(L"IfAbsent",  ElementUtils::GetAttributeByName(spElement, L"IfAbsent",  logger), logger))
            , m_failureAction(GetFailureAction(spElement, logger))
        {
        }

        Actions GetAction(bool present) const
        {
            return present ? m_ifPresent : m_ifAbsent;
        }

        FailureActionEnum GetFailureAction() const
        {
            return m_failureAction;
        }

    private:

        // Returns FailureActionEnum given a String
        static FailureActionEnum GetFailureActionEnum(const CString& failureAction)
        {
            if (failureAction.CompareNoCase(L"rollback") == 0)
                return FailureActionEnum::GetRollbackAction();
            if (failureAction.CompareNoCase(L"stop") == 0)
                return FailureActionEnum::GetStopAction();
            if (failureAction.CompareNoCase(L"continue") == 0)
                return FailureActionEnum::GetContinueAction();

            CInvalidXmlException ixe(L"schema validation error: " + failureAction + L" is not a valid attribute value. Rollback, Stop and Contiue are suppored values for OnFailureAction");
            throw ixe;

        }
        static FailureActionEnum GetInstallFailureActionEnum(const CString& failureAction)
        {
            if (failureAction.IsEmpty())
                return FailureActionEnum::GetRollbackAction(false);
            return GetFailureActionEnum(failureAction);
        }

        static FailureActionEnum GetRepairAndUninstallFailureActionEnum(const CString& failureAction)
        {
            if (failureAction.IsEmpty())
                return FailureActionEnum::GetContinueAction(false);
            if (0 == failureAction.CompareNoCase(L"rollback"))
            {
                CInvalidXmlException ixe(L"schema validation error: Rollback is a not valid OnFailureAciton attribute value for RepairAction or UninstallAction." );
                throw ixe;
            }
            return GetFailureActionEnum(failureAction);
        }

        static FailureActionEnum GetFailureAction(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        {
            CString failureAction = ElementUtils::GetOptionalAttributeByName(spElement, L"OnFailureBehavior", logger);
            
            CString installModeName = ElementUtils::GetName(spElement);
            if (0 == installModeName.CompareNoCase(L"installaction"))
            {
                return GetInstallFailureActionEnum(failureAction);
            }
            if (0 == installModeName.CompareNoCase(L"repairaction"))
            {
                return GetRepairAndUninstallFailureActionEnum(failureAction);
            }
            if (0 == installModeName.CompareNoCase(L"uninstallaction"))
            {
                return GetRepairAndUninstallFailureActionEnum(failureAction);
            }
            CInvalidXmlException ixe(L"schema validation error: " + installModeName + L" doesn't support OnFailureBehavior attrbute." );
            throw ixe;
        }

        static Actions GetAction(const CString& attributeName, const CString& action, ILogger& logger)
        {
            if (action == L"install")   return Install;
            if (action == L"uninstall") return Uninstall;
            if (action == L"repair")    return Repair;
            if (action == L"noop")      return Noop;

            CInvalidXmlException ixe(L"schema validation error:  invalid attribute value for " + attributeName + L": " + action);
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    };
    const Action m_install, m_uninstall, m_repair;

public:
    ActionTable(ActionTable::Actions installPresent,   ActionTable::Actions installAbsent,   FailureActionEnum installFailureAction,
                ActionTable::Actions uninstallPresent, ActionTable::Actions uninstallAbsent, FailureActionEnum uninstallFailureAction,
                ActionTable::Actions repairPresent,    ActionTable::Actions repairAbsent,    FailureActionEnum repairFailureAciton)
        : m_install  (installPresent,   installAbsent,    installFailureAction)
        , m_uninstall(uninstallPresent, uninstallAbsent,  uninstallFailureAction)
        , m_repair   (repairPresent,    repairAbsent,     repairFailureAciton)
    {}
    ActionTable()
        : m_install  (Noop, Install, FailureActionEnum::GetRollbackAction(false))
        , m_uninstall(Uninstall, Noop, FailureActionEnum::GetContinueAction(false))
        , m_repair   (Repair, Install, FailureActionEnum::GetContinueAction(false))
    {}
    // Constructor to help unit tests
    ActionTable(FailureActionEnum installFailureAction,
                FailureActionEnum uninstallFailureAction,
                FailureActionEnum repairFailureAciton)
                : m_install  (Noop,      Install,   installFailureAction)
                , m_uninstall(Uninstall, Noop,      uninstallFailureAction)
                , m_repair   (Repair,    Install,   repairFailureAciton)
    {}


    ActionTable(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_install  (ElementUtils::FindChildElementByName(spElement, L"InstallAction",   logger), logger)
        , m_uninstall(ElementUtils::FindChildElementByName(spElement, L"UninstallAction", logger), logger)
        , m_repair   (ElementUtils::FindChildElementByName(spElement, L"RepairAction",    logger), logger)
    {
        ElementUtils::VerifyName(spElement, L"ActionTable", logger);
        if (3 != ElementUtils::CountChildElements(spElement))
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of ActionTable child nodes!");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }
    virtual ~ActionTable() {}

    // TODO:  think about this.  Should these 3 methods be combined into 1?
    Actions GetInstallAction  (bool present) const 
    { 
        return   m_install.GetAction(present); 
    }
    Actions GetUninstallAction(bool present) const 
    { 
        return m_uninstall.GetAction(present); 
    }
    Actions GetRepairAction   (bool present) const 
    { 
        return    m_repair.GetAction(present); 
    }

    FailureActionEnum GetInstallFailureAction() const
    {
        return m_install.GetFailureAction();
    }

    FailureActionEnum GetUninstallFailureAction() const
    {
        return m_uninstall.GetFailureAction();
    }

    FailureActionEnum GetRepairFailureAction() const
    {
        return m_repair.GetFailureAction();
    }

    // Returns true if item is required to perform action
    // MSI, AgileMSI, Msp, Patches etc don't require payload for Repair and Uninstall.
    // Exe, File do require payload to be present.
    virtual bool IsPayloadRequired(Actions installAction) const
    {
        if (Install == installAction)
            return true;
        return false;
    }
};

// Fully functional Exe class except that this doesn't have helper items.
class ExeBase : public ItemBase
            , public LocalPath
            , public DownloadPath
            , public IsPresent
            , public ApplicableIf
            , public ActionTable
            , public CustomErrorHandling
            , public CanonicalTargetName
            , public IgnoreDownloadFailure
            , public RollbackOnPackageInstallFailure
{
public:
    class ExeTypeEnum
    {    
        enum ExeTypeEnum_ { Cartman_, IronMan_, HotIron_, MsuPackage_, LocalExe_, Unknown_ };
        ExeTypeEnum_    m_type;

        ExeTypeEnum(ExeTypeEnum_ type) : m_type(type) { }
    public:
        static const ExeTypeEnum& GetCartmanType(void)
        {
            static const ExeTypeEnum cartman(Cartman_);
            return cartman; 
        }

        static const ExeTypeEnum& GetIronManType(void)
        {
            static const ExeTypeEnum ironMan(IronMan_);
            return ironMan; 
        }

        static const ExeTypeEnum& GetMsuPackageType(void)
        {
            static const ExeTypeEnum msuPackage(MsuPackage_); 
            return msuPackage; 
        }

        static const ExeTypeEnum& GetUnknownType(void)
        {
            static const ExeTypeEnum unknown(Unknown_); 
            return unknown; 
        }

        static const ExeTypeEnum& GetLocalExeType(void)
        {
            static const ExeTypeEnum LocalExe(LocalExe_);
            return LocalExe;
        }

        bool IsCartmanExe(void)  const
        {
            return (m_type == Cartman_);
        }

        bool IsIronManExe(void)  const
        {
            return (m_type == IronMan_);
        }

        bool IsMsuPackageExe(void)  const
        {
            return (m_type == MsuPackage_); 
        }

        bool IsLocalExe(void) const
        {
            return (m_type == LocalExe_); 
        }

        bool IsUnknownExe(void)  const
        {
            return (m_type == Unknown_);
        }

    private:
        ExeTypeEnum(); // Hide default constructor
    };

    // Performs validation on log file hint string.
    static const bool IsValidLogFileHint(const CString& logFileHint, ILogger& logger)
    {
        if (logFileHint.GetLength() < 3) // "x.*"
        {
            CString log;
            log.Format(L"LogFileHint [%s] is invalid. Too few characters passed in.", logFileHint);
            LOG(logger, ILogger::Warning, log);
            return false;
        }
        
        if (0 == logFileHint.FindOneOf(L"*?\\"))
        {
            CString log;
            log.Format(L"LogFileHint [%s] is invalid. First character must not be '*', '?' or '\\'.", logFileHint);
            LOG(logger, ILogger::Warning, log);
            return false;
        }
        
        CString extension = logFileHint;
        extension.TrimRight();
        int extensionStart = extension.ReverseFind('.');

        if ( (extensionStart < 1) || (extensionStart == extension.GetLength() - 1) ) // X.* is valid
        {
            CString log;
            log.Format(L"LogFileHint [%s] is invalid. Log File hint extension is required.", logFileHint);
            LOG(logger, ILogger::Warning, log);
            return false;
        }
        return true;
    }


private:
    mutable CString m_install;
    CString m_uninstall, m_repair;
    ExeTypeEnum m_exeType;
    mutable CString m_logFileHint;
    ILogger& m_logger; 

    static const ExeTypeEnum& GetExeTypeAttribute(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        CString exeTypeAttribute = ElementUtils::GetOptionalAttributeByName(spElement, L"ExeType", logger);

        if (exeTypeAttribute == L"Cartman")
            return ExeTypeEnum::GetCartmanType();
        
        if (exeTypeAttribute == L"IronMan")
            return ExeTypeEnum::GetIronManType();
        
        if (exeTypeAttribute == L"LocalExe")
            return ExeTypeEnum::GetLocalExeType();
    
        if (exeTypeAttribute == L"MsuPackage")
            return ExeTypeEnum::GetMsuPackageType();
    
        if (exeTypeAttribute == L"HotIron")
        {
            IMASSERT2(0, L"Exe type HotIron is not yet implemented");
            throw E_NOTIMPL;
        }

        return ExeTypeEnum::GetUnknownType();
    }

    // For LocalExe type, this method validates that name begins with environment token 
    // and updates the local path name to the expanded path.
    // Throws on invalid Name authoring.
    void UpdateLocalExeName()
    {
        if (m_exeType.IsLocalExe())
        {
            CString unExpandedExe = LocalPath::GetName();
            CString validationErrorMessage;
            // Name sould contain minimum of a valid environmental variable pointing to an installed program to run.
            if ((unExpandedExe.GetLength() > 2 && unExpandedExe[0] == '%' && unExpandedExe.Find('%', 1) > 1))
            {
                CString expandedExe;
                if (CSystemUtil::ExpandEnvironmentVariables(LocalPath::GetName(), expandedExe))
                {
                    LocalPath::UpdateName(expandedExe);
                    return;
                }
                else
                    validationErrorMessage.Format(L"[%s] - schema validation failure. Environment variable cannot be expanded! Name sould contain minimum of a valid environmental variable pointing to an installed program to run.", unExpandedExe);
            }
            else
                validationErrorMessage.Format(L"[%s] - schema validation failure. Name sould contain minimum of a valid environmental variable pointing to an installed program to run.", unExpandedExe );
            CInvalidXmlException ixe(validationErrorMessage);
            throw ixe;
        }
    }


    void InternalConstructorVerification(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        CString strName = GetCanonicalTargetName();
        if (ShouldRollBack() && GetUninstallCommandLine().IsEmpty())
        {
                CInvalidXmlException ixe(L"When Rollback is true for item " + strName + L" a valid UninstallCommandLine is required.");
                throw ixe;
        }
        if (!m_logFileHint.IsEmpty())
        {
            CString logFileHint;
            int iTokenPosition = 0;
            int iLogFilesCount = 0;
            CString logFileHintToken = m_logFileHint.Tokenize(L"|", iTokenPosition);

            bool bLogFileHintValid = true;
            while (iTokenPosition != -1)
            {
                 CSystemUtil::ExpandEnvironmentVariables(logFileHintToken, logFileHint);
                 if (!IsValidLogFileHint(logFileHint, logger))
                 {
                    bLogFileHintValid = false;
                 }
                 logFileHintToken = m_logFileHint.Tokenize(L"|", iTokenPosition);
            }

            if (!bLogFileHintValid)
            {
                // Detail of each invalid hint is already logged by IsValidLogFileHint
                CInvalidXmlException ixe(L"schema validation failure: " + strName + L" has invalid LogFileHint");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }

        // Command lines need to be empty for MSU packages
        if (m_exeType.IsMsuPackageExe())
        {
            if ( !m_install.IsEmpty() || !m_uninstall.IsEmpty() || !m_repair.IsEmpty() )
            {
                CInvalidXmlException ixe(L"schema validation failure:  The InstallCommandLine, UninstallCommandLind and RepairCommandLine of an ExeBase of MsuPackage like " + strName + L" must be empty.");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }

        if (m_exeType.IsLocalExe())
        {
            if ( !(GetUrl().IsEmpty() && GetHashValue().IsEmpty() && (0 == GetItemSize())) )
            {
                CInvalidXmlException ixe(L"schema validation failure. URL, Sha1HashValue and DownLoadSize attributes are not valid for LocalExe type like " + strName + L" .");
                throw ixe;
            }
            UpdateLocalExeName();
        }
        else
        {
            // Every other type of Exe other than Local is required to have a Sha1HashValue
            if ( GetHashValue().IsEmpty() )
            {
                CInvalidXmlException ixe(L"schema validation failure. Sha1HashValue is required for " + strName + L".  Sha1HashValue is required for all Exe types except LocalExe.");
                throw ixe;
            }
        }
    }
protected:
    // Copy Constructor
    ExeBase(const ExeBase& rhs)
        : ItemBase(rhs)
        , DownloadPath(rhs)
        , LocalPath(rhs)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , CustomErrorHandling(rhs)
        , ActionTable(rhs)
        , CanonicalTargetName(rhs)
        , IgnoreDownloadFailure(rhs)
        , RollbackOnPackageInstallFailure(rhs)
        , m_install (rhs.m_install)
        , m_uninstall(rhs.m_uninstall)
        , m_repair(rhs.m_repair)
        , m_logFileHint(rhs.m_logFileHint)
        , m_exeType(rhs.m_exeType)
        , m_logger(rhs.m_logger)
    {}

    // Constructor with layout path
    ExeBase(const ExeBase& rhs, const CPath& path)
        : ItemBase(rhs)
        , DownloadPath(rhs)
        , LocalPath(rhs, path)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
        , CanonicalTargetName(rhs)
        , IgnoreDownloadFailure(rhs)
        , RollbackOnPackageInstallFailure(rhs)
        , m_install (rhs.m_install)
        , m_uninstall(rhs.m_uninstall)
        , m_repair(rhs.m_repair)
        , m_logFileHint(rhs.m_logFileHint)
        , m_exeType(rhs.m_exeType)
        , m_logger(rhs.m_logger)
    {}

public:
     ExeBase(const CString& name
         , const CString& url
         , const CString& installCommandLine
         , const CString& uninstallCommandLine
         , const CString& repairCommandLine
         , ULONGLONG itemSize
         , ULONGLONG systemDriveSize
         , ULONGLONG installedProductSize
         , const CString& canonicalTargetName
         , const bool shouldIgnoreDownloadFailure
         , const bool shouldRollback
         , const CString& logFileHint
         , ILogger& logger
         , const ExeTypeEnum& exeType=ExeTypeEnum::GetUnknownType()
         , const ActionTable& at = ActionTable()
         , ULONGLONG estimatedInstallTime = 0
        )
        : ItemBase(ItemBase::Exe, estimatedInstallTime)
        , DownloadPath(url, itemSize)
        , LocalPath(name, systemDriveSize, installedProductSize)
        , CanonicalTargetName(canonicalTargetName)
        , IgnoreDownloadFailure(shouldIgnoreDownloadFailure)
        , ActionTable(at)
        , RollbackOnPackageInstallFailure(shouldRollback)
        , m_install  (installCommandLine)
        , m_uninstall(uninstallCommandLine)
        , m_repair(repairCommandLine)
        , m_logFileHint(logFileHint)
        , m_exeType(exeType)
        , m_logger(logger)
    {
        if (m_exeType.IsLocalExe())
        {
            UpdateLocalExeName();
        }
     }

     // Main constructor
    ExeBase(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ItemBase(ItemBase::Exe, spElement, logger)
        , LocalPath(spElement, logger)
        , DownloadPath(spElement, logger)
        , CanonicalTargetName(spElement, logger)
        , CustomErrorHandling(ElementUtils::FindOptionalChildElementByName(spElement, L"CustomErrorHandling", logger), logger)
        , IgnoreDownloadFailure(spElement, logger)
        , RollbackOnPackageInstallFailure(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , m_install  (ElementUtils::GetAttributeByName(spElement, L"InstallCommandLine", logger))
        , m_uninstall(ElementUtils::GetAttributeByName(spElement, L"UninstallCommandLine", logger))
        , m_repair(ElementUtils::GetAttributeByName(spElement, L"RepairCommandLine", logger))
        , m_logFileHint(ElementUtils::GetOptionalAttributeByName(spElement, L"LogFileHint", logger))
        , m_exeType(GetExeTypeAttribute(spElement, logger))
        , m_logger(logger)
    {
        ElementUtils::VerifyName(spElement, L"Exe", logger);
        if (3 > ElementUtils::CountChildElements(spElement))
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of EXE child nodes!");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }

        // Ensure that all compressed items are downlodable and have compressed download size authored.
        bool bItemCompressed = ElementUtils::EvaluateToBoolValue(L"Compressed", false, ElementUtils::GetOptionalAttributeByName(spElement, L"Compressed", logger), logger);
        if (bItemCompressed)
        {
             LocalPath::SetItemCompressedFlag(bItemCompressed);
             if (GetUrl().IsEmpty() || (0 == GetCompressedItemSize()))
             {
                 throw CInvalidXmlException(L"Compressed items need to have URL and CompressedDownloadSize authored.");
             }
        }

        InternalConstructorVerification(spElement, logger);
    }

    // Helper item constructor
    ExeBase ( CComPtr<IXMLDOMElement> spElement
            , const CString& name
            , const CString& command
            , const ULONGLONG systemDriveSize
            , const ULONGLONG installedProductSize
            , ILogger& logger
            , const IsPresent& isPresent = IsPresent()
            , const ApplicableIf& applicableIf = ApplicableIf()
            , const ActionTable& actionTable = ActionTable())
        : ItemBase(ItemBase::Exe, 
                   ElementUtils::GetOptionalAttributeULONGLONGByName(spElement, L"EstimatedInstallTime", logger))
        , LocalPath(name, systemDriveSize, installedProductSize)
        , DownloadPath(spElement, logger)
        , CanonicalTargetName(spElement, logger)
        , IgnoreDownloadFailure(true)
        , RollbackOnPackageInstallFailure(false)
        , IsPresent(isPresent)
        , ApplicableIf(applicableIf)
        , ActionTable(actionTable)
        , m_install (command)
        , m_uninstall(command)
        , m_repair(command)
        , m_logFileHint(ElementUtils::GetOptionalAttributeByName(spElement, L"LogFileHint", logger))
        , m_exeType(GetExeTypeAttribute(spElement, logger))
        , m_logger(logger)
    {
        // Perform verification here.
        InternalConstructorVerification(spElement, logger);
        SetLockFileAfterVerification(false);
    }

    virtual ItemBase* Clone() const
    {
        return new ExeBase(*this);
    }
    virtual ItemBase* Twin(const CPath& layout) const
    {
        CPath local;
        local.Combine(layout, this->GetName());
        // For helpers and overrides, we have to check if file exists and use %temp%\packagename folder if
        // it doesn't.
        if (local.FileExists())
            return new ExeBase(*this, layout);
        else
            return new ExeBase(*this);
    }

    // Returns true if item is required to perform action
    // MSI, AgileMSI, Msp, Patches etc don't require payload for Repair and Uninstall.
    // Exe, File do require payload to be present.
    virtual bool IsPayloadRequired(Actions installAction) const
    {
        if (Noop == installAction)
            return false;
        return true;
    }

    // Gets switch used to tell exe that it is in "install" mode
    CString GetInstallCommandLine() const 
    { 
        return ResolveDollarVariables(m_install); 
    }

    // Sets the commandline that will be used during install
    void OverrideInstallCommandLine(const CString& installCommand) const 
    { 
        m_install = installCommand; 
    }

    // Gets switch used to tell exe that it is in "uninstall" mode
    CString GetUninstallCommandLine() const 
    {         
        return ResolveDollarVariables(m_uninstall); 
    }

    // Gets switch used to tell exe that it is in "uninstall" mode
    CString GetRepairCommandLine() const 
    { 
        return ResolveDollarVariables(m_repair); 
    }

    // Gets hint for name of log file in temp directory
    CString GetLogFileHint() const 
    { 
        return ResolveDollarVariables(m_logFileHint); 
    }
    
    // Sets the logfile hint
    void OverrideLogFileHint(const CString& logFileHint) const 
    { 
        m_logFileHint = logFileHint; 
    }

    ExeBase::ExeTypeEnum GetExeType(void) const { return m_exeType; }

    private:
        CString ResolveDollarVariables(const CString& textIn) const
        {
            class ParamGetter
            {
                CString m_csLogFileFolder;

            public:         
                ParamGetter(CString csLogFileFolder)
                    : m_csLogFileFolder(csLogFileFolder)
                {}

                CString GetParamValue(CString param) const
                { 
                    if (0 == param.CompareNoCase(L"$$LogFileFolder$$"))
                    {
                        CPath logFilePath(m_csLogFileFolder);
                        logFilePath.RemoveFileSpec();
                        return logFilePath;
                    }

                    if (0 == param.CompareNoCase(L"$$LogFilePrefix$$"))
                    {
                        return ModuleUtils::GetFileNameOnlyWithoutExtension(m_csLogFileFolder);
                    }   
                    IMASSERT2(false, L"Unsupported token!");
                    return param;
                }
            }paramGetter(m_logger.GetFilePath());

            ParamSubstituter<ParamGetter> ps(paramGetter, L"$$", L"$$");
            return ps.SubstituteAnyParams(textIn);
        }
};

typedef CSimpleMap<CString, ItemBase*> HelperMap;

// Container class of Helpers.
class HelperItems
{
    struct ItemMap : public CSimpleMap<CString, ItemBase*>
    {
        // Default Constructor
        ItemMap() {}

        // Copy Constructor with layout path
        ItemMap(const ItemMap& rhs)
        {
            for(int i=0; i<rhs.GetSize(); ++i)
            {
                Add(rhs.GetKeyAt(i), rhs.GetValueAt(i)->Clone()); 
            }
        }

        // Copy Constructor with layout path
        ItemMap(const ItemMap& rhs, const CPath& layout)
        {
            for(int i=0; i<rhs.GetSize(); ++i)
            {
                const ExeBase* exeBase = dynamic_cast<const ExeBase*>(rhs.GetValueAt(i));
                Add(rhs.GetKeyAt(i), exeBase->Clone());
            }
        }


        ItemMap(const CSimpleMap<CString, ItemBase*>& rhs)
        {
            for(int i=0; i<rhs.GetSize(); ++i)
                Add(rhs.GetKeyAt(i), rhs.GetValueAt(i)->Clone());
        }

        // Updates each helper by checking if the helper is available in workingFolder first
        // and if not, updates the helper path layout + filename
        void Update(const CPath& workingFolder, const CPath& layout, bool preferLocal) const
        {
            for (int i=0; i < GetSize(); ++i)
            {
                LocalPath* path = const_cast<LocalPath*>(dynamic_cast<const LocalPath*>((*this).GetValueAt(i)));
                if (path)
                {
                    if (preferLocal)
                    {
                        CPath payloadPath;
                        payloadPath.Combine(workingFolder, path->GetName());
 
                        if (payloadPath.FileExists())
                        {
                            path->UpdateName(payloadPath);
                            continue;
                        }
                    }
                    // Check if this is local Exe.
                    const IronMan::ExeBase* exe = dynamic_cast<const IronMan::ExeBase *>((*this).GetValueAt(i));
                    bool helperIsLocalExe = false;

                    if (NULL != exe)
                    {
                        if (exe->GetExeType().IsLocalExe())
                            helperIsLocalExe = true;
                    }

                    CPath layoutPath;
                    // LocalExe path doesn't need to be changed/updated as it is already expanded.
                    if (!helperIsLocalExe)
                    {
                        layoutPath.Combine(layout, path->GetOriginalName());
                        path->UpdateName(layoutPath);
                    }
                }
            }
        }
        virtual ~ItemMap()
        {
            for (int i=0; i<GetSize(); ++i)
            {
                delete (*this).GetValueAt(i);
            }
        }
    } m_helpersMap;

public:
    // Test Hook
    HelperItems(const CSimpleArray<ExeBase*>& helpers)
    {
        AddItems(helpers);
    }

    // Default Constructor
    HelperItems()
    {
    }

    // Copy Constructor
    HelperItems(const HelperItems& rhs) : m_helpersMap(rhs.m_helpersMap)
    {
    }

    // Copy Constructor with layout path
    HelperItems(const HelperItems& rhs, const CPath& layout) : m_helpersMap(rhs.m_helpersMap, layout)
    {

    }

    // Main Constructor
    HelperItems(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        AddItems(spElement, L"RetryHelper", logger);
    }

    // Adds additional hepler items, after constructor added default helper items
    int AddHelperItems(const CComPtr<IXMLDOMElement>& spElement, const CString& helperName, ILogger& logger)
    {
        return AddItems(spElement, helperName, logger);
    }

    // Returns true and helper Item if it exists in the helper map.
    // Else, return false;
    bool GetHelper(const CString& name, const ItemBase** helper) const
    {
        int helperIndex = m_helpersMap.FindKey(name);
        if (-1 == helperIndex)
            return false;
        *helper = dynamic_cast<const ItemBase*>(m_helpersMap.GetValueAt(helperIndex));
        return true;
    }

    // Update Helpers by checking if they are available in workingFolder or Layout
    // based on preferLocal.
    void Update(const CPath& workingFolder, const CPath& layout, bool preferLocal) const
    {
        m_helpersMap.Update(workingFolder, layout, preferLocal);
    }
    int GetSize() const
    {
        return m_helpersMap.GetSize();
    }

    const ItemBase* GetValueAt(int i) const
    {
        return m_helpersMap.GetValueAt(i);
    }

    virtual ~HelperItems()
    {
    }

    // Adds one helper item and returns the name that can be used as index
    // when calling GetHelper()
    CString AddItem(const CComPtr<IXMLDOMNode>& spNode, ILogger& logger)
    {
        CString helperName;
        CComPtr<IXMLDOMElement> spElement = CComQIPtr<IXMLDOMElement>(spNode);
        CString name = ElementUtils::GetAttributeByName(spElement, L"Name", logger, false);
        CString command = ElementUtils::GetOptionalAttributeByName(CComQIPtr<IXMLDOMElement>(spNode), L"CommandLine", logger);
        ULONGLONG systemDriveSize = ElementUtils::GetOptionalAttributeULONGLONGByName(CComQIPtr<IXMLDOMElement>(spNode), L"SystemDriveSize", logger);
        ULONGLONG installedProductSize = ElementUtils::GetOptionalAttributeULONGLONGByName(CComQIPtr<IXMLDOMElement>(spNode), L"InstalledProductSize", logger);

        CString itemName = ElementUtils::GetName(ElementUtils::GetParentElement(spElement));

        ExeBase* helper = NULL;

        // Patches don't have IsPresent or ApplicableIf
        if (0 == itemName.CompareNoCase(L"Patches"))
        {
            helper = new ExeBase(
                CComQIPtr<IXMLDOMElement>(spNode)
                , name
                , command
                , systemDriveSize
                , installedProductSize
                , logger
                , IsPresent   ()
                , ApplicableIf()
                , ActionTable (ElementUtils::FindOptionalChildElementByName(ElementUtils::GetParentElement(spElement), L"ActionTable", logger), logger));
        }
        else
        {
           helper = new ExeBase(
                CComQIPtr<IXMLDOMElement>(spNode)
                , name
                , command
                , systemDriveSize
                , installedProductSize
                , logger
                , IsPresent   (ElementUtils::FindChildElementByName(ElementUtils::GetParentElement(spElement), L"IsPresent", logger), logger)
                , ApplicableIf(ElementUtils::FindChildElementByName(ElementUtils::GetParentElement(spElement), L"ApplicableIf", logger), logger)
                , ActionTable (ElementUtils::FindChildElementByName(ElementUtils::GetParentElement(spElement), L"ActionTable", logger), logger));
        }

        if (helper)
        {
            helperName = CString(helper->GetName());
            if (-1 == m_helpersMap.FindKey(helperName))
            {
                m_helpersMap.Add(helperName, helper);
            }
            else
            {
                delete helper;
                CInvalidXmlException ixe(L"A helper with this name already exists. All helper names must be unique. : " + helperName);
                throw ixe;
            }
        }
        else
        {
            CCreationException ixe(L"Cannot create the helper item: " + helperName );
            throw ixe;
        }
        return helperName;
    }

private:
    // Test Hook Add method
    void AddItems(const CSimpleArray<ExeBase*>& helpers)
    {
        for (int i = 0; i < helpers.GetSize(); i++)
        {
            if (-1 == m_helpersMap.FindKey(helpers[i]->GetName()))
            {
                m_helpersMap.Add(helpers[i]->GetName(), helpers[i]);
            }
        }
    }

    // Internal method used by constructor and AddItems()
    int AddItems(const CComPtr<IXMLDOMElement>& spElement, const CString& helperName, ILogger& logger)
    {
        long helpersCount = 0, helpersAdded = 0;
        helpersCount = ElementUtils::CountChildElements(spElement, helperName);
        if (0 < helpersCount)
        {
            CComPtr<IXMLDOMNodeList> spHelperItems = ElementUtils::GetChildElementsByName(spElement, helperName, helpersCount, logger);
            for (int i = 0; i < helpersCount; i++)
            {
                CComPtr<IXMLDOMNode> spChild;
                HRESULT hr = spHelperItems->get_item(i, &spChild);
                if (S_OK == hr)
                {
                    AddItem(spChild, logger);
                    helpersAdded++;
                }
            }
        }
        return helpersAdded;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::MSI
// Represents an MSI file which installs something
//------------------------------------------------------------------------------
class MSI : public ItemBase
            , public DownloadPath
            , public LocalPath
            , public IsPresent
            , public ApplicableIf
            , public ActionTable
            , public CustomErrorHandling
#ifdef FeaturesAreImplented
            , public Features
#endif // FeaturesAreImplented
            , public CanonicalTargetName
            , public IgnoreDownloadFailure
            , public RollbackOnPackageInstallFailure
            , public HelperItems
{
    const CString m_productCode;    // This is the "ProductCode" of the MSI/MSP we need to acquire. This is just a UUID
    const CString m_csMsiOptions;   // This is the MSI options that we pass over when making MSI call. 
    const CString m_csMsiUninstallOptions; // MSI uninstall options to pass thru when making MSI call. 
    const CString m_csMsiRepairOptions; // MSI repair options to pass thru when making MSI call. 

protected: // so that AgileMSI can derive from MSI
    MSI(const MSI& rhs, const CPath& path)
        : ItemBase(rhs)
        , DownloadPath(rhs)
        , LocalPath(rhs, path)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
#ifdef FeaturesAreImplented
        , Features(rhs)
#endif // FeaturesAreImplented
        , CanonicalTargetName(rhs)
        , IgnoreDownloadFailure(rhs)
        , RollbackOnPackageInstallFailure(rhs)
        , HelperItems(rhs)
        , m_productCode(rhs.m_productCode)
        , m_csMsiOptions(rhs.m_csMsiOptions)
        , m_csMsiUninstallOptions(rhs.m_csMsiUninstallOptions)
        , m_csMsiRepairOptions(rhs.m_csMsiRepairOptions)
    {}
public:
    MSI(const CString& name
        , const CString& url
        , const CString& uuid
        , const CString& csProductVersion
        , ULONGLONG itemSize
        , ULONGLONG systemDriveSize
        , ULONGLONG installedProductSize
        , const CString& canonicalTargetName
        , const bool shouldIgnoreDownloadFailure
        , const bool shouldRollback
        , const CString& csMsiOption
        , const CString& csMsiUninstallOptions = CString()
        , const CString& csMsiRepairOptions = CString()
        , ULONGLONG estimatedInstallTime = 0
        , const ActionTable& at = ActionTable()
        , const HelperItems& helpers = HelperItems()
        )
        : ItemBase(ItemBase::Msi, estimatedInstallTime)
        , LocalPath(name, systemDriveSize, installedProductSize)
        , DownloadPath(url, itemSize)
        , CanonicalTargetName(canonicalTargetName)
        , IgnoreDownloadFailure(shouldIgnoreDownloadFailure)
        , ActionTable(at)
        , HelperItems(helpers)
        , RollbackOnPackageInstallFailure(shouldRollback)
#ifdef FeaturesAreImplented
        , Features(CComPtr<IXMLDOMElement>(), NullLogger::GetNullLogger())
#endif // FeaturesAreImplented
        , m_productCode(uuid)
        , m_csMsiOptions(csMsiOption)
        , m_csMsiUninstallOptions(csMsiUninstallOptions)
        , m_csMsiRepairOptions(csMsiRepairOptions)
    {}

    MSI(const MSI& rhs)
        : ItemBase(rhs)
        , DownloadPath(rhs)
        , LocalPath(rhs)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
#ifdef FeaturesAreImplented
        , Features(rhs)
#endif // FeaturesAreImplented
        , CanonicalTargetName(rhs)
        , IgnoreDownloadFailure(rhs)
        , RollbackOnPackageInstallFailure(rhs)
        , HelperItems(rhs)
        , m_productCode(rhs.m_productCode)
        , m_csMsiOptions(rhs.m_csMsiOptions)
        , m_csMsiUninstallOptions(rhs.m_csMsiUninstallOptions)
        , m_csMsiRepairOptions(rhs.m_csMsiRepairOptions)
    {}
    MSI(CComPtr<IXMLDOMElement> spElement, ILogger& logger, const CString& elementName=L"MSI")
        : ItemBase(ItemBase::Msi, spElement, logger)
        , DownloadPath(spElement, logger)
        , LocalPath(spElement, logger)
        , CanonicalTargetName(spElement, logger)
        , IgnoreDownloadFailure(spElement, logger)
        , RollbackOnPackageInstallFailure(spElement, logger)
        , HelperItems(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , CustomErrorHandling(ElementUtils::FindOptionalChildElementByName(spElement, L"CustomErrorHandling", logger), logger)
#ifdef FeaturesAreImplented
        , Features(ElementUtils::FindOptionalChildElementByName(spElement, L"Features", logger), logger)
#endif // FeaturesAreImplented
        , m_productCode(ElementUtils::GetAttributeByName(spElement, L"ProductCode", logger, true))
        , m_csMsiOptions(ElementUtils::GetOptionalAttributeByName(spElement, L"MSIOptions", logger, true))
        , m_csMsiUninstallOptions(ElementUtils::GetOptionalAttributeByName(spElement, L"MSIUninstallOptions", logger, true))
        , m_csMsiRepairOptions(ElementUtils::GetOptionalAttributeByName(spElement, L"MSIRepairOptions", logger, true))
    {
        ElementUtils::VerifyName(spElement, elementName, logger);
        unsigned int children = ElementUtils::CountChildElements(spElement);
        // Note that when Features are implemented, there will be 3 or more elements.
        // Helper items are optional and thus only if count of children is < 3, its invalid.
        if ( children < 3) 
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of MSI child nodes!");            
            throw ixe;
        }

        // Ensure that all compressed items are downlodable and have compressed download size authored.
        bool bItemCompressed = ElementUtils::EvaluateToBoolValue(L"Compressed", false, ElementUtils::GetOptionalAttributeByName(spElement, L"Compressed", logger), logger);
        if (bItemCompressed)
        {
             LocalPath::SetItemCompressedFlag(bItemCompressed);
             if (GetUrl().IsEmpty() || (0 == GetCompressedItemSize()))
             {
                 throw CInvalidXmlException(L"Compressed items need to have URL and CompressedDownloadSize authored.");
             }
        }

        // Ensure that this setup item does not contain invalid child elements
        if ( ElementUtils::ContainsInvalidChildElementByName(spElement, L"RepairOverride", logger) || ElementUtils::ContainsInvalidChildElementByName(spElement, L"UninstallOverride", logger) )
        {
            CInvalidXmlException ixe(L"schema validation failure:  MSI, AgileMSI and AgileMSP do not support RepairOverride or UninstallOverride child elements!");            
            throw ixe;
        }

        if (m_productCode.IsEmpty())
        {
            CInvalidXmlException ixe(L"schema validation failure:  Product Code cannot be empty.");            
            throw ixe;
        }

    }
    virtual ItemBase* Clone() const
    {
        return new MSI(*this);
    }
    virtual ItemBase* Twin(const CPath& layout) const
    {
        return new MSI(*this, layout);
    }

    CString GetProductCode() const 
    { 
        return m_productCode; 
    }

    const CString& GetMsiOptions() const 
    { 
        return m_csMsiOptions; 
    }
    const CString& GetMsiUninstallOptions() const
    {
        return m_csMsiUninstallOptions; 
    }
    const CString& GetMsiRepairOptions() const 
    { 
        return m_csMsiRepairOptions; 
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::MSP
// Represents an MSP patch file
//------------------------------------------------------------------------------
class MSP : public ItemBase
            , public LocalPath
            , public DownloadPath
            , public IsPresent
            , public ApplicableIf
            , public ActionTable
            , public CustomErrorHandling
            , public IgnoreDownloadFailure
            , public RollbackOnPackageInstallFailure
            , public HelperItems
{
    const CComBSTR m_blob;
    const CString m_patchCode;  // patch code, so that we don't need to download the MSP to uninstall   

protected: // so AgileMSP can derive
    MSP(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger, const IsPresent& ip, const ApplicableIf& ai, const ActionTable& at, const IgnoreDownloadFailure& idf)
        : ItemBase(ItemBase::Msp, 
                   ElementUtils::GetOptionalAttributeULONGLONGByName(spElement, L"EstimatedInstallTime", logger))
        , DownloadPath(spElement, logger)
        , LocalPath(spElement, logger)
        , IsPresent(ip)
        , ApplicableIf(ai)
        , ActionTable(at)
        , IgnoreDownloadFailure(idf)
        , RollbackOnPackageInstallFailure(spElement, logger)
        , m_patchCode(ElementUtils::GetAttributeByName(spElement, L"PatchCode", logger))

    {}

    MSP(const MSP& rhs, const CPath& path)
        : ItemBase(rhs)
        , DownloadPath(rhs)
        , LocalPath(rhs, path)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
        , IgnoreDownloadFailure(rhs)
        , RollbackOnPackageInstallFailure(rhs)
        , HelperItems(rhs)
        , m_blob(rhs.m_blob)
        , m_patchCode(rhs.m_patchCode)
    {}

public:
    using IgnoreDownloadFailure::IsIgnoreDownloadFailureUnspecified;

    MSP(const MSP& rhs)
        : ItemBase(rhs)
        , DownloadPath(rhs)
        , LocalPath(rhs)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
        , IgnoreDownloadFailure(rhs)
        , RollbackOnPackageInstallFailure(rhs)
        , HelperItems(rhs)
        , m_blob(rhs.m_blob)
        , m_patchCode(rhs.m_patchCode)
    {}
    MSP(const CString& name
        , const CString& patchCode
        , const CString& url
        , ULONGLONG itemSize
        , ULONGLONG systemDriveSize
        , ULONGLONG installedProductSize
        , bool shouldIgnoreDownloadFailure
        , bool shouldRollback
        , ULONGLONG estimatedInstallTime = 0
        , const ActionTable& at = ActionTable()
        , const HelperItems& helpers = HelperItems()
        , const bool bRemovable = true)
        : ItemBase(ItemBase::Msp, estimatedInstallTime)
        , DownloadPath(url, itemSize)
        , LocalPath(name, systemDriveSize, installedProductSize)
        , IgnoreDownloadFailure(shouldIgnoreDownloadFailure)
        , ActionTable(at)
        , RollbackOnPackageInstallFailure(shouldRollback)
        , HelperItems(helpers)
        , m_patchCode(patchCode)
    {}
    MSP(const CString& name
        , const CString& patchCode
        , const CString& url
        , ULONGLONG itemSize
        , const CString& hashValue
        , ULONGLONG systemDriveSize
        , ULONGLONG installedProductSize
        , bool shouldIgnoreDownloadFailure
        , bool shouldRollback
        , ULONGLONG estimatedInstallTime = 0
        , const bool bRemovable = false)
        : ItemBase(ItemBase::Msp, estimatedInstallTime)
        , DownloadPath(url, itemSize, hashValue)
        , LocalPath(name, systemDriveSize, installedProductSize)
        , IgnoreDownloadFailure(shouldIgnoreDownloadFailure)
        , RollbackOnPackageInstallFailure(shouldRollback)
        , m_patchCode(patchCode)
    {}

    MSP(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ItemBase(ItemBase::Msp, spElement, logger)
        , DownloadPath(spElement, logger)
        , LocalPath(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , CustomErrorHandling(ElementUtils::FindOptionalChildElementByName(spElement, L"CustomErrorHandling", logger), logger)
        , IgnoreDownloadFailure(spElement, logger)
        , RollbackOnPackageInstallFailure(spElement, logger)
        , HelperItems(spElement, logger)
        , m_patchCode(ElementUtils::GetAttributeByName(spElement, L"PatchCode", logger))
        , m_blob(GetBlob(spElement, logger))
    {
        ElementUtils::VerifyName(spElement, L"MSP", logger);
        if (3 > ElementUtils::CountChildElements(spElement))
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of MSP child nodes!");
            throw ixe;
        }

        // Ensure that all compressed items are downlodable and have compressed download size authored.
        bool bItemCompressed = ElementUtils::EvaluateToBoolValue(L"Compressed", false, ElementUtils::GetOptionalAttributeByName(spElement, L"Compressed", logger), logger);
        if (bItemCompressed)
        {
             LocalPath::SetItemCompressedFlag(bItemCompressed);
             if (GetUrl().IsEmpty() || (0 == GetCompressedItemSize()))
             {
                 throw CInvalidXmlException(L"Compressed items need to have URL and CompressedDownloadSize authored.");
             }
        }

        if (m_patchCode.IsEmpty())
        {
            CInvalidXmlException ixe(L"schema validation failure:  Patch Code cannot be empty!");
            throw ixe;
        }
        
        // Ensure that this setup item does not contain invalid child elements
        if ( ElementUtils::ContainsInvalidChildElementByName(spElement, L"RepairOverride", logger) || ElementUtils::ContainsInvalidChildElementByName(spElement, L"UninstallOverride", logger) )
        {
            CInvalidXmlException ixe(L"schema validation failure:  MSP does not support RepairOverride or UninstallOverride child elements!");            
            throw ixe;
        }
    }

    MSP(CComPtr<IXMLDOMElement> spElement, ILogger& logger, const ActionTable& at)
        : ItemBase(ItemBase::Msp, spElement, logger)
        , DownloadPath(spElement, logger)
        , LocalPath(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger) 
        , ActionTable(at)
        , IgnoreDownloadFailure(spElement, logger)
        , RollbackOnPackageInstallFailure(spElement, logger)
        , HelperItems(spElement, logger)
        , m_patchCode(ElementUtils::GetAttributeByName(spElement, L"PatchCode", logger))
        , m_blob(GetBlob(spElement, logger))
    {
        ElementUtils::VerifyName(spElement, L"MSP", logger);
        if (2 > ElementUtils::CountChildElements(spElement))
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of MSP child nodes!");        
            throw ixe;
        }

        if (m_patchCode.IsEmpty())
        {
            CInvalidXmlException ixe(L"schema validation failure:  Patch Code cannot be empty!");
            throw ixe;
        }

        // Ensure that this setup item does not contain invalid child elements
        if ( ElementUtils::ContainsInvalidChildElementByName(spElement, L"RepairOverride", logger) || ElementUtils::ContainsInvalidChildElementByName(spElement, L"UninstallOverride", logger) )
        {
            CInvalidXmlException ixe(L"schema validation failure:  MSP does not support RepairOverride or UninstallOverride child elements!");            
            throw ixe;
        }

    }
    virtual ItemBase* Clone() const
    {
        return new MSP(*this);
    }
    virtual MSP* Twin(const CPath& layout) const
    {
        return new MSP(*this, layout);
    }
    const CString& GetPatchCode() const { return m_patchCode; } 

public:
    const CComBSTR& GetBlob() const 
    { 
        return m_blob; 
    } // the whole InstallIf blob, in order to get to (possibly multiple) MsiXmlBlob
    MSP(const CString& name
        , const CString& patchCode
        , const CString& url
        , ULONGLONG itemSize
        , ULONGLONG systemDriveSize
        , ULONGLONG installedProductSize
        , const CComBSTR& blob
        , const bool shouldIgnoreDownloadFailure
        , const bool shouldRollback
        , ULONGLONG estimatedInstallTime = 0
        , const bool bRemovable = true)
        : ItemBase(ItemBase::Msp, estimatedInstallTime)
        , DownloadPath(url, itemSize)
        , LocalPath(name, systemDriveSize, installedProductSize)
        , IgnoreDownloadFailure(shouldIgnoreDownloadFailure)
        , RollbackOnPackageInstallFailure(shouldRollback)
        , m_blob(blob)
        , m_patchCode(patchCode)
    {}

private:
    //This function will iterate through and expand all ExpressionAlias under ApplicableIf.
    //It also ensure that there is at least 1 MsiXmlBlob in the ApplicableIf element.
    //The MsiXmlBlob is important because the welcome page uses it to list the products it is targetting.
    static CComBSTR GetBlob(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        CComBSTR bstr;
        CString name;        
        CComPtr<IXMLDOMElement> element = ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger);   
        if (element)
        {            
            //Inifinte loop since we will break when there is nothing left to process.
            while(true)
            {
                CComPtr<IXMLDOMNodeList> spNodeList;
                HRESULT hr = element->selectNodes(CComBSTR(L".//ExpressionAlias"), &spNodeList);

                if (SUCCEEDED(hr))
                {
                    long length = 0;
                    spNodeList->get_length(&length);

                    //We have either expanded all ExpressionAlias or there is no ExpressionAlias element, 
                    //either way, exit out of function.
                    if (0 == length)
                    {
                        break;
                    }

                    //For each ExpressionAlias found, expand it.
                    for(long l=0; l<length; ++l)
                    {
                        CComPtr<IXMLDOMNode> node;
                        if (SUCCEEDED(spNodeList->get_item(l, &node)))
                        {
                            CComPtr<IXMLDOMElement> expressionElement = XmlUtils::ExpandExpressionAlias(CComQIPtr<IXMLDOMElement>(node), logger, name);
                            element->replaceChild(expressionElement, node, NULL);
                        }
                    }
                }
                else 
                {
                    CInvalidXmlException ixe(L"schema validation failure:  Failed to Walk the ApplicableIf Nodelist.");
                    LOG(logger, ILogger::Error, ixe.GetMessage());
                    throw ixe;
                }
            }

            element->get_xml(&bstr);        
            if (-1 == CString(bstr).Find(L"<MsiXmlBlob>"))
            {
                CInvalidXmlException ixe(L"schema validation failure:  MsiXmlBlob must exists under the ApplicableIf Element");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }

        return bstr;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::Patches
// Represents a collection of MSP patch files
//------------------------------------------------------------------------------
class Patches 
    : public ItemBase
    , public IgnoreDownloadFailure
    , public RollbackOnPackageInstallFailure
    , public IsPresent    
    , public ActionTable
    , public CustomErrorHandling
    , public HelperItems
{
    ILogger& m_logger;
    CSimpleArray<MSP> m_patches;
public:
    Patches(bool shouldIgnoreDownloadFailure
            , bool shouldRollback
            , ILogger& logger
            , const ActionTable& at
            , const CustomErrorHandling& customErrorHandling = CustomErrorHandling()
            , const HelperItems& helperItems = HelperItems())
        : ItemBase(ItemBase::Patches, 0 /* EstimatedInstallTime not used for Patches just set it to 0 */)
        , IgnoreDownloadFailure(shouldIgnoreDownloadFailure)
        , RollbackOnPackageInstallFailure(shouldRollback)
        , m_logger(logger)
        , IsPresent()
        , ActionTable(at)
        , CustomErrorHandling(customErrorHandling)
        , HelperItems(helperItems)
    {}

    // Copy constructor with layout
    Patches(const Patches& rhs)
        : ItemBase(rhs) 
        , IgnoreDownloadFailure(rhs)
        , IsPresent(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
        , RollbackOnPackageInstallFailure(rhs)
        , HelperItems(rhs)
        , m_logger(rhs.m_logger)
        , m_patches(rhs.m_patches)
    {}

    Patches(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ItemBase(ItemBase::Patches
            , 0 /* EstimatedInstallTime not used for Patches just set it to 0 */
            , ElementUtils::EvaluateToBoolValue(L"PerMachine", false, ElementUtils::GetOptionalAttributeByName(spElement, L"PerMachine", logger), logger)) 
        , IgnoreDownloadFailure(spElement, logger)
        , RollbackOnPackageInstallFailure(spElement, logger)
        , m_logger(logger)
        , IsPresent()
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , CustomErrorHandling(ElementUtils::FindOptionalChildElementByName(spElement, L"CustomErrorHandling", logger), logger)
        , HelperItems(spElement, logger)
    {
        ElementUtils::VerifyName(spElement, L"Patches", logger);

        // Ensure that the item doesn't contain Compressed attributes.
        if (   ElementUtils::ContainsAttribute(spElement, L"Compressed") 
            || ElementUtils::ContainsAttribute(spElement, L"CompressedDownloadSize")
            || ElementUtils::ContainsAttribute(spElement, L"CompressedHashValue"))
        {
            CInvalidXmlException ixe(L"schema validation failure:  Patches does not support Compressed attributes!");            
            throw ixe;
        }

        // Ensure that this setup item does not contain invalid child elements
        if ( ElementUtils::ContainsInvalidChildElementByName(spElement, L"RepairOverride", logger) || ElementUtils::ContainsInvalidChildElementByName(spElement, L"UninstallOverride", logger) )
        {
            CInvalidXmlException ixe(L"schema validation failure:  Patches does not support RepairOverride or UninstallOverride child elements!");            
            throw ixe;
        }

        // Collect all MsiPatch elements so that each MsiXmlBlob for each MSP contains the MsiPatch elements for every patch
        ReplaceMsiXmlBlobsWithGlobalMsiXmlBlob(spElement, logger);

        CComPtr<IXMLDOMNode> spChild;
        
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            do {
                CComPtr<IXMLDOMNode> spSibling = spChild;
                if ((!ElementUtils::IsNodeComment(spSibling)) && (L"MSP" == ElementUtils::GetName(spSibling)))
                {
                    CComPtr<IXMLDOMElement> at = ElementUtils::FindOptionalChildElementByName(CComQIPtr<IXMLDOMElement>(spSibling), L"ActionTable", logger);
                    if (NULL != at)
                    {
                        CInvalidXmlException ixe(L"schema validation failure:  ActionTable element should not be defined in a Patches's MSP");                
                        throw ixe;
                    }

                    CComVariant cv;
                    if (S_OK == CComQIPtr<IXMLDOMElement>(spSibling)->getAttribute(CComBSTR(L"PerMachine"), &cv))
                    {
                        CInvalidXmlException ixe(L"schema validation failure:  PerMachine attribute is valid for Patches element only and not for MSP items under it.");                
                        throw ixe;
                    }

                    MSP patchSibling = MSP(CComQIPtr<IXMLDOMElement>(spSibling), logger, dynamic_cast<const ActionTable&>(*this));                    

                    // Patches can not have individual IgnoreDownloadFailure attribute

                    m_patches.Add(patchSibling);
                    LOG(logger, ILogger::Verbose, L"patch " + CString(patchSibling.GetName()) + L" added");
                }
                spChild = NULL;
                hr = spSibling->get_nextSibling(&spChild);                
                if (S_OK != hr)
                {
                    break;
                }                           
            } while (!!spChild);
        }
    
        if (0 == GetCount())
        {
            CInvalidXmlException ixe(L"No patches found!");            
            throw ixe;
        }
    }
    virtual ~Patches() {}
    virtual ItemBase* Clone() const
    {
        return new Patches(*this);
    }

    //We are ORing all the Patches's MSP's IsPresent element and return true if any item is evaluated to true.
    virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand) const
    {
        bool bResult = false;

        for(int uPatchesCount = 0; uPatchesCount < m_patches.GetSize(); ++uPatchesCount)        
        {
            const IsPresent& ip = dynamic_cast<const IsPresent&>(m_patches[uPatchesCount]);
            bResult |= ip.Evaluate(ipdtoDataToOperand);
        }
        return bResult;
    }

    unsigned int GetCount() const { return m_patches.GetSize(); }
    const MSP& GetMsp(unsigned int index) const
    {
        if (index >= GetCount())
        {
            COutOfBoundsException obe(L"Patch index");
            LOG(m_logger, ILogger::Error, obe.GetMessage());
            throw obe;
        }
        return m_patches[index];
    }
    void AddMsp(const MSP& msp)
    {
        m_patches.Add(msp);     
    }

    const CString GetNames() const
    {
        CString names;
        for(int uPatchesCount = 0; uPatchesCount < m_patches.GetSize(); ++uPatchesCount)
        {
            if (!CString(m_patches[uPatchesCount].GetName()).IsEmpty())
            {
                names += CString(m_patches[uPatchesCount].GetName()) + L";"; 
            }
        }
        return names.TrimRight(L';');
    }

    const CString GetPatchCodes() const
    {
        CString patchCodes;
        for(int uPatchesCount = 0; uPatchesCount < m_patches.GetSize(); ++uPatchesCount)
        {
            patchCodes += m_patches[uPatchesCount].GetPatchCode() + L";"; 
        }
        return patchCodes.TrimRight(L';');
    }

private:
    //This function replaces the every MsiXmlBlob with one that contains all MsiPatch elements under the
    //Patches item. This is needed because the MSPs under a Patches item may be dependent on each other and
    //they may not apply unless they are applied together 
    static void ReplaceMsiXmlBlobsWithGlobalMsiXmlBlob(CComPtr<IXMLDOMElement>& spPatchesElement, ILogger& logger)
    {
        CSimpleArray<CComPtr<IXMLDOMNode> > msiPatchNodeList;
        GetMsiPatchNodeList(spPatchesElement, msiPatchNodeList, logger);

        CComPtr<IXMLDOMNodeList> spMsiXmlBlobList;
        HRESULT hr = spPatchesElement->selectNodes(CComBSTR(L".//MsiXmlBlob"), &spMsiXmlBlobList);
        if (SUCCEEDED(hr))
        {
            long length = 0;
            spMsiXmlBlobList->get_length(&length);
            CComPtr<IXMLDOMNode> nodeFirstMsiXmlBlob;
            for(long l=0; l<length; ++l)
            {
                CComPtr<IXMLDOMNode> nodeMsiXmlBlob;
                if (SUCCEEDED(spMsiXmlBlobList->get_item(l, &nodeMsiXmlBlob)))
                {
                    if ( 0 == l )
                    {
                        // for first MsiXmlBlob, Append all MsiPatches to it
                        // then clone the MsiXmlBlob and use it to replace all others in the Patches Element
                        // append moves the Nodes from where they were to the new location
                        for (int msiPatchIndex = 0; msiPatchIndex < msiPatchNodeList.GetSize(); ++msiPatchIndex)
                        {
                            nodeMsiXmlBlob->appendChild(msiPatchNodeList[msiPatchIndex], NULL);
                        }
                        nodeFirstMsiXmlBlob = nodeMsiXmlBlob;
                    }
                    else
                    {
                        CComPtr<IXMLDOMNode> nodeGlobalMsiXmlBlob;
                        nodeFirstMsiXmlBlob->cloneNode(VARIANT_TRUE, &nodeGlobalMsiXmlBlob);
                        CComPtr<IXMLDOMNode> nodeParent;
                        nodeMsiXmlBlob->get_parentNode(&nodeParent);
                        nodeParent->replaceChild(nodeGlobalMsiXmlBlob, nodeMsiXmlBlob,NULL);
                    }
                }
            }
        }


    }

    //This function will retrieve a NodeList of all the MsiPatch elements that are under the Patches element.
    //This is needed because the MSPs under a Patches item may be dependent on each other and
    //they may not apply unless they are applied together 
    static HRESULT GetMsiPatchNodeList(const CComPtr<IXMLDOMElement>& spPatchesElement
                                        , CSimpleArray<CComPtr<IXMLDOMNode> >& msiPatchNodeList
                                        , ILogger& logger)
    {
        CComPtr<IXMLDOMNodeList> spMsiPatchNodeList;
        HRESULT hr = spPatchesElement->selectNodes(CComBSTR(L".//MsiPatch"), &spMsiPatchNodeList);
        if (SUCCEEDED(hr))
        {
            CSimpleArray<CString> msiPatchTextList;
            long length = 0;
            spMsiPatchNodeList->get_length(&length);
            for(long l=0; l<length; ++l)
            {
                CComPtr<IXMLDOMNode> node;
                if (SUCCEEDED(spMsiPatchNodeList->get_item(l, &node)))
                {
                    BSTR bstr;
                    node->get_xml(&bstr);
                    CString msiPatch(bstr);
                    if ( -1 == msiPatchTextList.Find(msiPatch) )
                    {
                        msiPatchTextList.Add(msiPatch);
                        msiPatchNodeList.Add(node);
                    }
                }
            }
        }
        return hr;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::Exe
// Represents an executable which will be run as part of the engine's 
// Performer actions. If it is being uninstalled we pass in whatever is
// specified in ParameterInfo.xml for the UninstallCommandLine
//------------------------------------------------------------------------------


class Exe 
    : public ExeBase
    , public HelperItems
{
private:
    ExeBase* m_pRepairOverride;
    ExeBase* m_pUninstallOverride;
    // Copy Constructor
    Exe(const Exe& rhs)
        : ExeBase(rhs)
        , HelperItems(rhs)
    {
        m_pRepairOverride = m_pUninstallOverride = NULL;
        if (rhs.m_pRepairOverride)
            m_pRepairOverride = dynamic_cast<ExeBase*>(rhs.m_pRepairOverride->Clone());
        if (rhs.m_pUninstallOverride)
            m_pUninstallOverride = dynamic_cast<ExeBase*>(rhs.m_pUninstallOverride->Clone());
    }

     // With layout path
    Exe(const Exe& rhs, const CPath& layout)
    : ExeBase(rhs, layout)
    , HelperItems(rhs)
    {
        m_pRepairOverride = m_pUninstallOverride = NULL;
        if (rhs.m_pRepairOverride)
            m_pRepairOverride = dynamic_cast<ExeBase*>(rhs.m_pRepairOverride->Twin(layout));
        if (rhs.m_pUninstallOverride)
            m_pUninstallOverride = dynamic_cast<ExeBase*>(rhs.m_pUninstallOverride->Twin(layout));
    }

public:
     Exe(const CString& name
         , const CString& url
         , const CString& installCommandLine
         , const CString& uninstallCommandLine
         , const CString& repairCommandLine
         , ULONGLONG itemSize
         , ULONGLONG systemDriveSize
         , ULONGLONG installedProductSize
         , const CString& canonicalTargetName
         , const bool shouldIgnoreDownloadFailure
         , const bool shouldRollback
         , const CString& logFileHint         
         , ILogger& logger
         , ULONGLONG estimatedInstallTime = 0
         , const ExeTypeEnum& exeType=ExeTypeEnum::GetUnknownType()
         , const ActionTable& at = ActionTable()
         , const HelperItems& helpers = HelperItems()         
         , ExeBase* pRepairOverride = NULL
         , ExeBase* pUninstallOverrideName = NULL
        )
        : ExeBase(name
            , url
            , installCommandLine
            , uninstallCommandLine
            , repairCommandLine
            , itemSize
            , systemDriveSize
            , installedProductSize
            , canonicalTargetName
            , shouldIgnoreDownloadFailure
            , shouldRollback
            , logFileHint
            , logger
            , exeType 
            , at
            , estimatedInstallTime)
        , HelperItems(helpers)
        , m_pRepairOverride(pRepairOverride)
        , m_pUninstallOverride(pUninstallOverrideName)
        {
        }

     // Main constructor
    Exe(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ExeBase(spElement, logger)
        , HelperItems(spElement, logger)
        , m_pRepairOverride(GetOverrideItem(spElement, L"RepairOverride", logger))
        , m_pUninstallOverride(GetOverrideItem(spElement, L"UninstallOverride", logger))

    {
    }

    const ExeBase* GetRepairOverride() const
    {
        return m_pRepairOverride;
    }

    const ExeBase* GetUninstallOverride() const
    {
        return m_pUninstallOverride;
    }

    virtual ItemBase* Clone() const
    {
        return new Exe(*this);
    }

    virtual ItemBase* Twin(const CPath& layout) const
    {
        return new Exe(*this, layout);
    }

    virtual ~Exe()
    {
        delete m_pRepairOverride;
        delete  m_pUninstallOverride;
    }
private:

    // Creates and returns Override ExeBase item if authored. 
    // Validates that only 1 override of given helperNodeName exists.
    static ExeBase* GetOverrideItem(const CComPtr<IXMLDOMElement>& spElement, const CString& helperNodeName, ILogger& logger)
    {
        long helpersCount = ElementUtils::CountChildElements(spElement, helperNodeName);
        if (helpersCount > 1)
        {
            CInvalidXmlException ixe(L"Only one sub item of this type can exist : " + helperNodeName);
            throw ixe;
        }
        ExeBase* pOverrideExe = NULL;
        if (helpersCount) // helpersCount is 1
        {
            CComPtr<IXMLDOMElement> spOverrideHelper = ElementUtils::FindChildElementByName(spElement, helperNodeName, logger);
            CString name = ElementUtils::GetAttributeByName(spOverrideHelper, L"Name", logger, false);
            CString command = ElementUtils::GetOptionalAttributeByName(spOverrideHelper, L"CommandLine", logger);
            ULONGLONG systemDriveSize = ElementUtils::GetOptionalAttributeULONGLONGByName(spOverrideHelper, L"SystemDriveSize", logger);
            ULONGLONG installedProductSize = ElementUtils::GetOptionalAttributeULONGLONGByName(spOverrideHelper, L"InstalledProductSize", logger);

            pOverrideExe = new ExeBase( 
                                  spOverrideHelper
                                , name
                                , command
                                , systemDriveSize
                                , installedProductSize
                                , logger
                                , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
                                , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
                                , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger));
        }
        return pOverrideExe;
    }
};


//------------------------------------------------------------------------------
// Class: IronMan::File
// Represents a file that needs to be acquired, but does not need to be
// directly installed. This might be an external CAB file consumed by an MSI
// or something like a bitmap.
//------------------------------------------------------------------------------


class File : public ItemBase,
             public DownloadPath, 
             public LocalPath, 
             public IsPresent,
             public ApplicableIf,
             public ActionTable,
             public IgnoreDownloadFailure
{
    File(const File& rhs, const CPath& path)
        : ItemBase(ItemBase::File, 0, false, rhs.GetId()) 
        , DownloadPath(rhs)
        , LocalPath(rhs, path)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , IgnoreDownloadFailure(rhs)
    {
    }
public:
    File(const CString& name, const CString& url, ULONGLONG itemSize, const ULONGLONG systemDriveSize, const ULONGLONG installedProductSize, bool shouldIgnoreDownloadFailure)
        : ItemBase(ItemBase::File, 0 /* EstimatedInstallTime not used for File just set it to 0 */) 
        , LocalPath(name, systemDriveSize, installedProductSize, 0)
        , DownloadPath(url, itemSize)
        , IgnoreDownloadFailure(shouldIgnoreDownloadFailure)
    {
    }
    
    File(const File& rhs)
        : ItemBase(rhs)
        , DownloadPath(rhs)
        , LocalPath(rhs)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , IgnoreDownloadFailure(rhs)
    {
    }

    File(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ItemBase(ItemBase::File, spElement, logger)
        , DownloadPath(spElement, logger)
        , LocalPath(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , IgnoreDownloadFailure(spElement, logger)
    {
        ElementUtils::VerifyName(spElement, L"File", logger);
        if (3 != ElementUtils::CountChildElements(spElement))
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of File child nodes!");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
        // Ensure that all compressed items are downlodable and have compressed download size authored.
        bool bItemCompressed = ElementUtils::EvaluateToBoolValue(L"Compressed", false, ElementUtils::GetOptionalAttributeByName(spElement, L"Compressed", logger), logger);
        if (bItemCompressed)
        {
             LocalPath::SetItemCompressedFlag(bItemCompressed);
             if (GetUrl().IsEmpty() || (0 == GetCompressedItemSize()))
             {
                 throw CInvalidXmlException(L"Compressed items need to have URL and CompressedDownloadSize authored.");
             }
        }
    }

    //-------------------------------------------------------------------------
    // Constructor used for File Payloads, don't support child elements
    //-------------------------------------------------------------------------
    File(CComPtr<IXMLDOMElement> spElement, IsPresent& isPresent, ApplicableIf& applicableIf, ActionTable& actionTable, ILogger& logger)
        : ItemBase(ItemBase::File, spElement, 0, logger)
        , DownloadPath(spElement, logger)
        , LocalPath(spElement, logger)
        , IsPresent(isPresent)
        , ApplicableIf(applicableIf)
        , ActionTable(actionTable)
        , IgnoreDownloadFailure(spElement, logger)
    {
        ElementUtils::VerifyName(spElement, L"File", logger);

        // Ensure that all compressed items are downloadable and have compressed download size authored.
        bool bItemCompressed = ElementUtils::EvaluateToBoolValue(L"Compressed", false, ElementUtils::GetOptionalAttributeByName(spElement, L"Compressed", logger), logger);
        if (bItemCompressed)
        {
             LocalPath::SetItemCompressedFlag(bItemCompressed);
        }
    }

    virtual ItemBase* Clone() const
    {
        return new File(*this);
    }
    virtual ItemBase* Twin(const CPath& layout) const
    {
        return new File(*this, layout);
    }

    // Returns true if item is required to perform action
    // MSI, AgileMSI, Msp, Patches etc don't require payload for Repair and Uninstall.
    // Exe, File do require payload to be present.
    virtual bool IsPayloadRequired(Actions installAction) const
    {
        if (Noop == installAction)
            return false;
        return true;
    }
};

class AgileMSP : public MSP
{
public:
    AgileMSP(const AgileMSP& rhs, const CPath& path)
        : MSP(rhs, path)
    { 
        ResetType(ItemBase::AgileMsp); 
    }

public:
    AgileMSP(CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : MSP(spElement, logger, GetIsPresent(spElement, logger), GetApplicableIf(spElement, logger), GetActionTable(spElement, logger), GetIgnoreDownloadFailure(spElement, logger))
    {
        ResetType(ItemBase::AgileMsp); 

        // Ensure that IgnoreDownloadFailure is not authored in as only AgileMSI's attribute value applies to AgileMSPs
        CComVariant cv;
        HRESULT hrIgnoreDownloadFailure = spElement->getAttribute(CComBSTR(L"IgnoreDownloadFailure"), &cv);
        if (S_OK == hrIgnoreDownloadFailure )
        {
            CInvalidXmlException ixe(L"IgnoreDownloadFailure should not be authored for Agile MSPs");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
        
        // Ensure that the item doesn't contain Compressed attributes.
        if (   ElementUtils::ContainsAttribute(spElement, L"Compressed") 
            || ElementUtils::ContainsAttribute(spElement, L"CompressedDownloadSize")
            || ElementUtils::ContainsAttribute(spElement, L"CompressedHashValue"))
        {
            CInvalidXmlException ixe(L"schema validation failure:  AgileMSP does not support Compressed attributes!");            
            throw ixe;
        }
    }

    AgileMSP(const AgileMSP& rhs) 
        : MSP(rhs) 
    { 
        ResetType(ItemBase::AgileMsp); 
    }
    
    AgileMSP(const CString& name, const CString& patchCode, const CString& url, ULONGLONG itemSize, ULONGLONG systemDriveSize, ULONGLONG installedProductSize)
        : MSP(name, patchCode, url, itemSize, systemDriveSize, installedProductSize, false, false, 0 /* EstimatedInstallTime not used for AgileMSP */)
    { 
        ResetType(ItemBase::AgileMsp); 
    }
    
    virtual ~AgileMSP() {}

    virtual AgileMSP* Twin(const CPath& path) const
    {
        return new AgileMSP(*this, path);
    }

private:
    static const IsPresent GetIsPresent(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        return IsPresent(ElementUtils::FindChildElementByName(ElementUtils::GetParentElement(spElement), L"IsPresent", logger), logger);
    }
    static const ApplicableIf GetApplicableIf(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        return ApplicableIf(ElementUtils::FindChildElementByName(ElementUtils::GetParentElement(spElement), L"ApplicableIf", logger), logger);
    }
    static const ActionTable GetActionTable(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        return ActionTable(ElementUtils::FindChildElementByName(ElementUtils::GetParentElement(spElement), L"ActionTable", logger), logger);
    }
    static const IgnoreDownloadFailure GetIgnoreDownloadFailure(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        return IgnoreDownloadFailure(ElementUtils::GetParentElement(spElement), logger);
    }
};

class AgileMSI : public MSI
{
    CSimpleArray<AgileMSP> m_msps;
    ILogger& m_logger;

    AgileMSI(const AgileMSI& rhs, const CPath& path)
        : MSI(rhs, path)
        , m_logger(rhs.m_logger)
    {
        for(int i=0; i<rhs.m_msps.GetSize(); ++i)
        {
            m_msps.Add(AgileMSP(rhs.m_msps[i],path));
        }
        ResetType(ItemBase::AgileMsi);
    }
public:

    AgileMSI(const CString& name
        , const CString& url
        , const CString& uuid
        , const CString& csProductVersion
        , ULONGLONG itemSize
        , ULONGLONG systemDriveSize
        , ULONGLONG installedProductSize
        , const CString& canonicalTargetName
        , const bool shouldIgnoreDownloadFailure
        , const bool shouldRollback
        , const CSimpleArray<AgileMSP>& patches
        , const CString& csMsiOption
        , const CString& csMsiUninstallOptions = CString()
        , const CString& csMsiRepairOptions = CString()
        , ULONGLONG estimatedInstallTime = 0
        , const ActionTable& at = ActionTable()
        , const HelperItems& helpers = HelperItems())
        : MSI(name, url, uuid, csProductVersion, itemSize, systemDriveSize, installedProductSize, canonicalTargetName, shouldIgnoreDownloadFailure, shouldRollback, csMsiOption, csMsiUninstallOptions, csMsiRepairOptions, estimatedInstallTime, at, helpers)
        , m_msps(patches)
        , m_logger(NullLogger::GetNullLogger())
    {
        ResetType(ItemBase::AgileMsi);

    }
    AgileMSI(const AgileMSI& rhs)
        : MSI(rhs)
        , m_msps(rhs.m_msps)
        , m_logger(rhs.m_logger)
    {
        ResetType(ItemBase::AgileMsi);
    }
    AgileMSI(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : MSI(spElement, logger, L"AgileMSI")
        , m_msps(GetAgileMsps(spElement, logger))
        , m_logger(logger)
    {
        ResetType(ItemBase::AgileMsi);

        ElementUtils::VerifyName(spElement, L"AgileMSI", logger);
        unsigned int children = ElementUtils::CountChildElements(spElement);
        if (children < 4)
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of AgileMSI child nodes!");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }

        // Ensure that the item doesn't contain Compressed attributes.
        if (   ElementUtils::ContainsAttribute(spElement, L"Compressed") 
            || ElementUtils::ContainsAttribute(spElement, L"CompressedDownloadSize")
            || ElementUtils::ContainsAttribute(spElement, L"CompressedHashValue"))
        {
            CInvalidXmlException ixe(L"schema validation failure:  AgileMSI does not support Compressed attributes!");            
            throw ixe;
        }
    }
    virtual ItemBase* Clone() const
    {
        return new AgileMSI(*this);
    }
    virtual ItemBase* Twin(const CPath& layout) const
    {
        return new AgileMSI(*this, layout);
    }
    virtual void ResetToBaseType()
    {
        ResetType(ItemBase::Msi);
    }
    unsigned int GetCount() const { return m_msps.GetSize(); }
    const AgileMSP& GetAgileMsp(unsigned int index) const
    {
        if (index >= GetCount())
        {
            COutOfBoundsException obe(L"Agile MSP index");
            LOG(m_logger, ILogger::Error, obe.GetMessage());
            throw obe;
        }
        return m_msps[index];
    }

    void ReplaceAgileMsps(const CSimpleArray<AgileMSP>& msps)
    {
        m_msps.RemoveAll();
        for(int i=0; i<msps.GetSize(); ++i)
        {
            m_msps.Add(msps[i]);
        }
    }
    
    const CSimpleArray<CString> GetMspPackages() const
    {
        CSimpleArray<CString> paths;

        for(int i=0; i<m_msps.GetSize(); ++i)
        {
            paths.Add(m_msps[i].GetName());
        }
        return paths; // TODO: should we use CPaths?
    }

private:
    static CSimpleArray<AgileMSP> GetAgileMsps(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        CSimpleArray<AgileMSP> msps;
        
        long mspCount = 0;
        CComPtr<IXMLDOMNodeList> spChildren = ElementUtils::GetChildElementsByName(spElement, L"AgileMSP", mspCount, logger);
        HRESULT hr;

        for(int i = 0; i < mspCount; i++)
        {
            CComPtr<IXMLDOMNode> spChild;
            hr = spChildren->get_item(i, &spChild);
            if (S_OK == hr)
            {
                AgileMSP agilePatch = AgileMSP(CComQIPtr<IXMLDOMElement>(spChild), logger);
                LOG(logger, ILogger::Verbose, L"agile msp " + CString(agilePatch.GetName()) + L" added");
                msps.Add(agilePatch);
            }
        }
  
        if (0 == msps.GetSize())
        {
            CInvalidXmlException ixe(L"No agile MSPs found!");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }

        return msps;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::ServiceControl
// This class controls services, enables us to start stop, pause and resume them
//------------------------------------------------------------------------------
class ServiceControl : public ItemBase
            , public IsPresent
            , public ApplicableIf
            , public ActionTable
            , public CustomErrorHandling
            , public CanonicalTargetName
            , public HelperItems
{
public:
    class ControlEnum
    {
        enum Control_{ Stop_, Start_, Pause_, Resume_};
        Control_ m_control;

        ControlEnum(Control_ control) : m_control(control){};
    public:
        static const ControlEnum& GetStop  (void) {static const ControlEnum stop  (Stop_  ); return stop; }
        static const ControlEnum& GetStart (void) {static const ControlEnum start (Start_ ); return start; }
        static const ControlEnum& GetPause (void) {static const ControlEnum pause (Pause_ ); return pause; }
        static const ControlEnum& GetResume(void) {static const ControlEnum resume(Resume_); return resume; }

        bool IsControlStop  (void) const {return (m_control == Stop_  );}
        bool IsControlStart (void) const {return (m_control == Start_ );}
        bool IsControlPause (void) const {return (m_control == Pause_ );}
        bool IsControlResume(void) const {return (m_control == Resume_);}

        CString ToString(void)
        {
            if (m_control == Stop_  ) return L"Stop";
            if (m_control == Start_ ) return L"Start";
            if (m_control == Pause_ ) return L"Pause";
            if (m_control == Resume_) return L"Resume";
            return L"Error-Unsupported Contro;";
        }
    private:
        ControlEnum(); // Hide default constructor
    };
private:
    ILogger& m_logger;
    const CString m_name;
    const ControlEnum m_control;

    // Validates that supported Control attributes are passed in and returns appropriate ControlEnum
    static const ControlEnum& GetControlAttribute(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        CString controlAttribute = ElementUtils::GetAttributeByName(spElement, L"Control", logger);
        
        if (controlAttribute == L"Stop"  ) return ControlEnum::GetStop();
        if (controlAttribute == L"Start" ) return ControlEnum::GetStart();
        if (controlAttribute == L"Pause" ) return ControlEnum::GetPause();
        if (controlAttribute == L"Resume") return ControlEnum::GetResume();

        CInvalidXmlException ixe(L"schema validation failure: Only Start, Stop, Pause and Resumeare supported for 'Control' attribute.");
        throw ixe;
    }

public:
    // Constructor for testing
    ServiceControl(const CString& serviceName, const ControlEnum& serviceControl, const CString& canonicalName, ILogger& logger, ULONGLONG estimatedInstallTime=0)
        : ItemBase(ItemBase::ServiceControl, estimatedInstallTime)
        , CanonicalTargetName(canonicalName)
        , m_name(serviceName)
        , m_control(serviceControl)
        , m_logger(logger)
    {}
    // Constructor
    ServiceControl(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ItemBase(ItemBase::ServiceControl, 
                   ElementUtils::GetOptionalAttributeULONGLONGByName(spElement, L"EstimatedInstallTime", logger))
        , CanonicalTargetName(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , CustomErrorHandling(ElementUtils::FindOptionalChildElementByName(spElement, L"CustomErrorHandling", logger), logger)
        , HelperItems(spElement, logger)
        , m_name(ElementUtils::GetAttributeByName(spElement, L"Name", logger))
        , m_control(GetControlAttribute(spElement, logger))
        , m_logger(logger)
    {
        // TODO Spec Issue - validate that service by name m_name exists? and throw if not? is that valid for /CreateLayout

        // Ensure that this setup item does not contain invalid child elements
        if ( ElementUtils::ContainsInvalidChildElementByName(spElement, L"RepairOverride", logger) || ElementUtils::ContainsInvalidChildElementByName(spElement, L"UninstallOverride", logger) )
        {
            CInvalidXmlException ixe(L"schema validation failure:  ServiceControl does not support RepairOverride or UninstallOverride child elements!");            
            throw ixe;
        }

        // Ensure that the item doesn't contain Compressed attributes.
        if (   ElementUtils::ContainsAttribute(spElement, L"Compressed") 
            || ElementUtils::ContainsAttribute(spElement, L"CompressedDownloadSize")
            || ElementUtils::ContainsAttribute(spElement, L"CompressedHashValue"))
        {
            CInvalidXmlException ixe(L"schema validation failure:  ServiceControl does not support Compressed attributes!");            
            throw ixe;
        }

    }

    ServiceControl(const ServiceControl& rhs)
        : ItemBase(rhs)
        , CanonicalTargetName(rhs)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
        , HelperItems(rhs)
        , m_name(rhs.m_name)
        , m_control(rhs.m_control)
        , m_logger(rhs.m_logger)
    {}


    const CString& GetServiceName(void) const { return m_name; };
    const ControlEnum& GetServiceControl(void) const { return m_control; };

    virtual ~ServiceControl() {}
    virtual ItemBase* Clone() const 
    { 
        return new ServiceControl(*this); 
    }

    // Returns true if item is required to perform action
    // MSI, AgileMSI, Msp, Patches etc don't require payload for Repair and Uninstall.
    // Exe, File do require payload to be present.
    virtual bool IsPayloadRequired(Actions installAction) const
    {
        return false;
    }

};


//------------------------------------------------------------------------------
// Class: IronMan::CleanupBlock
// Supports arbitrary number of following actions, all based on applicability
// of this item:
//      RemovePatch
//      RemoveProduct
//      UnadvertiseFeatures
//------------------------------------------------------------------------------

class CleanupBlock : public ItemBase
            , public IsPresent
            , public ApplicableIf
            , public ActionTable
            , public CanonicalTargetName
{
    ULONGLONG m_totalInstallSize;
    const CSimpleArray<CString> m_patchCodesToRemove;
    const CSimpleArray<CString> m_productCodesToUnAdvertise;
    const CSimpleArray<CString> m_productCodesToRemove;
    mutable CSimpleArray<CString> m_affectedProducts;
    mutable bool m_evaulatedAffectedProducts;
    CString m_nameOfCleanupBlock;
    const bool m_bDoUnAdvertiseFeaturesOnRemovePatch;

    mutable CSimpleMap<CString, CString> m_productToPatchesMapping; // patches are semi-colon delimited
public:

    CleanupBlock(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ItemBase(ItemBase::CleanupBlockType, 0 /* EstimatedInstallTime not used for CleanupBlock */)
        , CanonicalTargetName(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , m_nameOfCleanupBlock(ElementUtils::GetOptionalAttributeByName(spElement, L"Name", logger))
        , m_patchCodesToRemove       (GetMSIGUIDs(spElement, logger, L"RemovePatch",         L"PatchCode"))
        , m_productCodesToUnAdvertise(GetMSIGUIDs(spElement, logger, L"UnAdvertiseFeatures", L"ProductCode"))
        , m_productCodesToRemove     (GetMSIGUIDs(spElement, logger, L"RemoveProduct",       L"ProductCode"))
        , m_totalInstallSize(ElementUtils::GetAttributeULONGLONGByName(spElement, L"InstalledProductSize", logger))
        , m_evaulatedAffectedProducts(false)
        , m_bDoUnAdvertiseFeaturesOnRemovePatch(ElementUtils::EvaluateToBoolValue(L"DoUnAdvertiseFeaturesOnRemovePatch", true, ElementUtils::GetOptionalAttributeByName(spElement, L"DoUnAdvertiseFeaturesOnRemovePatch", logger), logger))
    {
        ElementUtils::VerifyName(spElement, L"CleanupBlock", logger);

        // Ensure that the item doesn't contain Compressed attributes.
        if (   ElementUtils::ContainsAttribute(spElement, L"Compressed") 
            || ElementUtils::ContainsAttribute(spElement, L"CompressedDownloadSize")
            || ElementUtils::ContainsAttribute(spElement, L"CompressedHashValue"))
        {
            CInvalidXmlException ixe(L"schema validation failure:  CleanupBlock does not support Compressed attributes!");            
            throw ixe;
        }

        if (m_nameOfCleanupBlock.IsEmpty())
        {
            m_nameOfCleanupBlock = L"CleanupBlock";
        }

        int cleanupElementsCount = m_patchCodesToRemove.GetSize() + m_productCodesToUnAdvertise.GetSize() + m_productCodesToRemove.GetSize();
        if (cleanupElementsCount == 0)
        {
            CInvalidXmlException ixe(L"schema validation failure:  no CleanupBlock child elements authored to be cleaned up. Valid elements are RemovePatch, UnAdvertiseFeatures or RemoveProduct!");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }

        // Verifies that only "install" and "noop" are authored for three modes.
        VerifyAction(GetInstallAction(true),    GetInstallAction(false));
        VerifyAction(GetRepairAction(true),     GetRepairAction(false));
        VerifyAction(GetUninstallAction(true),  GetUninstallAction(false));
    }

    // Unit test constructor
    CleanupBlock(const CSimpleArray<CString>& patchCodesToRemove
                , ULONGLONG totalInstallSize
                , const CSimpleArray<CString>& productCodesToUnAdvertise
                , const CSimpleArray<CString>& productCodesToRemove
                , const CString& canonicalName
                , const bool bDoUnAdvertiseFeaturesOnRemovePatch
                )
        : ItemBase(ItemBase::CleanupBlockType, 0 /* EstimatedInstallTime not used for CleanupBlock */)
        , CanonicalTargetName(canonicalName)
        , m_totalInstallSize(totalInstallSize)
        , m_patchCodesToRemove(patchCodesToRemove)
        , m_productCodesToUnAdvertise(productCodesToUnAdvertise)
        , m_productCodesToRemove(productCodesToRemove)
        , m_bDoUnAdvertiseFeaturesOnRemovePatch(bDoUnAdvertiseFeaturesOnRemovePatch)
    {}

    // Copy Constructor
    CleanupBlock(const CleanupBlock& rhs)
        : ItemBase(ItemBase::CleanupBlockType, 0 /* EstimatedInstallTime not used for CleanupBlock */)
        , CanonicalTargetName(rhs)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , m_nameOfCleanupBlock(rhs.m_nameOfCleanupBlock)
        , m_totalInstallSize(rhs.m_totalInstallSize)
        , m_patchCodesToRemove(rhs.m_patchCodesToRemove)
        , m_productCodesToUnAdvertise(rhs.m_productCodesToUnAdvertise)
        , m_productCodesToRemove(rhs.m_productCodesToRemove)
        , m_evaulatedAffectedProducts(rhs.m_evaulatedAffectedProducts)
        , m_affectedProducts(rhs.m_affectedProducts)
        , m_productToPatchesMapping(rhs.m_productToPatchesMapping)
        , m_bDoUnAdvertiseFeaturesOnRemovePatch(rhs.m_bDoUnAdvertiseFeaturesOnRemovePatch)
    {}


    // ActionTable virtual method
    virtual bool IsPayloadRequired(Actions installAction) const
    {
        return false;
    }

    // ItemBase Virtual method.
    virtual ItemBase* Clone() const
    {
        return new CleanupBlock(*this);
    }
    
    // Returns Semicolon separated product codes to Unadvertise.
    const CSimpleArray<CString>& GetProductCodesToUnAdvertise() const
    {
        return m_productCodesToUnAdvertise;
    }

    // Returns Semicolon separated product codes to Remove.
    const CSimpleArray<CString>& GetProductCodesToRemove() const
    {
        return m_productCodesToRemove;
    }

    const CString GetCleanupBlockName() const
    {
        return m_nameOfCleanupBlock;
    }
    
    ULONGLONG GetInstallSize(void) const
    {
        return m_totalInstallSize;
    }

    bool DoUnAdvertiseFeaturesOnRemovePatch(void) const
    {
        return m_bDoUnAdvertiseFeaturesOnRemovePatch;
    }

    // List of product codes that will be operated on, when CleanupBlock item is performed.
    // This is a combined list for all the operations, including RemovePatches, RemoveProducts and UnadvertiseFeatures.
    const CSimpleArray<CString> GetAffectedProducts() const
    {
        if (m_evaulatedAffectedProducts)
            return m_affectedProducts;

        UINT iProduct = 0;
        while(true) // enum every product on the box
        {
            WCHAR szProductCode[MAX_GUID_CHARS+1] = {0};
            UINT iRes = MsiEnumProductsEx(NULL, NULL, MSIINSTALLCONTEXT_MACHINE, iProduct++, szProductCode, NULL, NULL, NULL);
            if (ERROR_SUCCESS != iRes)
                break;

            // if the patch applies to this product, add the path to the local cache to the 
            // m_productToPatchesMapping map
            for(int i=0; i<m_patchCodesToRemove.GetSize(); ++i) // check each patch code against this product
            {
                CString localPackage;
                DWORD cchLocalPackage = 0;
                iRes = MsiGetPatchInfoEx(m_patchCodesToRemove[i]
                                        , szProductCode, NULL
                                        , MSIINSTALLCONTEXT_MACHINE
                                        , INSTALLPROPERTY_LOCALPACKAGE
                                        , localPackage.GetBuffer(cchLocalPackage)
                                        , &cchLocalPackage);
                localPackage._ReleaseBuffer();
                if (ERROR_MORE_DATA == iRes)
                {
                    cchLocalPackage += 1;  // Add 1 for the null teminator
                    iRes = MsiGetPatchInfoEx(m_patchCodesToRemove[i]
                                            , szProductCode, NULL
                                            , MSIINSTALLCONTEXT_MACHINE
                                            , INSTALLPROPERTY_LOCALPACKAGE
                                            , localPackage.GetBuffer(cchLocalPackage)
                                            , &cchLocalPackage);
                    localPackage._ReleaseBuffer();
                    if (ERROR_SUCCESS == iRes)
                    {
                        // Patch applies to this product, add local cache path to the map
                        int index = m_productToPatchesMapping.FindKey(szProductCode);
                        if (-1 == index)
                        {
                            m_productToPatchesMapping.Add(szProductCode, localPackage);
                            m_affectedProducts.Add(szProductCode);
                        }
                        else
                        {
                            CString patches = m_productToPatchesMapping.GetValueAt(index);
                            patches += L";" + localPackage;
                            m_productToPatchesMapping.SetAtIndex(index, szProductCode, patches);
                        }
                    }
                }
            }
        }

        for (int i = 0; i < m_productCodesToUnAdvertise.GetSize(); ++i)
        {
            if (-1 == m_affectedProducts.Find(m_productCodesToUnAdvertise[i]))
                m_affectedProducts.Add(m_productCodesToUnAdvertise[i]);
        }

        for (int i = 0; i < m_productCodesToRemove.GetSize(); ++i)
        {
            if (-1 == m_affectedProducts.Find(m_productCodesToRemove[i]))
                m_affectedProducts.Add(m_productCodesToRemove[i]);
        }

        m_evaulatedAffectedProducts = true;

        return m_affectedProducts;
    }

    
    // Updates the map with calculated map contents.
    void GetProductsToPatchesMapping(CSimpleMap<CString, CString>& productToPatchesMapping) const
    {
        if (!m_evaulatedAffectedProducts)
            GetAffectedProducts(); // Compute affected products if not already done (passive and silent mode).
        for(int i = 0; i < m_productToPatchesMapping.m_nSize; i++ )
        {
            productToPatchesMapping.Add(m_productToPatchesMapping.GetKeyAt(i), m_productToPatchesMapping.GetValueAt(i));
        }
    }

private:
    // Helper method to read ProductCodes from elements like: 
    // <RemovePatch         PatchCode  ="{D5AFCF56-F3DD-46C1-B2D0-16EA8B49C9E4}"/>
    // <RemoveProduct       ProductCode="{D5AFCF56-F3DD-46C1-B2D0-16EA8B49C9E4}"/>
    // <UnAdvertiseFeatures ProductCode="{D5AFCF56-F3DD-46C1-B2D0-16EA8B49C9E4}"/>
    // Returns list of all authored guids, for a given element.

    static CSimpleArray<CString> GetMSIGUIDs(CComPtr<IXMLDOMElement>& spElement, ILogger& logger, const CString& elementName, const CString& attributeName)
    {
        CSimpleArray<CString> msiGUIDS;

        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            if (ElementUtils::GetName(spChild) == elementName) // don't try to add ApplicableIf
                msiGUIDS.Add(ElementUtils::GetAttributeByName(CComQIPtr<IXMLDOMElement>(spChild), attributeName, logger));
            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    if (ElementUtils::GetName(spSibling) == elementName) // don't try to add ApplicableIf
                        msiGUIDS.Add(ElementUtils::GetAttributeByName(CComQIPtr<IXMLDOMElement>(spSibling), attributeName, logger));
                }
                spChild = spSibling;
            } while (!!spChild);
        }

        return msiGUIDS;
    };
    
    // Verifies if install action is just "install" and "noop".
    // "repair" and "uninstall" are not supported for CleanupBlock.
    void VerifyAction(const Actions& ifPresentAction, const Actions& ifAbsentAction) const
    {
        bool invalid =  (ifPresentAction != ActionTable::Install&& ifPresentAction != ActionTable::Noop)
                     || (ifAbsentAction  != ActionTable::Install&& ifAbsentAction  != ActionTable::Noop);
        if (invalid)
        {
            CInvalidXmlException ixe(L"schema validation failure:  Only 'install' and 'noop' are valid actions for CleanupBlock.");
            throw ixe;
        }
    }
};

// This class lets you specify UpgradeCode as Relation attribute and gives out 
// list of all ProductCodes that this UpgradeCode 
class RelatedProducts
            : public ItemBase
            , public IsPresent
            , public ApplicableIf
            , public ActionTable
            , public CanonicalTargetName
            , public CustomErrorHandling
            , public HelperItems
{
    ULONGLONG m_totalInstallSize;
    CString m_nameOfRelatedProducts;
    const CSimpleArray<CString> m_relatedProducts;
    const CString m_csMsiUninstallOptions; // Uninstall options to pass thru when making MSI call. 
    const CString m_csMsiRepairOptions; // Repair options to pass thru when making MSI call

public:

    // Main Constructor
    RelatedProducts(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : ItemBase(ItemBase::RelatedProductsType, spElement, logger)
        , CanonicalTargetName(spElement, logger)
        , IsPresent(ElementUtils::FindChildElementByName(spElement, L"IsPresent", logger), logger)
        , ApplicableIf(ElementUtils::FindChildElementByName(spElement, L"ApplicableIf", logger), logger)
        , ActionTable(ElementUtils::FindChildElementByName(spElement, L"ActionTable", logger), logger)
        , HelperItems(spElement, logger)
        , CustomErrorHandling(ElementUtils::FindOptionalChildElementByName(spElement, L"CustomErrorHandling", logger), logger)
        , m_nameOfRelatedProducts(ElementUtils::GetOptionalAttributeByName(spElement, L"Name", logger))
        , m_csMsiUninstallOptions(ElementUtils::GetOptionalAttributeByName(spElement, L"MSIUninstallOptions", logger, true))
        , m_csMsiRepairOptions(ElementUtils::GetOptionalAttributeByName(spElement, L"MSIRepairOptions", logger, true))
        , m_relatedProducts(EnumerateRelatedProducts(spElement, m_nameOfRelatedProducts, logger))
    {
        ElementUtils::VerifyName(spElement, L"RelatedProducts", logger);

        if (m_nameOfRelatedProducts.IsEmpty())
        {
            m_nameOfRelatedProducts = L"RelatedProducts";
        }

        VerifyActionTable();
    }

    // UnitTests Constructor
    RelatedProducts(const CString& name, const CSimpleArray<CString>& relatedProudcts, ILogger& logger, const CString& csMsiUninstallOptions = CString(), const CString& csMsiRepairOptions = CString())
        : m_nameOfRelatedProducts(name)
        , m_relatedProducts(relatedProudcts)
        , ItemBase(ItemBase::RelatedProductsType, 0)
        , CanonicalTargetName(name)
        , m_csMsiUninstallOptions(csMsiUninstallOptions)
        , m_csMsiRepairOptions(csMsiRepairOptions)
    {
    }

    // Copy Constructor
    RelatedProducts(const RelatedProducts& rhs)
        : ItemBase(rhs)
        , CanonicalTargetName(rhs)
        , IsPresent(rhs)
        , ApplicableIf(rhs)
        , ActionTable(rhs)
        , CustomErrorHandling(rhs)
        , HelperItems(rhs)
        , m_nameOfRelatedProducts(rhs.m_nameOfRelatedProducts)
        , m_relatedProducts(rhs.m_relatedProducts)
        , m_csMsiUninstallOptions(rhs.m_csMsiUninstallOptions)
        , m_csMsiRepairOptions(rhs.m_csMsiRepairOptions)
    {}

    // List of product codes that will be operated on, when CleanupBlock item is performed.
    // This is a combined list for all the operations, including RemovePatches, RemoveProducts and UnadvertiseFeatures.
    const CSimpleArray<CString> GetRelatedProducts() const
    {
        return m_relatedProducts;
    }

    // Returns the name of the 
    const CString GetName() const
    {
        return m_nameOfRelatedProducts;
    }

    // ItemBase Virtual method.
    virtual ItemBase* Clone() const
    {
        return new RelatedProducts(*this);
    }

    const CString& GetMsiUninstallOptions() const
    {
        return m_csMsiUninstallOptions; 
    }
    const CString& GetMsiRepairOptions() const 
    { 
        return m_csMsiRepairOptions; 
    }
private:
    // Iterates over all Relation elements and gets list of product codes affected by authored Upgradecodes, with no duplicates.
    static CSimpleArray<CString> EnumerateRelatedProducts(CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger)
    {
        CSimpleArray<CString> productCodes;

        long countRelations= 0;
        CComPtr<IXMLDOMNodeList> relationList = ElementUtils::GetChildElementsByName(spElement, L"Relation", countRelations, logger);

        for (int j = 0; j < countRelations; j++)
        {
            CComPtr<IXMLDOMNode> spRelation;
            HRESULT hrRelation = relationList->get_item(j, &spRelation);
            if (S_OK == hrRelation)
            {
                CComPtr<IXMLDOMElement> relation =  CComQIPtr<IXMLDOMElement>(spRelation);
                CString upgradeCode = ElementUtils::GetAttributeByName(relation, L"UpgradeCode", logger, true);
                CString minVersion = ElementUtils::GetOptionalAttributeByName(relation, L"VersionMin", logger, true);
                CString maxVersion = ElementUtils::GetOptionalAttributeByName(relation, L"VersionMax", logger, true);
                bool minVersionInclusive = ElementUtils::EvaluateToBoolValue(L"VersionMinInclusive", false, ElementUtils::GetOptionalAttributeByName(relation, L"VersionMinInclusive", logger), logger);
                bool maxVersionInclusive = ElementUtils::EvaluateToBoolValue(L"VersionMaxInclusive", false, ElementUtils::GetOptionalAttributeByName(relation, L"VersionMaxInclusive", logger), logger);

                // Get all the product codes to skip
                CSimpleArray<CString> productCodesToSkip;
                long countProductsToSkip = 0;
                countProductsToSkip = ElementUtils::CountChildElements(relation, L"SkipProduct");
                if (0 < countProductsToSkip)
                {
                    CComPtr<IXMLDOMNodeList> skipProducts = ElementUtils::GetChildElementsByName(relation, L"SkipProduct", countProductsToSkip, logger);

                    for (int i = 0; i < countProductsToSkip; i++)
                    {
                        CComPtr<IXMLDOMNode> spChild;
                        HRESULT hr = skipProducts->get_item(i, &spChild);
                        if (S_OK == hr)
                        {
                            CString productCodeToSkip = ElementUtils::GetAttributeByName(CComQIPtr<IXMLDOMElement>(spChild), L"ProductCode", logger, true);
                            productCodesToSkip.Add(productCodeToSkip);
                        }
                    }
                }

                // Get all product codes related to upgradecode after skipping given products codes to skip.
                int countProductCodes = MSIUtils::GetRelatedProducts(upgradeCode, productCodesToSkip, minVersion, minVersionInclusive, maxVersion, maxVersionInclusive, logger, productCodes);
                CString logLine;
                logLine.Format(L"For upgradecode %s, [%d] related products were found.", upgradeCode, countProductCodes);
                LOG(logger, ILogger::Information, logLine);
            }
        }
        CString logLine;
        logLine.Format(L"RelatedProducts item %s has %d related products.", name, productCodes.GetSize());
        LOG(logger, ILogger::Information, logLine);
        return productCodes;
    }

    // Verifies that "install" action is not authored in the action table as it is not supported.
    const void VerifyActionTable() const
    {
        bool invalid =     GetInstallAction(true)   == ActionTable::Install   || GetInstallAction(false)   == ActionTable::Install
                        || GetRepairAction(true)    == ActionTable::Install   || GetRepairAction(false)    == ActionTable::Install
                        || GetUninstallAction(true) == ActionTable::Install   || GetUninstallAction(false) == ActionTable::Install;
        if (invalid)
        {
            CInvalidXmlException ixe(L"schema validation failure:  Install action is not supported in the ActionTable for RelatedProducts.");
            throw ixe;
        }
    }
};



class Items
{
private:
    FailureActionEnum m_failureAction; 
    DWORD m_dwRetry;
    DWORD m_dwDelay;
    bool m_bShouldCopyPackageFile;
    ILogger& m_logger;

    ItemArray m_items;
    ItemArray m_childFileItems;

    static const FailureActionEnum& GetSubFailureAction(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        const CString& subFailureAction = ElementUtils::GetOptionalAttributeByName(spElement, L"OnSubFailureAction", logger, true);

        if (subFailureAction == L"Rollback")
            return FailureActionEnum::GetRollbackAction();
        if (subFailureAction == L"Stop")
            return FailureActionEnum::GetStopAction();
        if (subFailureAction == L"Continue")
            return FailureActionEnum::GetContinueAction();
        if (subFailureAction == L"")    // Default action if nothing is specified
            return FailureActionEnum::GetRollbackAction(false);

        CInvalidXmlException ixe(L"schema validation failure:  invalid attribute value for - OnSubFailureAction");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }
    
public:
    Items(DWORD dwRetry, DWORD dwDelay, bool bShouldCopyPackageFile, ILogger& logger, const FailureActionEnum& failureAction = FailureActionEnum::GetRollbackAction(false))
        : m_logger(logger)
        , m_failureAction(failureAction)
        , m_dwRetry(dwRetry)   
        , m_dwDelay(dwDelay)
        , m_bShouldCopyPackageFile(bShouldCopyPackageFile)
    {}

    Items(const CSimpleArray<ItemBase*>& items, DWORD dwRetry, DWORD dwDelay, bool bShouldCopyPackageFile, ILogger& logger)
        : m_logger(logger)
        , m_failureAction(FailureActionEnum::GetRollbackAction(false))
        , m_items(items)
        , m_dwRetry(dwRetry)   
        , m_dwDelay(dwDelay)
        , m_bShouldCopyPackageFile(bShouldCopyPackageFile)
    {}

    Items(const Items& rhs)
        : m_logger(rhs.m_logger)
        , m_failureAction(rhs.m_failureAction)
        , m_items(rhs.m_items)
        , m_childFileItems(rhs.m_childFileItems)
        , m_dwRetry(rhs.m_dwRetry)
        , m_dwDelay(rhs.m_dwDelay)
        , m_bShouldCopyPackageFile(rhs.m_bShouldCopyPackageFile)
    {}

    Items(CComPtr<IXMLDOMElement> spElement, ILogger& logger) 
        : m_logger(logger)
        , m_failureAction(Items::GetSubFailureAction(spElement, logger))
        , m_dwRetry(ElementUtils::GetOptionalAttributeDwordByName(spElement, L"DownloadRetries", 3, logger))
        , m_dwDelay(ElementUtils::GetOptionalAttributeDwordByName(spElement, L"DelayBetweenRetries", 1, logger))
        , m_bShouldCopyPackageFile(ComputeCopyPackageFlag(ElementUtils::GetOptionalAttributeByName(spElement, L"CopyPackageFilesToDownloadLocation", logger))) 
    {
        ElementUtils::VerifyName(spElement, L"Items", logger);

        CreateChildFileItems(spElement, m_childFileItems, logger);

        CComPtr<IXMLDOMNode> spChild;

        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            if (!ElementUtils::IsNodeComment(spChild))
            {
                ItemBase* itemNext = MakeItem(CComQIPtr<IXMLDOMElement>(spChild),logger);
                m_items.Add(itemNext);
            }

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    if (!ElementUtils::IsNodeComment(spSibling))
                    {
                        ItemBase* itemSibling = MakeItem(CComQIPtr<IXMLDOMElement>(spSibling),logger);
                        m_items.Add(itemSibling);
                    }
                }

                spChild = spSibling;
            } while (!!spChild);
        }
        if (0 == GetCount())
        {
            CInvalidXmlException ixe(L"No items found. The package must contain at least one item.");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }
    virtual ~Items() {}

    unsigned int GetCount() const
    {
        return m_items.GetSize();
    }

    // Returns count of child items authored to all items.
    unsigned int GetChildItemCount() const
    {
        return m_childFileItems.GetSize();
    }

    // Returns Child Item Array
    const ItemArray& GetChildItems() const
    {
        return m_childFileItems;
    }

    // Returns count of items authored of the given type.
    unsigned int GetCount(ItemBase::ItemType itemType) const
    {
        unsigned int count = 0;
        for (int index = 0; index < m_items.GetSize(); index++)
        {
            if (itemType == ((ItemBase *)m_items[index])->GetType())
                count++;
        }
        return count;
    }

    const ItemBase* GetItem(unsigned int index) const
    {
        return GetItemPrivate(index);
    }

    const ItemBase* GetChildItem(unsigned int index) const
    {
        return GetChildItemPrivate(index);
    }

    ItemBase* GetItem(unsigned int index)
    {
        return GetItemPrivate(index);
    }


    ItemBase* GetChildItem(unsigned int index)
    {
        return GetChildItemPrivate(index);
    }

    const ItemBase* GetItem(const CString& csItemId) const
    {
        return GetItemPrivate(csItemId);
    }

    // Returns count of child items authored to an item
    unsigned int GetChildItemCount(unsigned int nParentIndex) const
    {
        return m_items[nParentIndex]->GetReferencedFiles().GetSize();
    }

    ItemBase* GetChildItem(unsigned int nParentIndex, unsigned int nChildIndex)
    {
        CString childFileId = m_items[nParentIndex]->GetReferencedFiles()[nChildIndex];
        return GetItemPrivate(childFileId);
    }

    const FailureActionEnum GetFailureAction() const
    {
        return m_failureAction;
    }

    const DWORD GetRetries() const
    {
        return m_dwRetry;
    }

    const DWORD GetDelay() const
    {
        return m_dwDelay;
    }

    //Determine if we need to copy files.
        const bool GetCopyPackageFilesFlag() const
    {
        return m_bShouldCopyPackageFile;
    }
    
    void Add(ItemBase* pItem)
    {
        m_items.Add(pItem);
    }

    void AddChildItem(ItemBase* pItem)
    {
        m_childFileItems.Add(pItem);
    }

private:
    ItemBase* GetItemPrivate(unsigned int index) const
    {
        if (index >= GetCount())
        {
            COutOfBoundsException obe(L"Item index");
            LOG(m_logger, ILogger::Error, obe.GetMessage());
            throw obe;
        }
        return m_items[index];
    }

    ItemBase* GetChildItemPrivate(unsigned int index) const
    {
        if (index >= m_childFileItems.GetSize())
        {
            COutOfBoundsException obe(L"Child Item index");
            LOG(m_logger, ILogger::Error, obe.GetMessage());
            throw obe;
        }
        return m_childFileItems[index];
    }

    //------------------------------------------------------------------------------
    // Function: IronMan::Items::GetItemPrivate
    // Returns item that matches the Item Id, includes parent and children
    //------------------------------------------------------------------------------
    ItemBase* GetItemPrivate(const CString& csItemId) const
    {
        int index = 0;

        // Child elements of Items
        for( index = 0; index < GetCount(); ++index )
        {
            if ( m_items[index]->GetId() == csItemId )
            {
                return m_items[index];
            }
        }
        // GrandChild File elements of Items
        for( index = 0; index < m_childFileItems.GetSize(); ++index )
        {
            if ( m_childFileItems[index]->GetId() == csItemId )
            {
                return m_childFileItems[index];
            }
        }
        CNotFoundException nfe(L"Item id," + csItemId + L",");
        LOG(m_logger, ILogger::Error, nfe.GetMessage());
        throw nfe;
    }
private:

    //Compute the CopyPackageFilesToDownload flag.  The value is case in-sensitive.
    static bool ComputeCopyPackageFlag(const CString CopyPackageFilesToDownloadLocationAttributeValue)
    {
        bool bResult = false;
        //If the value is not specified, it is assume to be true.
        if (0 == CopyPackageFilesToDownloadLocationAttributeValue.CompareNoCase(L"true"))            
        {
            bResult = true;
        }
        return bResult;
    }


    //------------------------------------------------------------------------------
    // Function: IronMan::Items::CreateChildFileItems
    // Finds all File elements that are grandchildren of the Items element and
    // uses them to fill the childFileItems ItemArray
    //------------------------------------------------------------------------------
    static void CreateChildFileItems(CComPtr<IXMLDOMElement> spItems, ItemArray& childFileItems, ILogger& logger)
    {
        CComPtr<IXMLDOMNodeList> spNodeList;
        HRESULT hr = spItems->selectNodes(CComBSTR(L"./*/File"), &spNodeList);
        if (SUCCEEDED(hr))
        {
            LONG length;
            spNodeList->get_length(&length);
            for(long l=0; l<length; ++l)
            {
                CComPtr<IXMLDOMNode> node;
                if (SUCCEEDED(spNodeList->get_item(l, &node)))
                {
                    // IsPresent, ApplicableIf and ActionTable will not be used for File Payloads
                    ItemBase* pItem = new File(CComQIPtr<IXMLDOMElement>(node), IsPresent(), ApplicableIf(), ActionTable(), logger);
                    childFileItems.Add(pItem);
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    // Function: IronMan::Items::MakeItem
    // Knows how to create the appropriate item based on the element name in
    // the ParameterInfo.xml input
    //------------------------------------------------------------------------------
    static ItemBase* MakeItem(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        CString type = ElementUtils::GetName(spElement);
        
        ItemBase* pItem = NULL;
        if (type == L"MSI")
        {
            pItem = new MSI(spElement, logger);
        } 
        else if (type == L"AgileMSI")
        {
            pItem = new AgileMSI(spElement, logger);
        }
        else if (type == L"MSP")
        {
            pItem = new MSP(spElement, logger);
        }
        else if (type == L"Exe")
        {
            pItem = new Exe(spElement, logger);
        }
        else if (type == L"Patches")
        {
            pItem = new Patches(spElement, logger);
        }
        else if (type == L"File")
        {
            pItem = new File(spElement, logger);
        }
        else if (type == L"ServiceControl")
        {
            pItem = new ServiceControl(spElement, logger);
        }
        else if (type == L"CleanupBlock")
        {
            pItem = new CleanupBlock(spElement, logger);
        }
        else if (type == L"RelatedProducts")
        {
            pItem = new RelatedProducts(spElement, logger);
        }
        else
        {
            LOG(logger, ILogger::Error, L"Unknown Item type \"" + type + L"\". Valid types are MSI, MSP, Exe, Patches, ServiceControl and File. Theses are case sensitive.");
            CInvalidXmlException ixe(L"schema validation failure:  unknown Item type - " + type);
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }

        CString localPathName(L"(not applicable)");
        const LocalPath* lp = dynamic_cast<const LocalPath*>(pItem);
        if (lp)
        {
            localPathName = CString(lp->GetName());
        }
        LOG(logger, ILogger::Information, L"Adding Item type \"" + type + L"\", local path " + localPathName );
        return pItem;
    }
};

class Ui
{
    CString m_dll;
    CString m_name;  //Name of KB article, or Product
    CString m_version;  //Version of the Product

public:
    Ui(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_dll(ElementUtils::GetAttributeByName(spElement, L"Dll", logger))
        , m_name(ElementUtils::GetAttributeByName    (spElement, L"Name"       , logger))
        , m_version(ElementUtils::GetAttributeByName(spElement, L"Version", logger))
    {
        ElementUtils::VerifyName(spElement, L"UI", logger);
        if (0 != ElementUtils::CountChildElements(spElement))
        {
            CInvalidXmlException ixe(L"schema validation failure:  wrong number of UI child nodes!");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }
    virtual ~Ui() {}

    const CPath GetDll() const { return CPath(m_dll); }
    const CString GetName() const { return m_name; }
    const CString GetVersion() const { return m_version; }
};

//------------------------------------------------------------------------------
// Class: IronMan::ProcessBlock
// Contains the data needed to do a system check for one process
// represents the ProcessBlock element in the ParameterInfo.xml
//------------------------------------------------------------------------------
class ProcessBlock
{
    ILogger& m_logger;
    CString m_imageName;
public:
    ProcessBlock(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_imageName(ElementUtils::GetAttributeByName(spElement,L"ImageName", logger) )
    {
        ElementUtils::VerifyName(spElement, L"ProcessBlock", logger);
    }
    const CString& GetImageName() const { return m_imageName; }
};

//------------------------------------------------------------------------------
// Class: IronMan::ProcessBlocks
// Contains the data needed to do a system check on the running processes
// represents the ProcessBlocks element in the ParameterInfo.xml
//------------------------------------------------------------------------------
class ProcessBlocks
{
    ILogger& m_logger;
    CSimpleArray<ProcessBlock> m_processBlocks;
public:
    ProcessBlocks(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
    {

        if (spElement)
        {
            ElementUtils::VerifyName(spElement, L"ProcessBlocks", logger);

            CComPtr<IXMLDOMNode> spChild;
            HRESULT hr = spElement->get_firstChild(&spChild);
            if (S_OK == hr)
            {
                if (!ElementUtils::IsNodeComment(spChild))
                {
                    LOG(logger, ILogger::Verbose, L"ProcessBlock added");
                    m_processBlocks.Add(ProcessBlock(CComQIPtr<IXMLDOMElement>(spChild), logger));
                }
                do {
                    CComPtr<IXMLDOMNode> spSibling;
                    hr = spChild->get_nextSibling(&spSibling);
                    if (S_OK == hr)
                    {
                        if (!ElementUtils::IsNodeComment(spSibling))
                        {
                            LOG(logger, ILogger::Verbose, L"ProcessBlock added");
                            m_processBlocks.Add(ProcessBlock(CComQIPtr<IXMLDOMElement>(spSibling), logger));
                        }
                    }
                    spChild = spSibling;
                } while (!!spChild);
            }
            if (0 == m_processBlocks.GetSize() )
            {
                LOG(logger, ILogger::Verbose, L"No ProcessBlock element");
            }
        }
    }
    const CSimpleArray<CString> GetImageNamesToBlockOn() const
    {
        CSimpleArray<CString> imageNames;
        for (int i = 0; i < GetSize(); i++)
        {
            imageNames.Add(m_processBlocks[i].GetImageName() );
        }
        return imageNames;
    }
    int GetSize() const { return m_processBlocks.GetSize(); }
    ProcessBlock operator[] (DWORD nIndex) const
    {
        return m_processBlocks[nIndex];
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::ServiceBlock
// Contains the data needed to do a system check for one service
// represents the ServiceBlock element in the ParameterInfo.xml
//------------------------------------------------------------------------------
class ServiceBlock
{
    ILogger& m_logger;
    CString m_serviceName;
public:
    ServiceBlock(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_serviceName(ElementUtils::GetAttributeByName(spElement,L"ServiceName", logger) )
    {
        ElementUtils::VerifyName(spElement, L"ServiceBlock", logger);
    }
    const CString& GetServiceName() const { return m_serviceName; }
};

//------------------------------------------------------------------------------
// Class: IronMan::ServiceBlocks
// Contains the data needed to do a system check on the running services
// represents the ServiceBlocks element in the ParameterInfo.xml
//------------------------------------------------------------------------------
class ServiceBlocks
{
    ILogger& m_logger;
    CSimpleArray<ServiceBlock> m_serviceBlocks;
public:
    ServiceBlocks(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
    {
        if (spElement)
        {
            ElementUtils::VerifyName(spElement, L"ServiceBlocks", logger);

            CComPtr<IXMLDOMNode> spChild;
            HRESULT hr = spElement->get_firstChild(&spChild);
            if (S_OK == hr)
            {
                if (!ElementUtils::IsNodeComment(spChild))
                {
                    LOG(logger, ILogger::Verbose, L"ServiceBlock added");
                    m_serviceBlocks.Add(ServiceBlock(CComQIPtr<IXMLDOMElement>(spChild), logger));
                }
                do {
                    CComPtr<IXMLDOMNode> spSibling;
                    hr = spChild->get_nextSibling(&spSibling);
                    if (S_OK == hr)
                    {
                        if (!ElementUtils::IsNodeComment(spSibling))
                        {
                            LOG(logger, ILogger::Verbose, L"ServiceBlock added");
                            m_serviceBlocks.Add(ServiceBlock(CComQIPtr<IXMLDOMElement>(spSibling), logger));
                        }
                    }
                    spChild = spSibling;
                } while (!!spChild);
            }
            if (0 == m_serviceBlocks.GetSize() )
            {
                LOG(logger, ILogger::Verbose, L"No ServiceBlock element");
            }
        }
    }
    const CSimpleArray<CString> GetServiceNamesToBlockOn() const
    {
        CSimpleArray<CString> serviceNames;
        for (int i = 0; i < GetSize(); i++)
        {
            serviceNames.Add(m_serviceBlocks[i].GetServiceName() );
        }
        return serviceNames;
    }
    int GetSize() const { return m_serviceBlocks.GetSize(); }
    ServiceBlock operator[] (DWORD nIndex) const
    {
        return m_serviceBlocks[nIndex];
    }
};

class ProductDriveHints
{
public:
    struct IHint
    {
        virtual ~IHint() {}
        virtual bool EvaluateHint(WCHAR& driveLetter) = 0;
        virtual IHint* Clone() = 0;
    };

    class MsiComponentHint : public IHint
    {
        CString m_guid;
        ILogger& m_logger;
    public:
        MsiComponentHint(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : m_guid(ElementUtils::GetAttributeByName(spElement, L"Guid", logger))
            , m_logger(logger)
        {}
        MsiComponentHint(const MsiComponentHint& rhs)
            : m_guid(rhs.m_guid)
            , m_logger(rhs.m_logger)
        {}
        MsiComponentHint(const CString& guid, ILogger& logger)
            : m_guid(guid)
            , m_logger(logger)
        {}
        virtual bool EvaluateHint(WCHAR& driveLetter)
        {
            DWORD dw = MAX_PATH;
            CString cs;
            INSTALLSTATE ii = MsiLocateComponent(m_guid, cs.GetBuffer(MAX_PATH+1), &dw);
            cs._ReleaseBuffer();
            if (ii == INSTALLSTATE_LOCAL)
            {
                driveLetter = cs[0];
                LOG(m_logger, ILogger::Information, L"ComponentHint: " + m_guid + L" located at '" + cs + L"'");
                return true;
            }
            LOG(m_logger, ILogger::Information, L"ComponentHint: " + m_guid + L" not found");
            return false;
        }
        virtual MsiComponentHint* Clone() { return new MsiComponentHint(*this); }
    private:
        virtual INSTALLSTATE MsiLocateComponent(__in LPCWSTR szComponent, __out_ecount(*pcchBuf) LPWSTR lpPathBuf,__inout LPDWORD pcchBuf)
        {
            return ::MsiLocateComponent(szComponent, lpPathBuf, pcchBuf);
        }
    };

    class RegKeyHint : public IHint
    {
        ILogger& m_logger;
        CString m_location;
    public:
        RegKeyHint(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : m_location(ElementUtils::GetAttributeByName(spElement, L"Location", logger, true))
            , m_logger(logger)
        {
            ElementUtils::VerifyName(spElement, L"RegKeyHint", logger);
            RegKeyStringUtils::GetHiveFromString(m_location, L"RegKeyHint", logger);
        }
        RegKeyHint(const RegKeyHint& rhs)
            : m_logger(rhs.m_logger)
            , m_location(rhs.m_location)
        {}
        RegKeyHint(const CString& location, ILogger& logger)
            : m_logger(logger)
            , m_location(location)
        {}

        virtual bool EvaluateHint(WCHAR& driveLetter)
        {
            CString value;

            CString valueName;
            CString path(m_location);
            if (true == RegKeyStringUtils::ExtractValueAndPath(path, valueName))
            {
                CRegKey rk;
                LONG lres = HiveSelectionUtils::Open<CRegKey>(rk, path);
                if (lres == ERROR_SUCCESS)
                {
                    DWORD dwType = 0;
                    ULONG ulBytes = 0;
                    lres = rk.QueryValue(valueName, &dwType, NULL, &ulBytes);

                    if (lres == ERROR_SUCCESS)
                    {
                        switch(dwType)
                        {
                        default: // must be a string!
                            break;
                        case REG_SZ:
                        case REG_EXPAND_SZ:
                        case REG_MULTI_SZ:  // A sequence of null-terminated strings, terminated by an empty string (\0). 
                            lres = rk.QueryValue(valueName, &dwType, value.GetBuffer(ulBytes), &ulBytes); // overkill on the buffer size, but that's ok
                            value._ReleaseBuffer();
                            LOG(m_logger, ILogger::Information, L"RegKeyHint: " + m_location + L" contains '" + value + L"'");
                            driveLetter = value[0];
                            return true;
                        }
                    }
                }
            }

            LOG(m_logger, ILogger::Information, L"RegKeyHint: " + m_location + L" does NOT exist (or is not a string value).");
            return false;
        }
        virtual RegKeyHint* Clone() { return new RegKeyHint(*this); }
    };

    //------------------------------------------------------------------------------
    // Returns the number of Hints in the ProductDriveHints.  Used for testing
    //------------------------------------------------------------------------------
    int GetNumberOfHints() const 
    {
        return m_hints.GetSize();
    }

private:
    struct GiveUpHint : public IHint
    {
        virtual bool EvaluateHint(WCHAR& driveLetter)
        {
            CString cs(L"C:\\");
            ::GetSystemDirectory(cs.GetBuffer(MAX_PATH), MAX_PATH);
            cs._ReleaseBuffer();

            driveLetter = cs[0];
            return true;
        }
        virtual GiveUpHint* Clone() { return new GiveUpHint(); }
    };

    struct HintArray : public CSimpleArray<IHint*>
    {
        HintArray() {}
        HintArray(const HintArray& rhs)
        {
            for(int i=0; i<rhs.GetSize(); ++i)
            {
                Add(rhs[i]->Clone());
            }
        }
        virtual ~HintArray()
        {
            for (int i=0; i<GetSize(); ++i)
            {
                delete (*this)[i];
            }
        }
    } m_hints;

    static IHint* MakeHint(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        CString type = ElementUtils::GetName(spElement);
        
        if (type == L"ComponentHint")
        {
            return new MsiComponentHint(spElement, logger);
        }
        else if (type == L"RegKeyHint")
        {
            return new RegKeyHint(spElement, logger);
        }

        CInvalidXmlException ixe(L"Bad product drive hint type!");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

public:
    ProductDriveHints(CComPtr<IXMLDOMElement>& spElement, ILogger& logger)  
    {
        if ( NULL != spElement )
        {
            CComPtr<IXMLDOMNode> spChild;

            HRESULT hr = spElement->get_firstChild(&spChild);
            if (S_OK == hr)
            {
                m_hints.Add(MakeHint(CComQIPtr<IXMLDOMElement>(spChild),logger));

                do {
                    CComPtr<IXMLDOMNode> spSibling;
                    hr = spChild->get_nextSibling(&spSibling);
                    if (S_OK == hr)
                    {
                        m_hints.Add(MakeHint(CComQIPtr<IXMLDOMElement>(spSibling),logger));
                    }

                    spChild = spSibling;
                } while (!!spChild);
            }
            if (0 == m_hints.GetSize())
            {
                CInvalidXmlException ixe(L"No product drive hints found!");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
    }
    ProductDriveHints(const ProductDriveHints& rhs)
        : m_hints(rhs.m_hints)
    {}

    WCHAR EvaluateHints() const
    {
        WCHAR driveLetter = 0;
        for(int i=0; i<m_hints.GetSize(); ++i)
        {
            if (true ==  m_hints[i]->EvaluateHint(driveLetter))
                return driveLetter;
        }
        GiveUpHint().EvaluateHint(driveLetter);
        return driveLetter;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::SystemCheck
// Contains the data needed to do system checks before the install is done 
// represents the SystemCheck element in the ParameterInfo.xml
//------------------------------------------------------------------------------
class SystemCheck
{
    bool m_bDefined;
    ProcessBlocks m_processBlocks;
    ServiceBlocks m_serviceBlocks;
    ProductDriveHints m_productDrive;
    ILogger& m_logger;
public:
    SystemCheck(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_bDefined(false)
        , m_processBlocks(ProcessBlocks( ElementUtils::FindOptionalChildElementByName(spElement, L"ProcessBlocks", logger), logger) )
        , m_serviceBlocks(ServiceBlocks( ElementUtils::FindOptionalChildElementByName(spElement, L"ServiceBlocks", logger), logger) )
        , m_productDrive (ElementUtils::FindOptionalChildElementByName(spElement, L"ProductDriveHints", logger), logger)
    {
        if (spElement)
        {
            ElementUtils::VerifyName(spElement, L"SystemCheck", logger);
            m_bDefined = true;
        }
    }
    const bool IsDefined() const 
    { 
        return m_bDefined;
    }
    const ProcessBlocks& GetProcessBlocks() const { return m_processBlocks; }
    const ServiceBlocks& GetServiceBlocks() const { return m_serviceBlocks; }
    const ProductDriveHints& GetProductDriveHints() const { return m_productDrive; }
private:
};

class CSetupVersion
{
    ULONG field[2];
public:

    CSetupVersion(CString strVersion)
    {
        field[0] = 0;
        field[1] = 0;
        ParseSetupVersion(strVersion);
    }

    ULONG MajorVersion()
    {
        return field[0];
    }

    ULONG MinorVersion()
    {
        return field[1];
    }

private:
    CSetupVersion();

    void ParseSetupVersion(CString strVersion)
    {
        // The errno global variable was not being reset between unit tests, 
        //so setting it to zero before call to wcstol or wcstoul
        _set_errno(0);

        wchar_t* pStart = strVersion.GetBuffer();
        wchar_t* pEnd = NULL;
        CInvalidXmlException ixe(L"Invalid SetupVersion specified");

        for (int i=0; i<2; ++i)
        {
            if (iswdigit(*pStart) == 0)
                throw ixe;
                
            pEnd = NULL;
            field[i] = wcstoul(pStart, &pEnd, 0);

            errno_t error = 0;
            _get_errno(&error); // errno is set to ERANGE if overflow or underflow occurs
            if (error == ERANGE)
                throw ixe;

            if (i == 0)
            {
                if (pEnd == NULL || *pEnd != L'.')
                    throw ixe;
            }

            pStart = pEnd+1;
        }
        if (pEnd == NULL || *pEnd != L'\0')
            throw ixe;

        strVersion._ReleaseBuffer();
    }
};

class EngineData
{
    ILogger& m_logger;    
    BlockersElement m_blocker;
    Ui m_ui;
    EnterMaintenanceModeIf m_arpIf;  //The should we enter Maintenance mode if - <EnterMaintenanceModeIf/>
    Items m_items; //The what item should we download and install if - <Items/>
    SystemCheck m_systemCheck;
    Configuration m_configuration;
    CString m_csBundleId;

public:
    EngineData(const EngineData& rhs) 
        : m_ui(rhs.m_ui)
        , m_arpIf(rhs.m_arpIf)
        , m_items(rhs.m_items)
        , m_blocker(rhs.m_blocker)
        , m_configuration(rhs.m_configuration)
        , m_logger(rhs.m_logger)
        , m_systemCheck(rhs.m_systemCheck)
        , m_csBundleId(rhs.m_csBundleId)
    {
    }
    virtual ~EngineData() {}

    static CString StripComments(CString cs)
    {
        int start;
        while (-1 != (start = cs.Find(L"<!--")))
        {
            int end = cs.Find(L"-->", start + 4);
            if (end == -1)
            {
                IMASSERT(0 && "started a comment node, but didn't end it?");
                break;
            }

            CString comment = cs.Mid(start, end-start+3);
            cs.Replace(comment, L"");
        }
        return cs;
    }

    // Replaces the following HTML special chars: ", &, ', < & >
    // with their escaped versions: &quot; L"&amp;   L"&apos;  L"&lt;   L"&gt; 
    static void ReplaceSpecialHTMLChars(CString& str)
    {
        const int countSpecialChars = 5;
        WCHAR specialChars[countSpecialChars] = { L'\"',     L'&',       L'\'',      L'<',      L'>'};
        LPCWSTR subStrings[countSpecialChars] = { L"&quot;", L"&amp;",   L"&apos;",  L"&lt;",   L"&gt;" };

        LPCWSTR psz = str.GetBuffer();

        int iStart = 0;
        CString newStr;

        for (int iCurr=0; iCurr < CString::StringLength(str); ++iCurr)
        {
            for (int j=0; j<countSpecialChars; ++j)
            {
                if (str[iCurr] == specialChars[j])
                {
                    newStr += str.Mid(iStart, (iCurr - iStart));
                    iStart = iCurr+1;

                    newStr += subStrings[j];
                }
            }
        }

        if (CString::StringLength(newStr) > 0 && iStart > 0)
        {
            newStr += str.Mid(iStart);
            IMASSERT(CString::StringLength(newStr) > CString::StringLength(str));
            str = newStr;
        }
    }

    // Returns a new string made of the old string whith all the loc-ids replaced with localized text.
    static CString SubstituteLocIDs(CString strIn, ILocalizedData &ld)
    {
        class ParamGetter
        {
            ILocalizedData &m_ld;
        public:         
            ParamGetter(ILocalizedData& ld) : m_ld(ld) {}
            CString GetParamValue(CString param) const
            { 
                CString value = m_ld.GetLocalizedText(param); 
                ReplaceSpecialHTMLChars(value);
                return value;
            }
        };

        ParamSubstituter<ParamGetter> ps(ParamGetter(ld), L"#(loc.", L")");
        return ps.SubstituteAnyParams(strIn);
    }

    // ----------------------------------------------------------------------------------------
    // CreateEngineData()
    // returns the EngineData, given the location of the ParameterInfo.xml file
    // if the /uninstallpatch switch is set, then it will use modified ParameterInfo
    // that won't include all the unneeded parts
    // ----------------------------------------------------------------------------------------
    static const EngineData CreateEngineData(
        LPCTSTR             szFilename, 
        ILocalizedData&     ld=NullLocalizedData::GetNullLocalizedData(), 
        ILogger&            logger = NullLogger::GetNullLogger()
        )
    {
        return CreateEngineDataT<CCmdLineSwitches>(szFilename, ld, logger);
    }

    // ----------------------------------------------------------------------------------------
    // CreateEngineDataT()  DO NOT CALL DIRECTLY, FOR UNIT TESTING ONLY
    // returns the EngineData, given the location of the ParameterInfo.xml file
    // if the /uninstallpatch switch is set, then it will use modified ParameterInfo
    // that won't include all the unneeded parts
    // ----------------------------------------------------------------------------------------
    template<typename TCmdLineSwitches>
    static const EngineData CreateEngineDataT(
        LPCTSTR             szFilename, 
        ILocalizedData&     ld=NullLocalizedData::GetNullLocalizedData(), 
        ILogger&            logger = NullLogger::GetNullLogger()
        )
    {   
        // read the file into a BSTR, then call xmldom.loadXML version
        CComBSTR bstrXML = XmlUtils::ReadXml(szFilename,logger);

        // if the /UninstallPatch switch is not set, return unmodified
        TCmdLineSwitches switches;
        CString patchCode(switches.UninstallPatch());
        if (patchCode.IsEmpty())
        {
            return CreateEngineData(bstrXML, ld, logger);
        }
        else
        {
            // get original EngineData.  need it for UI/@Version and UI/@Dll
            EngineData engineDataOriginal(CreateEngineData(bstrXML, ld, logger));
            // get the modified ParameterInfo string
            CComBSTR bstrXMLNew(CreateXmlForUninstallPatch::CreateParameterInfo(patchCode
                                                                        , engineDataOriginal.GetUi().GetVersion()
                                                                        , engineDataOriginal.GetUi().GetDll()
                                                                        , ld
                                                                        , logger));
            return CreateEngineData( bstrXMLNew, ld, logger);
        }
    }

    static void PerformSetupVersionCheck(
        CComPtr<IXMLDOMElement> &spElement, 
        ILogger& logger)
    {
        return PerformSetupVersionCheckT<CCmdLineSwitches>(spElement, logger);
    }

    template<typename TCmdLineSwitches>
    static void PerformSetupVersionCheckT(
        CComPtr<IXMLDOMElement> &spElement, 
        ILogger& logger)
    {
        LOGEX(logger, ILogger::Verbose, L"Current SetupVersion = %s", c_pszSetupVersion);

        if (TCmdLineSwitches().NoSetupVersionCheck())
        {
            LOG(logger, ILogger::Verbose, L"Command line switch 'NoSetupVersionCheck' found - so not performing SetupVersion check.");
            return;
        }
        
        CString strSetupVersionSpecified = ElementUtils::GetAttributeByName(spElement, L"SetupVersion", logger);
        if (strSetupVersionSpecified.IsEmpty())
        {
            CInvalidXmlException ixe(L"SetupVersion not specified");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }

        LOGEX(logger, ILogger::Verbose, L"SetupVersion specified in ParameterInfo.xml is '%s'", strSetupVersionSpecified);
        if (strSetupVersionSpecified.CompareNoCase(c_pszSetupVersion) == 0)
            return; // SetupVersion specified in ParameterInfo is the current supported version.

        // If the major versions are same, and the minor version specified in
        // ParametrInfo.xml is less than the current SetupVersion we will put up 
        // a warning message and continue.
        CSetupVersion setupVersionCurrent(c_pszSetupVersion);
        CSetupVersion setupVersionSpecified(strSetupVersionSpecified);

        if (setupVersionSpecified.MajorVersion() == setupVersionCurrent.MajorVersion())
        {
            if (setupVersionSpecified.MinorVersion() < setupVersionCurrent.MinorVersion())
            {
                LOGEX(logger, ILogger::Warning, L"SetupVersion specified in ParameterInfo.xml has a minor version lower than the currently supported version.");
                return;
            }
            else if (setupVersionSpecified.MinorVersion() > setupVersionCurrent.MinorVersion())
            {
                CInvalidXmlException ixe(L"SetupVersion specified in ParameterInfo.xml has a minor version greater than the currently supported version.");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        else
        {
            CString msg = CString(L"SetupVersion specified in ParameterInfo.xml is ")
                           + CString((setupVersionSpecified.MajorVersion() < setupVersionCurrent.MajorVersion()) ? L"lower" : L"higher")
                           + CString(L" than the currently supported version.");

            CInvalidXmlException ixe(msg);
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }

    static const EngineData CreateEngineData(
        const CComBSTR&     bstrXML1, 
        ILocalizedData&     ld=NullLocalizedData::GetNullLocalizedData(), 
        ILogger&            logger = NullLogger::GetNullLogger()
        )
    {
        CString pop(L"threw exception");
        ENTERLOGEXIT(logger, pop);

        // first things first:  strip out all comments
        CString strXMLWithNoComments = StripComments(CString(bstrXML1));
        CComBSTR bstrXML = SubstituteLocIDs(strXMLWithNoComments, ld);

        try
        {
            CoInitializer coInitializer;

            // NOTE NOTE NOTE:  
            // no validation is done during XML Loading, as this would mean shipping the .xsd file.
            // Instead, the validation should be done at authoring time (see validate.js), and again at SetupBuild time.
            // Here we expect valid XML, validate via code and throw exceptions if it's not in the right format.

            CComPtr<IXMLDOMDocument2> spDoc;
            HRESULT hr = spDoc.CoCreateInstance(__uuidof(DOMDocument30)); // this also works:  spDoc.CoCreateInstance(_T("Msxml2.DOMDocument.3.0"));
            if (SUCCEEDED(hr))
            {
                VARIANT_BOOL vb = VARIANT_FALSE;

                // The bstrXML should not contain any reserved HTML chars, if any are needed they should be replaced with ther entity names.
                // Note when you read text from DOM, it will return the actual chars, even for the reserved HTML chars, instead of their entity names. 
                hr = spDoc->loadXML(bstrXML, &vb);
                if ((S_OK == hr) && // S_FALSE is a failure
                    (vb == VARIANT_TRUE))
                {
                    CComPtr<IXMLDOMElement> spElement;
                    hr = spDoc->get_documentElement(&spElement);
                    if (S_OK == hr)
                    {
                        PerformSetupVersionCheck(spElement, logger);
                        pop = L"succeeded";
                        return EngineData(spElement, logger);
                    }
                } 
                // Get IXMLDOMParseError object from IXMLDOMDocument2::validate and get the actual error
                CComPtr<IXMLDOMParseError> spParseError;
                spDoc->validate(&spParseError);
                CComBSTR bstrReason;
                spParseError->get_reason(&bstrReason);
                throw CInvalidXmlException(CString(bstrReason));
            }
            throw CHResultException(hr);
        }
        catch(const CException& e)
        {
          pop = L"threw exception";
          LOG(logger, ILogger::Error, e.GetMessage());
          throw;
        }
        catch(...)
        {
            pop = L"threw exception";
            LOG(logger, ILogger::Error, L"unknown exception thrown, caught and about to be rethrown.");
            throw;
        }
    }  

    const Ui& GetUi() const { return m_ui; }
    const Items& GetItems() const { return m_items; }
    const SystemCheck& GetSystemCheck() const { return m_systemCheck; }
    bool EvaluateEnterMaintenanceModeIf(const IProvideDataToOperand& ipdtoDataToOperand) const
    {
        CString section(L" evaluates to 'not in maintenance mode'");
        PUSHLOGSECTIONPOP(m_logger, L"MaintenanceMode determination", L"evaluating EnterMaintenanceModeIf", section);
        bool b = m_arpIf.Evaluate(ipdtoDataToOperand);
        if (b)
            section = L" evaluates to 'in maintenance mode'";
        return b;
    }

    const BlockersElement& GetBlocks() const 
    {        
        return m_blocker;
    }

    const Configuration& GetConfiguration() const
    {
        return m_configuration;
    }

    //------------------------------------------------------------------------------
    // BundleId
    // Contains the BundleId that uniquely identifies the Bundle
    //------------------------------------------------------------------------------
    const CString& BundleId() const
    {
        return m_csBundleId;
    }

private:
    const EngineData& operator=(const EngineData& rhs); // no impl
private:
    EngineData(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_ui(ElementUtils::FindChildElementByName(spElement, L"UI", logger), logger)
        , m_arpIf(ElementUtils::FindChildElementByName(spElement, L"EnterMaintenanceModeIf", logger), logger)
        , m_blocker(ElementUtils::FindOptionalChildElementByName(spElement, L"Blockers", logger), logger)
        , m_items(ElementUtils::FindChildElementByName(spElement, L"Items", logger), logger)
        , m_systemCheck(ElementUtils::FindOptionalChildElementByName(spElement, L"SystemCheck", logger), logger)
        , m_configuration(ElementUtils::FindOptionalChildElementByName(spElement, L"Configuration", logger), logger)
    {
        // Get BundleId
        
        m_csBundleId = ElementUtils::GetAttributeByName(ElementUtils::FindChildElementByName(spElement, L"Registration", logger), L"Id", logger).Trim();
        // BundleId cannot be an empty string
        if (m_csBundleId.IsEmpty())
        {
            CInvalidXmlException ixe(L"schema validation failure:  Registration/@BundleId cannot be empty");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }

        // Excpected number of top level elements. Only UI, EnterMaintenanceModeIf and Items are mandatory
        unsigned int numExpectedTopLevelElements = 3;

        if (m_systemCheck.IsDefined())
        {
            ++numExpectedTopLevelElements;
        }
        // Look for the optional blockers
        if (m_blocker.IsDefined())
        {
            ++numExpectedTopLevelElements;
        }
        
        if (m_configuration.IsDefined())
        {
            ++numExpectedTopLevelElements;
        }

        ElementUtils::VerifyName(spElement, L"Setup", logger);
    }
};

}

