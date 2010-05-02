//-------------------------------------------------------------------------------------------------
// <copyright file="idataproviders.h" company="Microsoft">
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

#include "ux\ux.h"
#include "IFeature.h"
#include "common\operation.h"
#include "schema\ConfigurationElement.h"
#include "IInstallItems.h"
#include "IDownloadItems.h"
#include "IBlockChecker.h"
#include "ILogger.h"
#include "IBurnCore.h"

namespace IronMan
{


struct INotifyEngine
{
#ifdef FeaturesAreImplented
// Features feature not implemented
    virtual void SetFeatureTreeRoot(IFeatureTreeRoot&) = 0;
#endif // FeaturesAreImplented
};

struct IProvideDataToUi
{
    virtual Operation::euiOperation GetOperation() = 0; // Is this an install, uninstall etc. ?
    virtual bool  InitializeItems() = 0;
    virtual bool  GetItemValidationState(const bool &bStopProcessing) = 0;
    virtual const CSimpleArray<CString>& GetAffectedProducts(const bool &bStopProcessing) = 0;
    virtual ILogger& GetEngineLogger() = 0; // Logger (to get name/path)    
    virtual Ux& GetUxLogger() = 0 ;
    virtual const CString & GetPackageName() = 0;   // Get name of package as defined in ParameterInfo.xml
    virtual const CSimpleArray<CString>& GetImageNamesToBlockOn() = 0; // Returns an array of image names to block on
    virtual const CSimpleArray<CString>& GetServiceNamesToBlockOn() = 0; // Returns an array of service names to block on
    virtual const ULONGLONG GetSpaceRequiredForDownload() = 0; 
    virtual const ULONGLONG GetSpaceRequiredOnProductInstalledDrive() = 0;
    virtual const ULONGLONG GetSpaceRequiredOnSystemDrive() = 0;
    virtual const WCHAR GetProductDriveLetter() const = 0;
#ifdef FeaturesAreImplented
// Features feature not implemented
    virtual IFeatureTreeRoot& GetFeatureTreeRoot() = 0;
#endif // FeaturesAreImplented
    virtual INotifyEngine& GetEngineNotificationInterface() = 0;
    virtual const UserExperienceDataCollection::Policy UxCollectionPolicy() const = 0;
    virtual void UpdateLiveOperation(Operation::euiOperation) = 0;
    virtual IInstallItems& GetInstallItems() = 0;
    virtual IDownloadItems& GetDownloadItems() = 0;
    virtual bool IsSimultaneousDownloadAndInstallDisabled() const = 0;
    virtual const IProvideDataToOperand& GetDataToOperand() const = 0;
    virtual IBlockChecker& GetBlockChecker() = 0;
    virtual unsigned int GetAuthoredItemCount(ItemBase::ItemType itemType) const = 0;
    virtual unsigned int GetAuthoredItemCount() const = 0;
    virtual HRESULT Detect() = 0;
    virtual HRESULT Plan(__in BURN_ACTION action) = 0;
    virtual HRESULT GetBurnView(__in IBurnView**)  const = 0;
    virtual HRESULT SetSource(__in LPCWSTR pathSource) = 0;
}; 
}
