//-------------------------------------------------------------------------------------------------
// <copyright file="BlockerElement.h" company="Microsoft">
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
#include "..\Expressions.h"
#include "common\CustomErrors.h"

namespace IronMan
{
// This file contains a set of classes that form the object model for the blocker element

    //The Base that both BlockIf and BlockIfGroup extended from.
    class BlockIfBase
    {        
    public:
        enum BlockIfType
        {
            Undefined = 0,
            BlockIf   = 1,
            BlockIfGroup = 2			
        };
    private:
        CString m_csDisplayText;
        BlockIfType m_type;

    public:
        BlockIfBase(BlockIfType type, CString csDisplayText)
            : m_type(type)
            , m_csDisplayText(csDisplayText)
        {
        }

        virtual BlockIfBase* Clone() const  = 0;        

        virtual ~BlockIfBase()
        {}

        //Copy Constructor
        BlockIfBase(const BlockIfBase& rhs)
            : m_csDisplayText(rhs.m_csDisplayText)            
            , m_type(rhs.m_type)
        {
        }

        const CString& GetDisplayText() const
        {
            return m_csDisplayText;
        }

        const BlockIfType GetType() const
        {
            return m_type;
        }
    };

    //The object representation of the BlockIf element
    class BlockIfElement : public BlockIf, public BlockIfBase
    {
        const CString m_strId;
    public:
        //Constructor
        BlockIfElement(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : BlockIf(spElement, logger)
            , BlockIfBase(BlockIfBase::BlockIf, ElementUtils::GetOptionalAttributeByName(spElement, L"DisplayText", logger))
            , m_strId(GetID(spElement, logger))
        {
            ElementUtils::VerifyName(spElement, L"BlockIf", logger);
        } 

        virtual BlockIfElement* Clone() const 
        { 
            return new BlockIfElement(*this); 
        }

        //Copy Constructor
        BlockIfElement(const BlockIfElement& rhs)
            : BlockIf(rhs)
            , BlockIfBase(rhs)
            , m_strId(rhs.m_strId)
        {
        }

        //---------------------------------------------------------------------
        // GetID
        // If authored, returns the ID of the BlockIf element to be used
        // for reporting user experience data.  If not authored returns an 
        // empty string. Is not localized and must be less than 64 characters
        //---------------------------------------------------------------------
        const CString GetID() const
        {
            if ( m_strId.IsEmpty() )
            {
                return GetDisplayText();
            }
            else
            {
                return m_strId;
            }
        }

    private:
        //---------------------------------------------------------------------
        // GetID
        // Gets ID from the authoring to be stored
        // Validates string:
        // Cannot be localized(this has already been validated)
        // Must not be greater than 64 characters
        //---------------------------------------------------------------------
        static const CString GetID(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        {
            CString strId(ElementUtils::GetOptionalAttributeByName(spElement, L"ID", logger));
            // Cannot be greater than 64 characters
            if ( strId.GetLength() > 64 )
            {
                CInvalidXmlException ixe(L"schema validation failure:  BlockIf/@ID cannot be more than 64 characters.");
                throw ixe;
            }
            return strId;
        }

    };

    //The container for BlockerIf
    class BlockIfGroupElement : public BlockIfBase
    {        
    public:
        struct BlockerArray : public CSimpleArray<BlockIfElement*>
        {
            BlockerArray() {}
            BlockerArray(const BlockerArray& rhs)
            {
                for(int i=0; i<rhs.GetSize(); ++i)
                    Add(rhs[i]->Clone());
            }
            BlockerArray(const CSimpleArray<BlockIfElement*>& rhs)
            {
                for(int i=0; i<rhs.GetSize(); ++i)
                    Add(rhs[i]->Clone());
            }

            virtual ~BlockerArray()
            {
                for (int i=0; i<GetSize(); ++i)
                {
                    delete (*this)[i];
                }
            }
        } m_blockers;

    public:
        //Constructor
        BlockIfGroupElement(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : BlockIfBase(BlockIfBase::BlockIfGroup, ElementUtils::GetOptionalAttributeByName(spElement, L"DisplayText", logger))
        {
            ElementUtils::VerifyName(spElement, L"BlockIfGroup", logger);
            if (0 >= ElementUtils::CountChildElements(spElement, L"BlockIf"))
            {
                CInvalidXmlException ixe(L"schema validation failure: BlockIfGroup must have at least one child element.");
                throw ixe;
            }
            GetBlocker(spElement, logger);
        }

        virtual BlockIfGroupElement* Clone() const 
        { 
            return new BlockIfGroupElement(*this); 
        }

        //Copy Constructor
        BlockIfGroupElement(const BlockIfGroupElement& rhs)
            :BlockIfBase(rhs)
            , m_blockers(rhs.m_blockers)
        {
        }

    private:
        void GetBlocker(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        {
            if (NULL == spElement)
            {         
                return;
            }

            int iChildCount = static_cast<int>(ElementUtils::CountChildElements(spElement));
            for (int iIndex = 0; iIndex < iChildCount; ++iIndex)
            {                
                m_blockers.Add(new BlockIfElement(ElementUtils::FindChildElementByNumber(spElement, iIndex, logger), logger));
            }
        }
    };

    //For both Warning and Stop blocker
    class SpecificBlockerElement
    {
        CString m_csBlockerType;
        long m_stopBlockersReturnCode;

    public:        
        struct BlockerArray : public CSimpleArray<BlockIfBase*>
        {
            BlockerArray() {}
            BlockerArray(const BlockerArray& rhs)
            {
                for(int i=0; i<rhs.GetSize(); ++i)
                {
                    Add(rhs[i]->Clone());                    
                }                    
            }
            BlockerArray(const CSimpleArray<BlockIfBase*>& rhs)
            {
                for(int i=0; i<rhs.GetSize(); ++i)
                {
                    Add(rhs[i]->Clone());
                }
            }

            virtual ~BlockerArray()
            {
                for (int i=0; i<GetSize(); ++i)
                {
                    if (BlockIfBase::BlockIfGroup == (*this)[i]->GetType())
                    {
                        BlockIfGroupElement * bige = dynamic_cast<BlockIfGroupElement *>((*this)[i]);
                        delete bige;
                    }
                    else
                    {
                        BlockIfElement * bie = dynamic_cast<BlockIfElement *>((*this)[i]);
                        delete bie;
                    }
                }
            }
        } m_blockers;

    public:
        SpecificBlockerElement(const CComPtr<IXMLDOMElement>& spElement, CString csBlockerType, ILogger& logger)
            : m_csBlockerType(csBlockerType)            
        {
            if (NULL != spElement)
            {
                ElementUtils::VerifyName(spElement, csBlockerType, logger);
                GetBlockers(spElement, logger);
                // If ReturnCode is not specified
                // Defauft ReturnCode is CustomErrors::StopBlockerHitOrSystemRequirementNotMet(5100)
                m_stopBlockersReturnCode = ElementUtils::GetOptionalAttributeLongByName(spElement
                                                                                        , L"ReturnCode"
                                                                                        , CustomErrors::StopBlockerHitOrSystemRequirementNotMet
                                                                                        , logger);
            }
        }

        virtual SpecificBlockerElement* Clone() const 
        { 
            return new SpecificBlockerElement(*this); 
        }

        //Copy Constructor
        SpecificBlockerElement(const SpecificBlockerElement& rhs)
            : m_csBlockerType(rhs.m_csBlockerType)             
            , m_blockers(rhs.m_blockers)
            , m_stopBlockersReturnCode(rhs.m_stopBlockersReturnCode)
        {            
        }

        const UINT TotalNumberOfBlocker() const
        {
            return m_blockers.GetSize();
        }

        const CString GetBlockerTypeName() const
        {
            return m_csBlockerType;
        }

        //------------------------------------------------------------------------------
        // GetStopBlockersReturnCode
        //
        // Returns the StopBlockers\@ReturnCode value.
        // Defauft if not authored: CustomErrors::StopBlockerHitOrSystemRequirementNotMet(5100)
        //-------------------------------------------------------------------------------
        const long GetStopBlockersReturnCode() const
        {
            return m_stopBlockersReturnCode;
        }

    private:        
        void GetBlockers(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        {
            int iChildCount = static_cast<int>(ElementUtils::CountChildElements(spElement));
            for (int iIndex = 0; iIndex < iChildCount; ++iIndex)
            {
                m_blockers.Add(AddBlocker(ElementUtils::FindChildElementByNumber(spElement, iIndex, logger), logger));
            }
        }

        static BlockIfBase* AddBlocker(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        {
            CString csElementTypeName = ElementUtils::GetName(spElement);
            
            if (0 == csElementTypeName.Compare(L"BlockIfGroup"))
            {  
                return new BlockIfGroupElement(spElement, logger);
            }
            else 
            {            
                return new BlockIfElement(spElement, logger);		
            }
        }
    };

    //The <Blockers> element
    class BlockersElement
    {
    public:
        SpecificBlockerElement m_SuccessBlocker;
        SpecificBlockerElement m_StopBlocker;
        SpecificBlockerElement m_WarnBlocker;

    public:
        BlockersElement(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : m_SuccessBlocker(ElementUtils::FindOptionalChildElementByName(spElement, L"SuccessBlockers", logger), L"SuccessBlockers", logger)
            , m_StopBlocker(ElementUtils::FindOptionalChildElementByName(spElement, L"StopBlockers", logger), L"StopBlockers", logger)
            , m_WarnBlocker(ElementUtils::FindOptionalChildElementByName(spElement, L"WarnBlockers", logger), L"WarnBlockers", logger)
        {
            if (NULL != spElement)
            {
                ElementUtils::VerifyName(spElement, L"Blockers", logger);
                int iNumberOfSuccessBlocker = ElementUtils::CountChildElements(spElement, L"SuccessBlockers");
                int iNumberOfStopBlocker = ElementUtils::CountChildElements(spElement, L"StopBlockers");
                int iNumberOfWarnBlocker = ElementUtils::CountChildElements(spElement, L"WarnBlockers");
         
                if ((0 == iNumberOfSuccessBlocker) && (0 == iNumberOfStopBlocker) && (0 == iNumberOfWarnBlocker))
                {
                    CInvalidXmlException ixe(L"schema validation failure: no valid child element found for 'Blockers' node.");
                    throw ixe;
                }

                if (0 < iNumberOfSuccessBlocker)
                {
                    if (1 < iNumberOfSuccessBlocker)
                    {
                        CInvalidXmlException ixe(L"schema validation failure: More than 1 Success Block defined.");                        
                        throw ixe;
                    }

                    if (0 == m_SuccessBlocker.TotalNumberOfBlocker())
                    {
                        CInvalidXmlException ixe(L"schema validation failure: Success blockers has no child node");                    
                        throw ixe;
                    }
                }

                if (0 < iNumberOfStopBlocker)
                {
                    if (1 < iNumberOfStopBlocker)
                    {
                        CInvalidXmlException ixe(L"schema validation failure: More than 1 Stop Block defined.");                        
                        throw ixe;
                    }

                    if (0 == m_StopBlocker.TotalNumberOfBlocker())
                    {
                        CInvalidXmlException ixe(L"schema validation failure: Stop blockers has no child node");                    
                        throw ixe;
                    }
                }

                if (0 < iNumberOfWarnBlocker) 
                {
                    if (1 < iNumberOfWarnBlocker)
                    {
                        CInvalidXmlException ixe(L"schema validation failure: More than 1 Warning Block defined.");                    
                        throw ixe;
                    }

                    if (0 == m_WarnBlocker.TotalNumberOfBlocker())
                    {
                        CInvalidXmlException ixe(L"schema validation failure: Warn blockers has no child node");
                        throw ixe;
                    }
                }
            }        
        }

        const bool IsDefined() const
        {
            return 0 < (m_SuccessBlocker.TotalNumberOfBlocker() 
                            + m_StopBlocker.TotalNumberOfBlocker() 
                            + m_WarnBlocker.TotalNumberOfBlocker());
        }
    };
} // namespace IronMan
