//-------------------------------------------------------------------------------------------------
// <copyright file="CustomError.h" company="Microsoft">
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

#include "..\interfaces\ILogger.h"

namespace IronMan
{
/*------------------------------------------------------------------------------
  This set of classes are written to express the author's intention for handling
  custom errors for each item.  For each error (CustomError), the author can map
  to one of the following:

  a. Success
  b. Failure
  c. Retry.

     In Retry mapping, the author can do one, or both of the following:
     a. Execute a helper item (CustomErrorItem)
     b. Re-install the current item.
   
  Overall Class diagram
  =====================

    CustomErrorItem
         /|\
          |(contains)
          |				 (Inherit)
    CustomErrorRetry	----------->  CustomErrorMappingBase
                                               /|\
                                                |(contains)
                                                |
                                            CustomError
                                               /|\
                                                | (contains)
                                                |
                                        CustomErrorHandling

------------------------------------------------------------------------------*/


//------------------------------------------------------------------------------
// Class: IronMan::CustomErrorItem
//------------------------------------------------------------------------------
class CustomErrorItem
{
    //Beware: When adding new variable, ensure that the copy constructor is updated as well.
    CString m_csName;
    CString m_csArgument;
    CString m_csLogfile;

public:
    // Constructor
    CustomErrorItem(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_csName(GetName(ElementUtils::GetAttributeByName(spElement, L"Name", logger), logger))
        , m_csArgument(ElementUtils::GetOptionalAttributeByName(spElement, L"CommandLine", logger))
        , m_csLogfile(ElementUtils::GetOptionalAttributeByName(spElement, L"LogFile", logger))
    {
        ElementUtils::VerifyName(spElement, L"ItemRef", logger);
    }

    //Copy Constructor
    CustomErrorItem(const CustomErrorItem& rhs)
        : m_csName(rhs.m_csName)
        , m_csArgument(rhs.m_csArgument)
        , m_csLogfile(rhs.m_csLogfile)
    {		
    }
    
    // Destructor
    virtual ~CustomErrorItem()
    {
    }

    // Returns the subitem, e.g. RetryHelper, name
    const CString GetItemName() const
    {
        return m_csName;
    }

    // Returns the argument override.
    const CString GetArgument() const
    {
        return m_csArgument;
    }

    //Returns the log file hint override.
    const CString GetLogFile() const
    {
        return m_csLogfile;
    }

private:
    //Get subitem name
    static CString GetName(CString csItemName, ILogger& logger)
    {
        return csItemName;
        //TODO: Need to walk up to the parent to look for SubItem and verify the item is defined.
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::CustomErrorMappingBase
// This class define the way the error should be handled, e.g. it means
// Success, Failure, Retry
//------------------------------------------------------------------------------
class CustomErrorMappingBase
{
private:
    //Beware: When adding new variable, ensure that the copy constructor is updated as well.
    CString m_csMappingName;

public:
    //Constructor
    CustomErrorMappingBase(CString csMappingName)
        : m_csMappingName(csMappingName)
    {
    }

    //Returns the name of mapping - Success, Failure or Retry.
    const CString GetMappingName() const
    {
        return m_csMappingName;
    }

    // Destructor
    virtual ~CustomErrorMappingBase()
    {}
};

//------------------------------------------------------------------------------
// Class: IronMan::CustomErrorMappingBase
// This class is a specialization of CusomErrorMappingBase to
// a.  Execute an item (Optional)
// b.  Reinstall the page.
//------------------------------------------------------------------------------
class CustomErrorRetry : public CustomErrorMappingBase
{
private:
    //Beware: When adding new variable, ensure that the copy constructor is updated as well.
    CustomErrorItem* m_errorItem;
    UINT m_uRetryLimit;
    UINT m_uDelay;

public:
    // Constructor
    CustomErrorRetry(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : CustomErrorMappingBase(L"Retry")
        , m_errorItem(GetErrorItem(ElementUtils::FindOptionalChildElementByName(spElement, L"ItemRef", logger), logger))
        , m_uRetryLimit(ElementUtils::GetAttributeIntByName(spElement, L"Limit", logger))
        , m_uDelay(ElementUtils::GetAttributeIntByName(spElement, L"Delay", logger)) 
    {
        ElementUtils::VerifyName(spElement, L"Retry", logger);
    }

    //Copy Constructor
    CustomErrorRetry(const CustomErrorRetry& rhs)
        : CustomErrorMappingBase(L"Retry")
        , m_errorItem(NULL)
        , m_uRetryLimit(rhs.m_uRetryLimit)
        , m_uDelay(rhs.m_uDelay) 
    {
        // Helper items are optional for Retry Elements.  The scenario is a author want to re-run the setup item after a delay
        if ( rhs.HelperItemExists() )
        {
            m_errorItem = new CustomErrorItem(*(rhs.m_errorItem));
        }
    }

    // Destructor
    virtual ~CustomErrorRetry()
    {
        if (NULL != m_errorItem)
        {
            delete m_errorItem;
            m_errorItem = NULL;
        }
    }

    //Determine if the subitem authoring exists.
    const bool HelperItemExists() const
    {
        return NULL != m_errorItem;
    }

    //Get the subitem object.
    const CustomErrorItem* GetHelperItem() const
    {
        return m_errorItem;
    }

    //Get the number of retry setted.
    const UINT GetRetryLimit() const
    {
        return m_uRetryLimit;
    }

    //Get the seconds to delay before retry.
    const UINT GetDelay() const
    {
        return m_uDelay;
    }

private:	
    static CustomErrorItem* GetErrorItem(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        if (NULL != spElement)
        {
            return new CustomErrorItem(spElement, logger);
        }
        return NULL;
    }
};

//------------------------------------------------------------------------------
// Class: IronMan::CustomErrorMappingBase
//------------------------------------------------------------------------------
class CustomError
{	
    //Beware: When adding new variable, ensure that the copy constructor is updated as well.
    CustomErrorMappingBase* m_pMapping;
    CString m_csErrorCode;

public:
    // Constructor
    CustomError(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_csErrorCode(ElementUtils::GetAttributeByName(spElement, L"ReturnCode", logger))
        , m_pMapping(CreateMapping(spElement, logger))
    {
        ElementUtils::VerifyName(spElement, L"CustomError", logger);
    }

    virtual CustomError* Clone() const { return new CustomError(*this); }

    //Copy Constructor
    CustomError(const CustomError& rhs)
        : m_csErrorCode(rhs.m_csErrorCode)
        , m_pMapping(CreateMapping(rhs.m_pMapping))
    {
    }

    //Get the return code
    const CString GetReturnCode() const
    {
        return m_csErrorCode;
    }

    //Get the map that contains all the mapping.
    const CustomErrorMappingBase* GetMapping() const
    {
        static int useless = 0;
        return m_pMapping;
    }

    // Destructor
    ~CustomError()
    {
        if (NULL != m_pMapping)
        {
            delete m_pMapping;
            m_pMapping = NULL;
        }
    }

private:
    static CustomErrorMappingBase* CreateMapping(CustomErrorMappingBase* rhs)
    {
        if (0 == rhs->GetMappingName().Compare(L"Retry"))
        {
            return new CustomErrorRetry(*(static_cast<const IronMan::CustomErrorRetry*>(rhs)));
        }        
        else
        {
            return new CustomErrorMappingBase(rhs->GetMappingName());	
        }
    }

    static CustomErrorMappingBase* CreateMapping(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {
        if (ElementUtils::CountChildElements(spElement) != 1)
        {
            CInvalidXmlException ixe(L"schema validation failure: More than 1 CustomError Mapping block defined.");
        }

        CString csMappingElementName = ElementUtils::GetName(ElementUtils::GetFirstChild(spElement));
        LOG(logger, ILogger::Debug, L"The mapping element defined: " + csMappingElementName);

        if (!csMappingElementName.Compare(L"Retry"))
        {  
            LOG(logger, ILogger::Debug, L"Create CustomErrorRetry object");
            return new CustomErrorRetry(ElementUtils::FindChildElementByName(spElement, L"Retry", logger), logger);
        }
        else 
        {
            LOG(logger, ILogger::Debug, L"Create CustomErrorMappingBase object");
            return new CustomErrorMappingBase(csMappingElementName);		
        }
    }
};

class CustomErrorHandling
{	
public:
    struct ItemMap : public CSimpleMap<CString, CustomError*>
    {
        ItemMap() {}
        ItemMap(const ItemMap& rhs)
        {
            for(int i=0; i<rhs.GetSize(); ++i)
                Add(rhs.GetKeyAt(i), rhs.GetValueAt(i)->Clone());
        }
        ItemMap(const CSimpleMap<CString, CustomError*>& rhs)
        {
            for(int i=0; i<rhs.GetSize(); ++i)
                Add(rhs.GetKeyAt(i), rhs.GetValueAt(i)->Clone());
        }

        virtual ~ItemMap()
        {
            for (int i=0; i<GetSize(); ++i)
            {
                delete (*this).GetValueAt(i);
            }
        }
    } m_customError;

    //Copy Constructor
    CustomErrorHandling(const CustomErrorHandling& rhs)   
        : m_customError(rhs.m_customError) 
    {}

    //Needed by Engine data.
    CustomErrorHandling()
    {}

    // Constructor
    CustomErrorHandling(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)		
    {
        if (NULL == spElement)
        {
            LOG(logger, ILogger::Debug, L"CustomErrorHandling element not defined");
            return;
        }

        LOG(logger, ILogger::Debug, L"Processing CustomErrorHandling element block");
        ElementUtils::VerifyName(spElement, L"CustomErrorHandling", logger);
        int iChildCount = ElementUtils::CountChildElements(spElement);
        if (iChildCount <= 0)
        {
            CInvalidXmlException ixe(L"schema validation failure: Expect at least one CustomError element.");
        }

        for (int iIndex = 0; iIndex < iChildCount; ++iIndex)
        {
            CString csReturnCode = ElementUtils::GetAttributeByName(ElementUtils::FindChildElementByNumber(spElement, iIndex, logger), L"ReturnCode", logger);
            LOG(logger, ILogger::Debug, L"Adding Custom Code: " + csReturnCode);
            m_customError.Add(	csReturnCode
                                , new CustomError(ElementUtils::FindChildElementByNumber(spElement, iIndex, logger),logger));				
        }
    }
    // Destructor
    virtual ~CustomErrorHandling()
    {
    }
};
} // namespace IronMan
