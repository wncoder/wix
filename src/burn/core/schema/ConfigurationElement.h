//-------------------------------------------------------------------------------------------------
// <copyright file="ConfigurationElement.h" company="Microsoft">
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

#include "..\common\XmlUtils.h"
#include "..\interfaces\ILogger.h"
#include "..\CmdLineParser.h"

namespace IronMan
{
//------------------------------------------------------------------------------
// Class: IronMan::UserExperienceDataCollection
// Determine the policy to collect Usage information.
// The default policy is "Disabled"
//------------------------------------------------------------------------------
template <typename CCmdLineSwitches>
class UserExperienceDataCollectionT
{
public:
        //Enum for the selection
    enum Policy
    {
        UserControlled = 1, //Depends on the Ux Checkbox on User's selection on the Eula Page
        Disabled = 2,       //Hide the UX Checkbox in Eula page and use NullMetrics so that we never collect any data.
        AlwaysUploaded = 3  //Gray out the UX Checkbox and set to always checked. 
    }; 

private:
    ILogger& m_logger;
    Policy m_policy;
    UxEnum::patchTrainEnum m_servicingTrain;
    const bool m_bElementDefined;
    const bool m_bCmdLineOverride;
    CPath m_pthMetricFile;

public:
    UserExperienceDataCollectionT(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_bElementDefined(true)
        , m_servicingTrain(UxEnum::ptNone)
        , m_bCmdLineOverride(GetCmdLineOverride())
        , m_pthMetricFile(ModuleUtils::GetDllPath())
    {
        m_pthMetricFile.Append(ElementUtils::GetOptionalAttributeByName(spElement,L"MetricLoader", logger));
        SetPolicy(GetPolicy(ElementUtils::GetOptionalAttributeByName(spElement,L"Policy", logger), logger));
        SetCEIPDatapoints(spElement, logger);
        ElementUtils::VerifyName(spElement, L"UserExperienceDataCollection", logger);
    }

    UserExperienceDataCollectionT()
        : m_bElementDefined(false)
        , m_bCmdLineOverride(GetCmdLineOverride())
        , m_servicingTrain(UxEnum::ptNone)
        , m_logger(IronMan::NullLogger::GetNullLogger())
    {
        SetPolicy(UserExperienceDataCollectionT::UserControlled);
    }

    const Policy GetPolicy() const
    {
        return m_policy;
    }

    const CPath GetMetricLoaderExe() const
    {
        return m_pthMetricFile;
    }

    const UxEnum::patchTrainEnum GetServicingTrain() const
    {
        return m_servicingTrain;
    }

    const bool IsDefined() const
    {
        return m_bElementDefined;
    }

    const bool CmdLineOverride() const
    {
        return m_bCmdLineOverride;
    }

private:
    //The default value if Policy is not defined is UserControlled.
    static Policy GetPolicy(CString strValue, ILogger& logger)
    {
        if (L"UserControlled" == strValue)
        {
            return UserExperienceDataCollectionT::UserControlled;
        }
        else if ((L"Disabled" == strValue) || strValue.IsEmpty())
        {
            return UserExperienceDataCollectionT::Disabled;
        }
        else if (L"AlwaysUploaded" == strValue)
        {
            return UserExperienceDataCollectionT::AlwaysUploaded;
        }

        CInvalidXmlException ixe(L"schema validation failure: Invalid Policy Value being defined.");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    void SetPolicy(Policy policyToUseIfNotOverridden)
    {
        // /CEIPconsent command line override is only used if the PI.xml policy is UserControled or
        // missing(it then defaults to UserControlled)
        if ( m_bCmdLineOverride && UserExperienceDataCollectionT::UserControlled == policyToUseIfNotOverridden)
        {
            m_policy = UserExperienceDataCollectionT::AlwaysUploaded;
        }
        else
        {
            m_policy = policyToUseIfNotOverridden;
        }
    }

    //Iterate through to process process all the child element of UserExperienceDataCollection
    void SetCEIPDatapoints(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        CComPtr<IXMLDOMNode> spChild;

        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            SetCEIPDatapoint(spChild);
            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    SetCEIPDatapoint(spSibling);
                }
                spChild = spSibling;
            } while (!!spChild);
        }
    }

