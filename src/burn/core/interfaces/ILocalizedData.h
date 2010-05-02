//-------------------------------------------------------------------------------------------------
// <copyright file="ILocalizedData.h" company="Microsoft">
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


// Interface that defines the method, that can be use to get localized text.
struct ILocalizedData
{
    // GetLocalizedText() return the localized text for the given locID string
    virtual const CString GetLocalizedText(const CString& locID) const = 0;  

    virtual ~ILocalizedData() {}
};



// Interface that defines methods to: 
//      Create a LocalizedData object, 
//      Validate that localized text for all loc-ids are defined in the LocalizedData.xml file
//      Compute & hold on to the UILanguage that will be used
struct ILocalizedDataProvider
{
    // Returns a LocalizedData object that the UI Language. Creates the LocalizedData object on first call.
    virtual ILocalizedData& GetLocalizedData(void) = 0;

    // Verifys that localized text for all loc-ids are defined in the LocalizedData.xml file
    virtual bool Validate(void) = 0;

    // Return the UI language that will be used. Computes on first call.
    virtual LCID GetUILanguage(void) = 0;

    virtual ~ILocalizedDataProvider() {}
};



class NullLocalizedData : private ILocalizedData
{
public:
    virtual const CString GetLocalizedText(const CString& text) const
    {
        return text;
    }
    ~NullLocalizedData()
    {
    }

    static ILocalizedData& GetNullLocalizedData()
    {
        static NullLocalizedData nld;
        return nld;
    }
};

struct NullLocalizedDataProvider : public ILocalizedDataProvider
{
    virtual ILocalizedData& GetLocalizedData(void)
    {
        return NullLocalizedData::GetNullLocalizedData();
    }
    virtual bool Validate(void)
    {
        return true;
    }
    virtual LCID GetUILanguage(void)
    {
        return 1033;
    }
    ~NullLocalizedDataProvider()
    {
    }

    // Static creator
    static ILocalizedDataProvider& GetNullLocalizedDataProvider()
    {
        static NullLocalizedDataProvider nldp;
        return nldp;
    }
};


} // namespace IronMan
