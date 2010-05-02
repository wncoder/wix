//-------------------------------------------------------------------------------------------------
// <copyright file="CmdLineParser.h" company="Microsoft">
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

//------------------------------------------------------------------------------
// Command line parser and command line switches classes.
//
// This file contains the classes and functions used to parse the command line
// and get information about the command line switches. It also validates the
// switches and their arguments, if any.
//
// This file also has the "master list" of all the recognized switches
// Search for "** Master list of switches **" below.
//
//-------------------------------------------------------------------------------

namespace IronMan
{
    class CCmdLineParser
    {
        CString m_csCmdLine;
    public:
        CCmdLineParser(LPCTSTR lpCmdLine) : m_csCmdLine(lpCmdLine) {}
        virtual ~CCmdLineParser() {}

        bool ContainsOption(LPCTSTR strOptionToFind, int *piOffsetOfOption = NULL) const
        {
            bool fFoundOption = false;
            CString csOptionToFind(strOptionToFind);
            csOptionToFind.MakeUpper();
            CString csCmdLineUppercased(m_csCmdLine);
            csCmdLineUppercased.MakeUpper();

            int iOffsetOfOption = -1;
            // Check middle of line
            if (-1 != (iOffsetOfOption = csCmdLineUppercased.Find(_T(" /") + csOptionToFind + _T(" ")))   ||
                -1 != (iOffsetOfOption = csCmdLineUppercased.Find(_T(" -") + csOptionToFind + _T(" ")))      )
            {
                iOffsetOfOption += 2; // for the space and / or - 
                fFoundOption =  true;
            }
            else
            // Check end of line
            if ( (csCmdLineUppercased.Right(1 + csOptionToFind.GetLength()) == _T("/") + csOptionToFind)  ||
                 (csCmdLineUppercased.Right(1 + csOptionToFind.GetLength()) == _T("-") + csOptionToFind)    )
            {
                iOffsetOfOption = csCmdLineUppercased.GetLength() - (1 + csOptionToFind.GetLength());
                fFoundOption =  true;
            }

            if (piOffsetOfOption != NULL)
            {
                *piOffsetOfOption = iOffsetOfOption;
            }

            return fFoundOption;
        }

        // Get the option value associated with an option keyword, handles quoted strings
        // and understands "" to stand for " inside quoted strings. The return value
        // will have the enclsiing quotes stripped and the double quotes changed to single,
        // so "PROPERTY=""String""" becomes PROPERTY="String"
        CString GetOptionValue(LPCTSTR strOptionToFind) const
        {
            int iOffsetOfOption = 0;
            CString strOption(strOptionToFind);
            CString strValue;
            bool fFoundOption = ContainsOption(strOptionToFind,&iOffsetOfOption);
            if (fFoundOption)
            {
                strValue = m_csCmdLine.Mid(iOffsetOfOption + strOption.GetLength() + 1);
                strValue.Trim();
                // Check for "quoted argument"
                bool fIsQuoted = (0 == strValue.Left(1).Find(L"\""));
                if (fIsQuoted)
                {
                    // Strip leading quote
                    strValue = strValue.Mid(1);
                    int iQuotePos = 0;

                    while ( (iQuotePos < strValue.GetLength())  && (iQuotePos = strValue.Find(L"\"",iQuotePos)) > 0)
                    {
                        if (strValue.Find(L"\"\"",iQuotePos) == iQuotePos )
                        {
                            // double quote so move on past both quotes
                            iQuotePos += 2;
                        }
                        else
                        {
                            // Not a double quote so this is final single quote or end of string
                            break;
                        }
                    }

                    if ( iQuotePos > 0)
                    {
                        strValue = strValue.Left(iQuotePos);
                        // Change double quotes to single quotes
                        strValue.Replace(L"\"\"",L"\"");
                    }
                }
                else
                {
                    int iNextSpace = strValue.FindOneOf(L" \t");
                    if ( iNextSpace != -1 )
                    {
                        strValue = strValue.Left(iNextSpace);
                    }
                }
            }

            return strValue;
        }

        // Get the command line
        const CString & GetCommandLine(void) const
        {
            return m_csCmdLine;
        }
    };