    //Process the child element of UserExperienceDataCollection
    void SetCEIPDatapoint(const CComPtr<IXMLDOMNode>& spNode)
    {
        CComPtr<IXMLDOMElement> spElement = CComQIPtr<IXMLDOMElement>(spNode);
        //It is a comment, don't have to process.
        if (ElementUtils::IsNodeComment(spElement))
        {
            return;
        }

        CString strName = ElementUtils::GetName(spElement);
        if (0 == strName.CompareNoCase(L"PatchType"))
        {
            SetPatchType(ElementUtils::GetAttributeByName(spElement, L"Type", m_logger));
        }
        else
        {
            CString strErrorMessage;
            strErrorMessage.Format(L"Invalid UserExperienceDataCollection's child element: %s",strName);
            LOG(m_logger, ILogger::Error, strErrorMessage);
            CInvalidXmlException ixe(strErrorMessage);
            throw ixe;
        }
    }

    //Process the PatchType element to extract the PatchType - GDR | LDR | None.
    void SetPatchType(CString strType)
    {
        bool bIsError = false;
        m_servicingTrain = UxEnum::GetTrainFromString(strType, bIsError);

        if (bIsError)
        {
            CString strErrorMessage;
            strErrorMessage.Format(L"Invalid PatchType: %s",strType);
            LOG(m_logger, ILogger::Error, strErrorMessage);
            CInvalidXmlException ixe(strErrorMessage);
            throw ixe;
        }
    }

    static bool GetCmdLineOverride()
    {
        CCmdLineSwitches switches;
        return switches.CEIPconsentSwitchPresent();
    }
};
typedef UserExperienceDataCollectionT<CCmdLineSwitches> UserExperienceDataCollection;

