//-------------------------------------------------------------------------------------------------
// <copyright file="ValidCertificate.h" company="Microsoft">
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

#include "schema\EngineData.h"
#include "schema\LocalizedData.h"
#include "CmdLineParser.h"
#include "ModuleUtils.h"
#include "common\logutils.h"

namespace IronMan
{
//------------------------------------------------------------------------------
// Class: IronMan::ValidateSemantics
// This iterates through the UiData and EngineData and verifies
// that the files that are supposed to be in the disk are actually in the
// package.  This is to ensure we don't ship a package that is missing any
// resources
//------------------------------------------------------------------------------
class ValidateSemantics
{
public:
    //This function throws when the following conditions are true
    // a. The file to verify does not exist.
    // b. BOM does not exist.
    static void IsFileUTF16WithBOM(LPCWSTR path, ILogger& logger)
    {
        CAtlFile file;
        //Open with FILE_SHARE_READ to prevent race condition failure.
        HRESULT hr = file.Create(ModuleUtils::FullPath(path, logger), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
        if (FAILED(hr))
        {
            CString str;
            str.Format(L"File %s could not be opened for read", CString(path));
            LOG(logger, ILogger::Error, str);
            CNotFoundException nfe(str);
            throw nfe;
        }

        const DWORD size = 2;
        BYTE buffer[size];
        DWORD read = 0;
        file.Read(buffer, size, read);

        if (!((buffer[0] == 0xFF) && (buffer[1] == 0xFE)))
        {
            CString csExceptionMessage;
            csExceptionMessage.Format(L"File %s is not UTF-16 with Byte Order Marks (BOM)",path);
            LOG(logger, ILogger::Error, csExceptionMessage);
            CInvalidXmlException ixe(csExceptionMessage);
            throw ixe;
        }
    }

    //-------------------------------------------------------------------------
    // VerifyTokensAreUsedCorrectlyInParameterInfoFile
    // This function throws if
    // 1. No UI element
    // 2. UI element contains "#(loc." in any attribute
    // 3. The UI element does not have a closing tag.
    // 4. Any BlockIf/@ID contains "#(loc."
    // UI/@Name and BlockIf/@ID cannot be localized, since they are used by user experience data
    //-------------------------------------------------------------------------
    static void VerifyTokensAreUsedCorrectlyInParameterInfoFile(CPath path, ILogger& logger)
    {
        const CComBSTR bstrXML(XmlUtils::ReadXml(path, logger));
        ValidateSemantics::IsTokenDefinedInUIElement(CString(bstrXML), logger);
        ValidateSemantics::IsTokenDefinedInAnyBlockIfIdAttribute(bstrXML, logger);
    }

    //-------------------------------------------------------------------------
    // IsTokenDefinedInUIElement
    // This function throws if the UI element has any attributes that
    // contains "#(loc."
    // UI/@Name cannot be localized, since it is used by user experience data
    //-------------------------------------------------------------------------
    static void IsTokenDefinedInUIElement(const CString& strXml, ILogger& logger)
    {
        int iStart = strXml.Find(L"<UI ", 0);
        if (iStart < 0)
        {
            CString str = L"Missing UI element in parameterinfo.xml";
            LOG(logger, ILogger::Error, str);
            CInvalidXmlException ixe(str);
            throw ixe;
        }

        int iEnd = strXml.Find(L">", iStart);
        if (iEnd < 0)
        {
            CString str = L"Missing closing > for UI element in parameterinfo.xml";
            LOG(logger, ILogger::Error, str);
            CInvalidXmlException ixe(str);
            throw ixe;
        }

        int iLocToken = strXml.Find(L"#(loc.", iStart);
        if ((iLocToken < iEnd) && (iLocToken != -1))
        {
            CString str = L"UI element in parameterinfo.xml cannot contain any token (#(loc.[Name]) reference.";
            LOG(logger, ILogger::Error, str);
            CInvalidXmlException ixe(str);
            throw ixe;
        }
    }

    //-------------------------------------------------------------------------
    // IsTokenDefinedInAnyBlockIfIdAttribute
    // This function throws if the ID attribute of any BlockIf element
    // contains "#(loc."
    // BlockIf/@ID cannot be localized, since it is used by user experience data to
    // bucket when a BlockIf is hit
    //-------------------------------------------------------------------------
    static void IsTokenDefinedInAnyBlockIfIdAttribute(const CComBSTR& bstrXML, ILogger& logger)
    {
        // Load the XML
        XmlDoc doc(NullLocalizedData::GetNullLocalizedData(), logger);
        HRESULT hr = doc.LoadXML(bstrXML);
        if (S_OK == hr)
        {
            try
            {
                // create an XPath query that gets the BlockIf elements that contain an ID attribute
                XmlNodeList nodeList = XmlNodeList::SelectNodes(L"//BlockIf[@ID]", doc.GetDocElement());
                for(long nodeIndex=0; nodeIndex < nodeList.Length(); ++nodeIndex)
                {
                    // Verify the there is not a #(loc. in the BlockIf/@ID
                    CString strId = nodeList[nodeIndex].OptionalAttribute(L"ID");
                    int iLocToken = strId.Find(L"#(loc.");
                    if ( -1!= iLocToken )
                    {
                        // There is a #(loc. in the BlockIf/@ID, throw error
                        CString str = L"BlockIf/@ID cannot contain any token (#(loc.[Name]) references. BlockIf/@ID=\"" + strId + L"\"";
                        CInvalidXmlException ixe(str);
                        throw ixe;
                    }
                }
                // XML loaded successfully and there were no tokens defined in any BlockIf/@ID
            }
            catch(const CException& e)
            {
                LOG(logger, ILogger::Error, e.GetMessage());
                throw;
            }
            catch(...)
            {
                LOG(logger, ILogger::Error, L"unknown exception thrown, caught and about to be rethrown.");
                throw;
            }
        }
    }

public:
    static bool ParameterInfoSemanticChecker(const EngineData& engineData, ILogger& logger)
    {
        return ValidateNameAttributeInUIElementIsValid(engineData.GetUi().GetName(), logger);
    }

public:
    static void OutputErrorMessage(ILogger& logger, ILogger::LoggingLevel level = ILogger::Error)
    {
        CString executableNameWithoutExtension = ModuleUtils::GetProgramName();
        CString errorMessage = executableNameWithoutExtension + L" encountered an unexpected error in the package contents.\r\n" +
            L"The package will not be installed.\r\n" +
            L"Please contact your vendor for a new package.";
        LOG(logger, level, errorMessage);

        CCmdLineSwitches switches;
        if (!switches.QuietMode() && !switches.PassiveMode() )
        {
            ::MessageBox(NULL, errorMessage, executableNameWithoutExtension, MB_OK | MB_ICONERROR);
        }
    }

private:
    //Allow only valid name in the UI element. 
    static bool ValidateNameAttributeInUIElementIsValid(const CString &name, ILogger& logger)
    {
        bool bValidate = true;
        WCHAR invalidChar[11] = {'/', '\\', '?', '%', '*', ':', '|', '"', '<', '>'};
        for (int i=0; i < 10; ++i)
        {
            if (-1 != name.Find(invalidChar[i]))
            {
                LOGEX(logger, ILogger::Error, L"The attribute name in the UI element contains invalid character.");
                bValidate = false;
                break;
            }
        }
        return bValidate;
    }

    static bool ValidateItemsExistIfNoURL(const Items& items, ILogger& logger)
    {
        bool bValidate = true;
        for (unsigned int i = 0; i < items.GetCount(); ++i)
        {
            // Special case for Patches Item
            if ( ItemBase::Patches == items.GetItem(i)->GetType() )
            {
                const Patches* patches = dynamic_cast<const Patches*>(items.GetItem(i));
                for (unsigned int j = 0; j < patches->GetCount(); ++j)
                {
                    if ( !ValidateItemExistIfNoURL(&(dynamic_cast<const ItemBase&>(patches->GetMsp(j))), logger) )
                    {
                        bValidate = false;
                    }
                }
            }
            else
            {
                if ( !ValidateItemExistIfNoURL(items.GetItem(i), logger) )
                {
                    bValidate = false;
                }
            }
        }
        return bValidate;
    }

    static bool ValidateItemExistIfNoURL(const ItemBase* item, ILogger& logger)
    {
        // only check if item exists on disk if it does not have a URL associated with it
        const DownloadPath* pDownloadable = dynamic_cast<const DownloadPath*>(item);
        if ( (NULL != pDownloadable) && (pDownloadable->GetUrl().IsEmpty()) )
        {
            const LocalPath* pLocalPath = dynamic_cast<const LocalPath*>(item);

            if (NULL != pLocalPath)
            {
                CPath fullPath;
                if (CPath(pLocalPath->GetName()).IsRelative())
                {
                    fullPath.Combine(ModuleUtils::GetIncludedPayloadsFolderPath(), pLocalPath->GetName());
                }
                else 
                {
                    fullPath = CPath(pLocalPath->GetName());
                }

                if (!fullPath.FileExists())
                {
                    const Exe* exe = dynamic_cast<const Exe *>(item);
                    if (NULL != exe)
                    {
                        // We should defer the validation of local exe as it is too soon here
                        // and we will block CreateLayout scenario if not.
                        if (exe->GetExeType().IsLocalExe())
                            return true;
                    }

                    CString errorMessage(L"Package authoring error. The Url for this item is not authored and the item does not exist locally: ");
                    LOG(logger, ILogger::Error, errorMessage + fullPath.m_strPath );
                    return false;
                }
            }
            else
            {
                LOG(logger, ILogger::Error, L"pLocalPath is NULL!!!!!!");
                return false;
            }
        }
        return true;
    }

    static void LogErrorMessage(CString missingResource, unsigned int langId, ILogger& logger)
    {
        CString errorMessage;
        errorMessage.AppendFormat(L"Missing resource for %u: ", langId);
        errorMessage.Append(missingResource);
        LOG(logger, ILogger::Error, errorMessage );

    }
};

}