    class CCmdLineSwitches
    {
        CCmdLineParser m_cmdLineParser;
        CSimpleArray<CString> m_rgstrKnownSwitches;
    public:
        CCmdLineSwitches(LPCTSTR szCmdLineIncludingExe = ::GetCommandLine()) : m_cmdLineParser(szCmdLineIncludingExe)
        {
            // All known command line switches should added here
            // ** Master list of switches **
            if (m_rgstrKnownSwitches.GetSize() == 0)
            {
                m_rgstrKnownSwitches.Add(L"CEIPconsent");
                m_rgstrKnownSwitches.Add(L"chainingpackage");
                m_rgstrKnownSwitches.Add(L"createlayout"); 
                m_rgstrKnownSwitches.Add(L"lcid");
                m_rgstrKnownSwitches.Add(L"log");
                m_rgstrKnownSwitches.Add(L"msioptions");
                m_rgstrKnownSwitches.Add(L"norestart");
                m_rgstrKnownSwitches.Add(L"passive");
                m_rgstrKnownSwitches.Add(L"showfinalerror");
                m_rgstrKnownSwitches.Add(L"pipe");
                m_rgstrKnownSwitches.Add(L"promptrestart");
                m_rgstrKnownSwitches.Add(L"forcerestart");
                m_rgstrKnownSwitches.Add(L"q");
                m_rgstrKnownSwitches.Add(L"repair");
                m_rgstrKnownSwitches.Add(L"serialdownload");
                m_rgstrKnownSwitches.Add(L"uninstall");
                m_rgstrKnownSwitches.Add(L"NoSetupVersionCheck");
                m_rgstrKnownSwitches.Add(L"uninstallpatch");
                m_rgstrKnownSwitches.Add(L"burn.elevated");
                m_rgstrKnownSwitches.Add(L"UnRegister");
            }
        }

        virtual ~CCmdLineSwitches()
        {
        }

