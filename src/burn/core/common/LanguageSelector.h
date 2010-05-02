//-------------------------------------------------------------------------------------------------
// <copyright file="LanguageSelector.h" company="Microsoft">
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

class ThreadLanguageSelector
{
public:
    static LCID GetLocaleId()
    {
        return ::GetThreadLocale();
    }
    static unsigned int GetLangId()
    {
        return LANGIDFROMLCID(GetLocaleId());
    }
    static bool PrimaryLangIdMatches(unsigned int langId)
    {
        return PrimaryLangIdMatches(GetLocaleId(), langId);
    }
    static bool PrimaryLangIdMatches(LCID localeId, unsigned int langId)
    {
        LCID otherLcid = static_cast<LCID>(langId); // LCID is a WORD

        return PRIMARYLANGID(localeId) == PRIMARYLANGID(otherLcid);
    }
    static bool LangIdMatchesExactly(unsigned int langId)
    {
        return GetLangId() == langId;
    }
    static bool LangIdMatchesExactly(LCID localeId, unsigned int langId)
    {
        return LANGIDFROMLCID(localeId) == langId;
    }

};

}