/*------------------------------------------------------------------------------
 Class: IronMan::DownloadInstallSetting
 Determine the DownloadInstallSetting from authored values in Configuration
 seciton and overridable commandline switch, /serialdownload.
 Default mode is simultaneous download and install. When SerialDownload attribute
 is authored true, serial download and install is performed. 
 If /serialdownload is passed in via command line, this overrides the authoring 
 and the defualt behavior of simultaneous download and install. Only serial download 
 and install are performed.
------------------------------------------------------------------------------*/
template <typename CCmdLineSwitches>
class DownloadInstallSettingT
{
public:
    //Enum for the selection
    enum SettingEnum
    {
        Simutaneous = 1,  //Install and Download happens at the same time
        Serial = 2,  //Install happens only after Download Completes
        NotDefined = 3  // Not authored in.
    }; 

private:
    ILogger& m_logger;
    SettingEnum m_setting;
    const bool m_bDefined;
    const bool m_bCmdLineOverride;

public:
    // Constructor when element is authored.
    DownloadInstallSettingT(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_bDefined(true)
        , m_bCmdLineOverride(GetCmdLineOverride())
    {
        UpdateSetting(GetSetting(ElementUtils::GetAttributeByName(spElement,L"SerialDownload", logger), logger));
        ElementUtils::VerifyName(spElement, L"DownloadInstallSetting", logger);
    }

    // Constructor used when the element is not authored.
    DownloadInstallSettingT()
        : m_bDefined(false)
        , m_bCmdLineOverride(GetCmdLineOverride())
        , m_logger(IronMan::NullLogger::GetNullLogger())
    {
        UpdateSetting(DownloadInstallSettingT::NotDefined);
    }

    const SettingEnum GetSetting() const
    {
        return m_setting;
    }

    const bool IsDefined() const
    {
        return m_bDefined;
    }

private:
    static SettingEnum GetSetting(CString csValue, ILogger& logger)
    {
        if (0 == csValue.CompareNoCase(L"true"))
            return DownloadInstallSettingT::Serial;
        else if (0 == csValue.CompareNoCase(L"false"))
            return DownloadInstallSettingT::Simutaneous;
        CInvalidXmlException ixe(L"schema validation failure: Invalid SerialDownload Value. Only True and False are supported.");
        throw ixe;
    }

    // If /serialdownload is passed at command line, it will override authored value.
    void UpdateSetting(SettingEnum setting)
    {
        if ( m_bCmdLineOverride )
        {
            m_setting = DownloadInstallSettingT::Serial;
        }
        else
        {
            m_setting = setting;
        }
    }

    static bool GetCmdLineOverride()
    {
        CCmdLineSwitches switches;
        return switches.SerialDownloadSwitchPresent();
    }
};
typedef DownloadInstallSettingT<CCmdLineSwitches> DownloadInstallSetting;

//------------------------------------------------------------------------------
// Class: IronMan::DisabledCommandLineSwitch
// Contains the data needed to do a system check for one service
// represents the DisabledCommandLineSwitch element in the ParameterInfo.xml
//------------------------------------------------------------------------------
class DisabledCommandLineSwitch
{
    ILogger& m_logger;
    CString m_switchName;
public:
    DisabledCommandLineSwitch(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_switchName(ElementUtils::GetAttributeByName(spElement,L"Name", logger) )
    {
        ElementUtils::VerifyName(spElement, L"CommandLineSwitch", logger);
    }

    const CString& GetSwitchName() const
    {
        return m_switchName;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::DisabledCommandLineSwitches
// Gets the list of disabled command line switches from ParameterInfo.xml
// represents the DisabledCommandLineSwitches element in the ParameterInfo.xml
//------------------------------------------------------------------------------
class DisabledCommandLineSwitches
{
    ILogger& m_logger;
    CSimpleArray<DisabledCommandLineSwitch> m_disabledCommandLineSwitches;
public:
    DisabledCommandLineSwitches(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
    {
        if ( spElement )
        {
            ElementUtils::VerifyName(spElement, L"DisabledCommandLineSwitches", logger);

            CComPtr<IXMLDOMNode> spChild;
            HRESULT hr = spElement->get_firstChild(&spChild);
            if (S_OK == hr)
            {
                if (!ElementUtils::IsNodeComment(spChild))
                {
                    DisabledCommandLineSwitch newDisabledCommandLineSwitch(CComQIPtr<IXMLDOMElement>(spChild), logger);
                    LOG(logger, ILogger::Verbose, L"Disabled CommandLineSwitch added: " + newDisabledCommandLineSwitch.GetSwitchName() );
                    m_disabledCommandLineSwitches.Add(newDisabledCommandLineSwitch);
                }
                do {
                    CComPtr<IXMLDOMNode> spSibling;
                    hr = spChild->get_nextSibling(&spSibling);
                    if (S_OK == hr)
                    {
                        if (!ElementUtils::IsNodeComment(spSibling))
                        {
                            DisabledCommandLineSwitch newDisabledCommandLineSwitch(CComQIPtr<IXMLDOMElement>(spSibling), logger);
                            LOG(logger, ILogger::Verbose, L"Disabled CommandLineSwitch added: " + newDisabledCommandLineSwitch.GetSwitchName() );
                            m_disabledCommandLineSwitches.Add(newDisabledCommandLineSwitch);
                        }
                    }
                    spChild = spSibling;
                } while (!!spChild);
            }

            // There must be at least one CommandLineSwitch child element if the DisabledCommandLineSwitches is specified
            if (0 == m_disabledCommandLineSwitches.GetSize() )
            {
                CInvalidXmlException ixe(L"The DisabledCommandLineSwitches block has no CommandLineSwitches specified - either add them or remove the DisabledCommandLineSwitches block");
                throw ixe;
            }
        }

        if (0 == m_disabledCommandLineSwitches.GetSize() )
        {
            LOG(logger, ILogger::Verbose, L"No DisabledCommandLineSwitches block was specified");
        }
    }

    DisabledCommandLineSwitches()
        : m_logger(IronMan::NullLogger::GetNullLogger())
    {
    }

    DisabledCommandLineSwitches& operator=(const DisabledCommandLineSwitches& rhs)
    {
        if (this != &rhs)
        {
            m_logger = rhs.m_logger;
            m_disabledCommandLineSwitches = rhs.m_disabledCommandLineSwitches;
        }

        return *this;
    }

    // Return number of switches disabled, zero or more
    int GetSize() const
    {
        return m_disabledCommandLineSwitches.GetSize();
    }

    // Return the disabled switch at specified index
    DisabledCommandLineSwitch operator[] (DWORD nIndex) const
    {
        return m_disabledCommandLineSwitches[nIndex];
    }

    const bool IsDefined() const
    {
        return 0 < GetSize();
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::AdditionalCommandLineSwitch
// Contains the data needed to represent
// the AdditionalCommandLineSwitch element in the ParameterInfo.xml
// These are used purely by oprands, which can use the presence or absence
// of the additional switches in boolean conditional expressions,
// e.g. to determine applicability
//------------------------------------------------------------------------------
class AdditionalCommandLineSwitch
{
    ILogger& m_logger;
    CString m_switchName;
public:
    AdditionalCommandLineSwitch(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_switchName(ElementUtils::GetAttributeByName(spElement,L"Name", logger) )
    {
        ElementUtils::VerifyName(spElement, L"CommandLineSwitch", logger);
    }

    const CString& GetSwitchName() const
    {
        return m_switchName;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::AdditionalCommandLineSwitches
// Gets the list of additonal command line switches from ParameterInfo.xml
// These are used purely by oprands, which can use the presence or absence
// of the additional switches in boolean conditional expressions,
// e.g. to determine applicability
// This class represents the AdditionalCommandLineSwitches element ParameterInfo.xml
//------------------------------------------------------------------------------
class AdditionalCommandLineSwitches
{
    ILogger& m_logger;
    CSimpleArray<AdditionalCommandLineSwitch> m_additionalCommandLineSwitches;
public:
    AdditionalCommandLineSwitches(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
    {
        if ( spElement )
        {
            ElementUtils::VerifyName(spElement, L"AdditionalCommandLineSwitches", logger);

            CComPtr<IXMLDOMNode> spChild;
            HRESULT hr = spElement->get_firstChild(&spChild);
            if (S_OK == hr)
            {
                if (!ElementUtils::IsNodeComment(spChild))
                {
                    AdditionalCommandLineSwitch newAdditionalCommandLineSwitch(CComQIPtr<IXMLDOMElement>(spChild), logger);
                    LOG(logger, ILogger::Verbose, L"Additional CommandLineSwitch added: " + newAdditionalCommandLineSwitch.GetSwitchName() );
                    m_additionalCommandLineSwitches.Add(newAdditionalCommandLineSwitch);
                }
                do {
                    CComPtr<IXMLDOMNode> spSibling;
                    hr = spChild->get_nextSibling(&spSibling);
                    if (S_OK == hr)
                    {
                        if (!ElementUtils::IsNodeComment(spSibling))
                        {
                            AdditionalCommandLineSwitch newAdditionalCommandLineSwitch(CComQIPtr<IXMLDOMElement>(spSibling), logger);
                            LOG(logger, ILogger::Verbose, L"Additional CommandLineSwitch added: " + newAdditionalCommandLineSwitch.GetSwitchName() );
                            m_additionalCommandLineSwitches.Add(newAdditionalCommandLineSwitch);
                        }
                    }
                    spChild = spSibling;
                } while (!!spChild);
            }

            // There must be at least one CommandLineSwitch child element if the AdditionalCommandLineSwitches is specified
            if (0 == m_additionalCommandLineSwitches.GetSize() )
            {
                CInvalidXmlException ixe(L"The AdditionalCommandLineSwitches block has no CommandLineSwitches specified - either add them or remove the AdditionalCommandLineSwitches block");
                throw ixe;
            }
        }

        if (0 == m_additionalCommandLineSwitches.GetSize() )
        {
            LOG(logger, ILogger::Verbose, L"No AdditionalCommandLineSwitches block was specified");
        }
    }

    AdditionalCommandLineSwitches()
        : m_logger(IronMan::NullLogger::GetNullLogger())
    {
    }

    AdditionalCommandLineSwitches& operator=(const AdditionalCommandLineSwitches& rhs)
    {
        if (this != &rhs)
        {
            m_logger = rhs.m_logger;
            m_additionalCommandLineSwitches = rhs.m_additionalCommandLineSwitches;
        }

        return *this;
    }

    // Return number of switches Additional, zero or more
    int GetSize() const
    {
        return m_additionalCommandLineSwitches.GetSize();
    }

    // Return the Additional switch at specified index
    AdditionalCommandLineSwitch operator[] (DWORD nIndex) const
    {
        return m_additionalCommandLineSwitches[nIndex];
    }

    const bool IsDefined() const
    {
        return 0 < GetSize();
    }
};


// This class controls the instance setting of the installer to one or more.
// If Name is authored, it is returned in GetMutextName() method. This is used
// by engine to create a Global mutex by that name and no other setup pakcage can 
// run with the same mutex authored.
class BlockingMutex
{
    ILogger& m_logger;
    const CString m_mutexName;
    const bool m_bDefined;
public:

    // Constructor
    BlockingMutex(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_mutexName(ElementUtils::GetAttributeByName(spElement,L"Name", logger))
        , m_bDefined(true)
    {
        if (m_mutexName.IsEmpty() || (-1 != m_mutexName.Find(L'\\')))
        {
            CInvalidXmlException ixe(L"BlockingMutex Name attribute should not be empty and cannot contain '\\'.");
            throw ixe;
        }
        ElementUtils::VerifyName(spElement, L"BlockingMutex", logger);
    }

    // Epmty Contstructor
    BlockingMutex()
        : m_logger(IronMan::NullLogger::GetNullLogger())
        , m_mutexName(L"")
        , m_bDefined(false)
    {
    }

    // Unit tests Constructor
    BlockingMutex(const CString& mutexName, bool defined, ILogger& logger = IronMan::NullLogger::GetNullLogger())
        : m_mutexName(mutexName)
        , m_bDefined(defined)
        , m_logger(logger)
    {
    }

    // Returns the name of the authored mutex
    const CString& GetMutexName() const
    {
        return m_mutexName;
    }

    // True if the element is authored in.
    const bool IsDefined() const
    {
        return m_bDefined;
    }
};

// This class controls if Files In Use UI is prompted to the user or not..
// If Prompt is authored false, any files in use messages from windows installer 
// are 'ignored' and install will continue.
class FilesInUseSetting
{
    ILogger& m_logger;
    const bool m_bPrompt;
    const bool m_bDefined;
public:

    // Constructor
    FilesInUseSetting(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_bPrompt(ElementUtils::EvaluateToBoolValue(L"Prompt"
                                                      ,true
                                                      ,ElementUtils::GetAttributeByName(spElement,L"Prompt", logger)
                                                      ,logger))
        , m_bDefined(true)
    {
        ElementUtils::VerifyName(spElement, L"FilesInUseSetting", logger);
    }

    // Epmty Contstructor
    FilesInUseSetting()
        : m_logger(IronMan::NullLogger::GetNullLogger())
        , m_bPrompt(true)
        , m_bDefined(false)
    {
    }

    // Unit tests Constructor
    FilesInUseSetting(const bool bPrompt, bool defined, ILogger& logger = IronMan::NullLogger::GetNullLogger())
        : m_bPrompt(bPrompt)
        , m_bDefined(defined)
        , m_logger(logger)
    {
    }

    // Returns true if FilesInUseSetting needs to be processed.
    const bool GetPrompt() const
    {
        return m_bPrompt;
    }

    // True if the element is authored in.
    const bool IsDefined() const
    {
        return m_bDefined;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::Certificate
// Contains data needed to represent a Certificate element in ParameterInfo.xml
// These entries typically contain a friendly Name and the Authority Key Identifier
// specifiying the root authorties the author wishes to accept, e.g. Microsoft Root
// though they can be augmented with a specific certificate thumbprint to make
// it more restrictive
//------------------------------------------------------------------------------
class Certificate
{
    static const DWORD SHA1_HASH_LEN = 20;
    ILogger& m_logger;
    CString m_friendlyName;
    CString m_authorityKeyIdentifier;
    BYTE m_rgbAuthorityKeyIdentifier[SHA1_HASH_LEN];
    CString m_SHA1Thumbprint;
    BYTE m_rgbSHA1Thumbprint[SHA1_HASH_LEN];
public:
    // Construct from XML
    Certificate(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_friendlyName(ElementUtils::GetAttributeByName(spElement,L"Name", logger) )
        , m_authorityKeyIdentifier(ElementUtils::GetAttributeByName(spElement,L"AuthorityKeyIdentifier", logger) )
        , m_SHA1Thumbprint(ElementUtils::GetOptionalAttributeByName(spElement, L"Thumbprint", logger) )
    {
        ElementUtils::VerifyName(spElement, L"Certificate", logger);
        InitializeByteValueFromStrings();
    }

    // Construct from strings
    Certificate(const CString & strFriendlyName
              , const CString & strAuthorityKeyIdentifier
              , const CString & strSHA1Thumbprint
                )
        :
          m_friendlyName(strFriendlyName)
        , m_authorityKeyIdentifier(strAuthorityKeyIdentifier)
        , m_SHA1Thumbprint(strSHA1Thumbprint)
        , m_logger(IronMan::NullLogger::GetNullLogger())
    {
        InitializeByteValueFromStrings();
    }

    // Get mandatory friendly name specified by author
    const CString& GetName() const
    {
        return m_friendlyName;
    }

    // Get Mandatory Authority Key Identifier - Identifies the public key that corresponds
    // to the private key used to sign the certificate or CRL. The identification is
    // based on the issuer's key identifier or on the issuer's name and serial number.
    // This is essentially the "Root Authority" for the signing chain
    const CString& GetAuthorityKeyIdentifierString() const
    {
        return m_authorityKeyIdentifier;
    }

    // Get Mandatory Authority Key Identifier as BYTEs
    const BYTE* GetAuthorityKeyIdentifier() const
    {
        return m_rgbAuthorityKeyIdentifier;
    }

    // Get optional Thumbprint of signing certificate, used if we wish to restrict acceptance to an individual certificate
    const CString& GetThumbprintString() const
    {
        return m_SHA1Thumbprint;
    }
    // Get optional Thumbprint BYTEs if we wish to restrict acceptance to an individual certificate
    const BYTE* GetThumbprint() const
    {
        return m_rgbSHA1Thumbprint;
    }

private:
    // Convert strings to byte values for initialization
    void InitializeByteValueFromStrings()
    {
        memset(m_rgbAuthorityKeyIdentifier, 0, sizeof(m_rgbAuthorityKeyIdentifier));
        (void)StrHexDecode((LPCWSTR)m_authorityKeyIdentifier, m_rgbAuthorityKeyIdentifier, sizeof(m_rgbAuthorityKeyIdentifier));
        memset(m_rgbSHA1Thumbprint, 0, sizeof(m_rgbSHA1Thumbprint));
        (void)StrHexDecode((LPCWSTR)m_SHA1Thumbprint, m_rgbSHA1Thumbprint, sizeof(m_rgbSHA1Thumbprint));
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::AcceptableCertificates
// 
// This class represents the AcceptableCertificates element ParameterInfo.xml
//------------------------------------------------------------------------------
class AcceptableCertificates
{
    ILogger& m_logger;
    CSimpleArray<Certificate> m_acceptableCertificates;
public:
    // Construct from XML
    AcceptableCertificates(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
    {
        if ( spElement )
        {
            ElementUtils::VerifyName(spElement, L"AcceptableCertificates", logger);

            CComPtr<IXMLDOMNode> spChild;
            HRESULT hr = spElement->get_firstChild(&spChild);
            if (S_OK == hr)
            {
                if (!ElementUtils::IsNodeComment(spChild))
                {
                    Certificate newCertificate(CComQIPtr<IXMLDOMElement>(spChild), logger);
                    LOG(logger, ILogger::Verbose, L"Acceptable Certificate added: " + newCertificate.GetName() );
                    m_acceptableCertificates.Add(newCertificate);
                }
                do {
                    CComPtr<IXMLDOMNode> spSibling;
                    hr = spChild->get_nextSibling(&spSibling);
                    if (S_OK == hr)
                    {
                        if (!ElementUtils::IsNodeComment(spSibling))
                        {
                            Certificate newCertificate(CComQIPtr<IXMLDOMElement>(spSibling), logger);
                            LOG(logger, ILogger::Verbose, L"Acceptable Certificate added: " + newCertificate.GetName() );
                            m_acceptableCertificates.Add(newCertificate);
                        }
                    }
                    spChild = spSibling;
                } while (!!spChild);
            }

            // There must be at least one Certificate child element if the AcceptableCertificates is specified
            if (0 == m_acceptableCertificates.GetSize() )
            {
                CInvalidXmlException ixe(L"The AcceptableCertificates block has no Certificate specified - either add them or remove the AcceptableCertificates block");
                throw ixe;
            }
        }

        if (0 == m_acceptableCertificates.GetSize() )
        {
            LOG(logger, ILogger::Verbose, L"No AcceptableCertificates block was specified");
        }
    }

    // Default constructor
    AcceptableCertificates()
        : m_logger(IronMan::NullLogger::GetNullLogger())
    {
    }

    // Operator =
    AcceptableCertificates& operator=(const AcceptableCertificates& rhs)
    {
        if (this != &rhs)
        {
            m_logger = rhs.m_logger;
            m_acceptableCertificates = rhs.m_acceptableCertificates;
        }

        return *this;
    }

    // Return number of certificates, zero or more
    int GetSize() const
    {
        return m_acceptableCertificates.GetSize();
    }

    // Return the certificate at specified index
    Certificate operator[] (DWORD nIndex) const
    {
        return m_acceptableCertificates[nIndex];
    }

    // Are any certificates defined?
    const bool IsDefined() const
    {
        return 0 < GetSize();
    }

    // Add a certificate to the array
    BOOL Add(const Certificate &newCertificate)
    {
        return m_acceptableCertificates.Add(newCertificate);
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::Configuration
// To Customize the engine.
//------------------------------------------------------------------------------
class Configuration
{
    ILogger& m_logger;
    DisabledCommandLineSwitches m_disabledCommandLineSwitches;
    AdditionalCommandLineSwitches m_additionalCommandLineSwitches;
    UserExperienceDataCollection m_userExperienceDataCollection;
    DownloadInstallSetting m_downloadInstallSetting;
    BlockingMutex m_blockingMutex;
    FilesInUseSetting m_FilesInUseSetting;
    static AcceptableCertificates m_acceptableCertificates;

public:
    Configuration(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
        : m_logger(logger)
        , m_disabledCommandLineSwitches(GetDisabledCommandLineSwitches( ElementUtils::FindOptionalChildElementByName(spElement, L"DisabledCommandLineSwitches", logger), logger) )
        , m_additionalCommandLineSwitches(GetAdditionalCommandLineSwitches( ElementUtils::FindOptionalChildElementByName(spElement, L"AdditionalCommandLineSwitches", logger), logger) )
        , m_userExperienceDataCollection(GetUserExperienceDataCollection( ElementUtils::FindOptionalChildElementByName(spElement, L"UserExperienceDataCollection", logger), logger) )
        , m_downloadInstallSetting(GetDownloadInstallSetting( ElementUtils::FindOptionalChildElementByName(spElement, L"DownloadInstallSetting", logger), logger) )
        , m_blockingMutex(GetBlockingMutex(ElementUtils::FindOptionalChildElementByName(spElement, L"BlockingMutex", logger), logger) )
        , m_FilesInUseSetting(GetFilesInUseSetting(ElementUtils::FindOptionalChildElementByName(spElement,L"FilesInUseSetting",logger), logger) )
    {        
        //If Configuration element is defined, there must be at least one child element
        if (spElement)
        {
            if (!IsDefined())
            {
                CInvalidXmlException ixe(L"schema validation failure: there must be a valid child element for Configuration.");
                LOG(m_logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }

            // Get acceptable certificates from ParameterInfo.xml configuration section and store them away
            GetAcceptableCertificates( ElementUtils::FindOptionalChildElementByName(spElement, L"AcceptableCertificates", logger), logger);
        }
    }

    const DisabledCommandLineSwitches& GetDisabledCommandLineSwitches() const
    {
        return m_disabledCommandLineSwitches;
    }

    const AdditionalCommandLineSwitches& GetAdditionalCommandLineSwitches() const
    {
        return m_additionalCommandLineSwitches;
    }

    const UserExperienceDataCollection& GetUserExperienceDataCollectionData() const
    {
        return m_userExperienceDataCollection;
    }

    const BlockingMutex& GetBlockingMutex() const
    {
        return m_blockingMutex;
    }

    const FilesInUseSetting& GetFilesInUseSetting() const
    {
        return m_FilesInUseSetting;
    }

    const AcceptableCertificates& GetAcceptableCertificates() const
    {
        return m_acceptableCertificates;
    }

    bool IsDefined() const
    {
        return m_disabledCommandLineSwitches.IsDefined()
            || m_additionalCommandLineSwitches.IsDefined()
            || m_userExperienceDataCollection.IsDefined()
            || m_downloadInstallSetting.IsDefined()
            || m_acceptableCertificates.IsDefined()
            || m_blockingMutex.IsDefined();
    }

    bool IsSimultaneousDownloadAndInstallDisabled() const
    {
        return (m_downloadInstallSetting.GetSetting() == DownloadInstallSetting::Serial);
    }

private:
    static DisabledCommandLineSwitches GetDisabledCommandLineSwitches(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement)
        {
            return DisabledCommandLineSwitches( spElement, logger);
        }
        return DisabledCommandLineSwitches();
    }

    static AdditionalCommandLineSwitches GetAdditionalCommandLineSwitches(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement)
        {
            return AdditionalCommandLineSwitches( spElement, logger);
        }
        return AdditionalCommandLineSwitches();
    }

    static UserExperienceDataCollection GetUserExperienceDataCollection(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement)
        {
            return UserExperienceDataCollection( spElement, logger);
        }
        return UserExperienceDataCollection();
    }

    static DownloadInstallSetting GetDownloadInstallSetting(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement)
            return DownloadInstallSetting(spElement, logger);
        return DownloadInstallSetting();
    }

    // Helper method to create BlockingMutex.
    // If element is not authored, returns default undefined object.
    static BlockingMutex GetBlockingMutex(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement)
            return BlockingMutex(spElement, logger);
        return BlockingMutex();
    }

    // Helper method to create FilesInUseSetting.
    // If element is not authored, returns default undefined object.
    static FilesInUseSetting GetFilesInUseSetting(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement)
            return FilesInUseSetting(spElement, logger);
        return FilesInUseSetting();
    }
    

public:
    static AcceptableCertificates & GetAcceptableCertificates(CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement)
        {
            m_acceptableCertificates = AcceptableCertificates( spElement, logger);
        }

        return m_acceptableCertificates;
    }
};

} // namespace IronMan