        // Mode in which UI is suppressed, for admin installs
        bool QuietMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("q")) || 
                   m_cmdLineParser.ContainsOption(_T("quiet"));
        }

        // Returns true if passive install required, i.e. progress bar but no user interaction
        bool PassiveMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("passive"));
        }

        //Return true if it is not in silent, quiet mode or the /UnRegister is not passed in
        bool InteractiveMode() const
        {
            return !(QuietMode() || PassiveMode() || UnRegisterSwitchPresent());
        }

        //------------------------------------------------------------------------------
        // ShowFinalError
        // Returns true if ShowFinalError is required, i.e. during passive mode the
        // Finish Page will be displayed if the install was not successfull
        //------------------------------------------------------------------------------
        bool ShowFinalError() const
        {
            return m_cmdLineParser.ContainsOption(_T("showfinalerror"));
        }

        // If any of the patches required reboot, setup should neither prompt nor cause a reboot
        bool NoRestartMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("norestart"));
        }

        // If any of the patches required reboot, setup should prompt for reboot after install, and trigger reboot if user agrees
        bool PromptForRestartMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("promptrestart"));
        }

        // Setup should force a restart on completion irrespective of whether it was requested by any packages
        bool ForceRestartMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("forcerestart"));
        }

        // Should uninstall the payloads
        // Should be in UninstallMode if either /uninstall switch is passed in  or the /UninstallPatch switch is passed in
        bool UninstallMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("uninstall")) || m_cmdLineParser.ContainsOption(_T("uninstallpatch"));
        }

        // Start up in maintenance mode irrespective of the ARPIf condition in the ParameterInfo.xml file
        bool MaintenanceMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("repair"));
        }

        // Do not perform SetupVersion check.
        bool NoSetupVersionCheck() const
        {
            return m_cmdLineParser.ContainsOption(_T("NoSetupVersionCheck"));
        }

        // Users wants help message
        bool RequireHelp() const
        {
            bool bWantHelp = m_cmdLineParser.ContainsOption(_T("?"))  ||
                             m_cmdLineParser.ContainsOption(_T("h"))  ||
                             m_cmdLineParser.ContainsOption(_T("help"));
            return bWantHelp;
        }

        //------------------------------------------------------------------------------
        // RequireHelpDueToInvalidSwitchDependency
        // Returns true when the command line is incorrect due do dependencies between
        // two or more switches
        //------------------------------------------------------------------------------
        bool RequireHelpDueToInvalidSwitchDependency() const
        {
            if (   (ShowFinalError() && !PassiveMode()) // ShowFinalError requires PassiveMode
                || (QuietMode() && PassiveMode())  // Quiet cannot be passed with passive
                || (UninstallMode() && MaintenanceMode())  // Repair cannot be passed in with Uninstall or UninstallPatch
                || (NoRestartMode() && PromptForRestartMode())  // NoRestart cannot be passed in with PromptForRestart
                || (CreateLayoutSwitchPresent() && (UninstallMode() || MaintenanceMode()))
                )
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        //------------------------------------------------------------------------------
        // NeedToShowHelpDialog
        // Returns true when the the help dialog needs to be show either for
        // If help required, display that and exit
        // If invalid dependend switches are passed in, display help and exit.
        // If a disabled commandline switch is specified, display help and exit.
        //------------------------------------------------------------------------------
        bool NeedToShowHelpDialog(const bool bHelpStringIllegalCommandLineSwitchesAreUsed) const
        {
            return RequireHelp()
                || RequireHelpDueToInvalidSwitchDependency()
                || bHelpStringIllegalCommandLineSwitchesAreUsed;
        }

        // Fakes the progress bar but doesn't really install anything
        bool DemoMode() const
        {
            return m_cmdLineParser.ContainsOption(_T("demo"));
        }

        // Returns the patch file location, e.g. C:\someplace\subfolder\patch.msp, or empty string if not specified
        CString GetPatchFileSpecification() const
        {
            const CString strPatchFileSpecOption(L"patch");
            CString strPatchFileSpec = m_cmdLineParser.GetOptionValue(strPatchFileSpecOption);
            return strPatchFileSpec;
        }

        // Gets the options to be passed to the Msi operations, e.g. /L*v
        CString GetMsiOptions() const
        {
            const CString strMsiOptionsOption(L"msioptions");
            CString strMsiOptions = m_cmdLineParser.GetOptionValue(strMsiOptionsOption);
            return strMsiOptions;
        }

        // Returns the patch file location, e.g. C:\someplace\subfolder\\logs\logfile.log, or empty string if not specified
        CString GetLogFileSpecification() const
        {
            const CString strLogFileSpecOption(L"log");
            CString strLogFileSpec = m_cmdLineParser.GetOptionValue(strLogFileSpecOption);
            return strLogFileSpec;
        }

        // Returns the SourceLocation path if specified
        CString GetSourceLocation() const
        {
            return m_cmdLineParser.GetOptionValue(L"SourceLocation");
        }

        // returns name of named Pipe for control by external chainer
        CString GetPipeName() const
        {
            return m_cmdLineParser.GetOptionValue(L"pipe");
        }

        // returns name of named Elevated Pipe for control by external chainer
        CString GetElevationPipeName() const
        {
            return m_cmdLineParser.GetOptionValue(L"burn.elevated");
        }

        // Get the folder spcified for the layout creation
        CString CreateLayout() const
        {
            return m_cmdLineParser.GetOptionValue(L"createlayout");
        }

        // Get the patch code to uninstall
        CString UninstallPatch() const
        {
            return m_cmdLineParser.GetOptionValue(L"uninstallpatch");
        }

        // Get the value passed to the LCID switch as a string
        CString GetLCID() const
        {
            return m_cmdLineParser.GetOptionValue(L"lcid");
        }

        // Get the value passed to the LCID switch as an LCID instead of a string
        LCID GetLCIDValue() const
        {
            LCID lcid = 0;
            CString strLCID = GetLCID();
            int radix = 10;
            if ( (strLCID.Find(L'x') >= 0) || (strLCID.Find(L'X') >= 0))
            {
                radix = 16;
            }

            lcid = (LCID)wcstoul(strLCID,0,radix);
            return lcid;
        }

        const CSimpleArray<CString> & GetKnownSwitches(void)
        {
            return m_rgstrKnownSwitches;
        }

        bool OptionValuesAreGood(CString & strDescription) const
        {
            bool fIsBad = false;

            // Check /LCID has numeric value passed with it
            if (m_cmdLineParser.ContainsOption(L"lcid")&& (GetLCIDValue() == 0))
            {
                strDescription.Append(L"The /LCID switch requires a numeric value. ");
                fIsBad = true;
            }

            // If creating a layout, verify that option value is not empty
            // If creating a layout, verify that the path is not a relative path
            // Relative path for the layout is not supported
            if ( CreateLayoutSwitchPresent() )
            {
                if ( CPath(CreateLayout()).IsRelative() )
                {
                    strDescription.Append(L"The /createlayout switch requires an absolute folder path. ");
                    fIsBad = true;
                }
                else
                {
                    fIsBad |= HasBadOrNoParameter(L"createlayout",L"folder path",strDescription);
                }
            }

            // If /pipe is specified it has to have a name 
            fIsBad |= HasBadOrNoParameter(L"pipe",L"name",strDescription);

            // If /burn.elevated is specified it has to have a name
            fIsBad |= HasBadOrNoParameter(L"burn.elevated",L"name",strDescription);

            // If /chainingpackage is specified it has to have a name
            fIsBad |= HasBadOrNoParameter(L"chainingpackage",L"name",strDescription);

            // If /msioptions is specified it has to have an options string 
            fIsBad |= HasBadOrNoParameter(L"msioptions",L"options string",strDescription);

            // If /log is specified it has to have a  file specification
            fIsBad |= HasBadOrNoParameter(L"log",L"file specification",strDescription,false /* Leading slash is valid */);

            // If /UninstallPatch is specified it has to have a Patch Code
            fIsBad |= HasBadOrNoParameter(L"uninstallpatch",L"patch code",strDescription);

            return !fIsBad;
        }

        // If the AdditionalCommandLineSwitches was specified, ensure that all switches passed in are known
        // or are in the additional command line switches configuration section
        bool AllSwitchesAreValid(const CSimpleArray<CString> & rgstrAdditionalCommandLineSwitches, CString & strUnrecognizedSwitches)
        {
            bool fOnlyRecognizedSwitchesPresent = true;
            int iNumAdditionalSwitches= rgstrAdditionalCommandLineSwitches.GetSize();
            int iNumKnownSwitches= m_rgstrKnownSwitches.GetSize();
            int curPos = 0;
            
            int numArgs=0;
            LPWSTR* args = ::CommandLineToArgvW(m_cmdLineParser.GetCommandLine(), &numArgs);

            if (NULL == args)
            {
                return false;
            }

            if (numArgs == 1)
            {
                ::LocalFree(args);
                return true;
            }

            // Loop over other command line items
            for (int i=1; i<numArgs; ++i)
            {
                bool fMatchesAKnownSwitch = false;
                bool fMatchesAnAdditionalSwitch = false;
                bool fIsASwitch = (args[i][0] == L'/' || args[i][0] == L'-');

                CString strPossibleSwitch = args[i];
                if (fIsASwitch)
                {
                    strPossibleSwitch = strPossibleSwitch.Mid(1);
                }
                
                if (m_cmdLineParser.ContainsOption(strPossibleSwitch))
                {
                    // Check against all known switches
                    for (int i = 0; i < iNumKnownSwitches; ++i)
                    {
                        if (0 == strPossibleSwitch.CompareNoCase(m_rgstrKnownSwitches[i]))
                        {
                            fMatchesAKnownSwitch = true;
                            break;
                        }
                    }

                    if (!fMatchesAKnownSwitch)
                    {
                        // Check against all additional command line switches, if applicable
                        for (int i = 0; i < iNumAdditionalSwitches; ++i)
                        {
                            if (0 == strPossibleSwitch.CompareNoCase(rgstrAdditionalCommandLineSwitches[i]))
                            {
                                fMatchesAnAdditionalSwitch = true;
                                break;
                            }
                        }
                    }
                }

                if (!fMatchesAKnownSwitch && !fMatchesAnAdditionalSwitch  && curPos >= 0)
                {
                    // Can only get here if the switch did not match a known or additional engine switch used by operands
                    if (!strUnrecognizedSwitches.IsEmpty())
                    {
                        strUnrecognizedSwitches += L" ";
                    }

                    // Add unrecognized switches and parameters to return string
                    bool bAddQuotes = (NULL != wcschr(args[i], L' '));
                    
                    if (bAddQuotes)
                    {
                        strUnrecognizedSwitches += L"\"";
                    }
                    strUnrecognizedSwitches += args[i];
                    if (bAddQuotes)
                    {
                        strUnrecognizedSwitches += L"\"";
                    }

                    if (fIsASwitch)
                    {
                        fOnlyRecognizedSwitchesPresent = false;
                    }
                }
            };

            ::LocalFree(args);

            return fOnlyRecognizedSwitchesPresent;
        }

        /*If /serialdownload is passed in via command line, this overrides the authoring 
        and the default behavior of simultaneous download and install. Only serial download 
        and install are performed.*/
        bool SerialDownloadSwitchPresent() const
        {
            return m_cmdLineParser.ContainsOption(L"serialdownload");
        }

        bool CreateLayoutSwitchPresent() const
        {
            return m_cmdLineParser.ContainsOption(L"createlayout");
        }

        bool CEIPconsentSwitchPresent() const
        {
            return m_cmdLineParser.ContainsOption(L"CEIPconsent");
        }

        // Determine if specified switch is present
        bool SwitchIsPresent(const CString &strSwitchToLookFor) const
        {
            return m_cmdLineParser.ContainsOption(strSwitchToLookFor);
        }

        // True when the process was launched with elevated credentials
        bool IsRunningElevated() const
        {
            return  m_cmdLineParser.ContainsOption(L"burn.elevated");
        }

        // True when the process was launched to run as server
        bool IsRunningServer() const
        {
            return  m_cmdLineParser.ContainsOption(L"burn.server");
        }

        // If the UnRegister switch is passed in, then the ARP entry will
        // be deleted and the Bundle cache will be deleted
        // Will be silent with no UI
        BOOL UnRegisterSwitchPresent() const
        {
            return  m_cmdLineParser.ContainsOption(L"UnRegister");
        }

        // Returns true if the Reg Key passed in contains the ParentBundleId
        // on the command line or no ParentBundleId was passed in with the /UnRegister switch
        CString UnRegisterBundleId() const
        {
            return m_cmdLineParser.GetOptionValue(L"UnRegister");
        }

    private:
        // If a switch needs a parameter call this to check if parameter is supplied
        bool HasBadOrNoParameter(const CString &switchName, const CString &parameterType, CString & strDescription, bool fCheckForLeadingSlash = true) const
        {
            bool fIsBad = false;

            // Only check for non-empty parameter if the switch is actually specified on the command line
            if ( m_cmdLineParser.ContainsOption(switchName) )
            {
                CString strOptionValue(m_cmdLineParser.GetOptionValue(switchName));

                // If it is specified then it should have a parameter after the switch
                if ( strOptionValue.IsEmpty() )
                {
                    strDescription.Append(CString(L"The /") + switchName +  + CString(L" switch requires a ") + parameterType + CString(L". ") );
                    fIsBad = true;
                }
                else if (fCheckForLeadingSlash)
                {
                    // Does it start with illlegal /
                    fIsBad = BeginsWithIllegalUnquotedForwardSlash(switchName,strOptionValue,parameterType,strDescription);
                }
            }

            return fIsBad;
        }

        // If a switch has a parameter call this to check that the parameter does not begin with forward slash / - if you really need to pass that then quote it "/likethis"
        // The idea of this check is to stop one switch swalling another, e.g. /chainingpackage /aububblesuppress could eat a switch intended to be passed on
        bool BeginsWithIllegalUnquotedForwardSlash(const CString &switchName, const CString &strOptionValue, const CString &parameterType, CString & strDescription) const
        {
            bool fIsBad = false;
            if ( strOptionValue.GetAt(0) == L'/' )
            {
                strDescription.Append(CString(L"The /") + switchName +  + CString(L" ") + parameterType + CString(L" parameter ") + strOptionValue + CString(L" cannot begin with a / character unless surrounded by double quotations \"\"/likethis\"\". ") );
                fIsBad = true;
            }

            return fIsBad;
        }
    };
}
