//-------------------------------------------------------------------------------------------------
// <copyright file="Blockers.h" company="Microsoft">
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
//    Process the authored blockers
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DataToOperand.h"
#include "common\BlockInfo.h"
#include "..\Common\XmlUtils.h"
#include "expressions.h"
#include "schema\BlockerElement.h"
#include "ux\ux.h"

namespace IronMan
{

class Blockers
{
    ILogger& m_logger;

public:
    Blocks m_blocks;

public:
    Blockers(ILogger& logger) 
        : m_logger(logger)
    {
    }

    void Evaluate(const BlockersElement& blockerElement, const IProvideDataToOperand& dataToOperand, Ux& uxLogger)
    {
        if (blockerElement.IsDefined())
        {
            bool bSuccessBlocks = GetBlocksPrivate(blockerElement.m_SuccessBlocker, dataToOperand, m_blocks.SuccessBlocks, uxLogger, UxEnum::bSuccess);

            if (bSuccessBlocks == false)
            {
                bool bStopBlocks = GetBlocksPrivate(blockerElement.m_StopBlocker, dataToOperand, m_blocks.StopBlocks, uxLogger, UxEnum::bStop);
                bool bWarnBlocks = GetBlocksPrivate(blockerElement.m_WarnBlocker, dataToOperand, m_blocks.WarnBlocks, uxLogger, UxEnum::bWarn);            
            }
            //return bStopBlocks || bWarnBlocks;
        }
    }

private:
    Blockers();
    Blockers& operator=(const Blockers& rhs);

private: 
    bool GetBlocksPrivate(const SpecificBlockerElement& specificBlockerElement
                            , const IProvideDataToOperand& dataToOperand
                            , CSimpleArray<BlockInfo>& blocks
                            , Ux& uxLogger
                            , UxEnum::blockerEnum blockerType) const
    { 
        //We want to clear the block even if spBlockersChildElement is NULL.
        blocks.RemoveAll();

        //This condition is true when Blockers block includes only either the WarnBlockers or the StopBlockers. 
        if (0 == specificBlockerElement.TotalNumberOfBlocker())
        {
            return false;
        }

        for(int iIndex = 0; iIndex < specificBlockerElement.m_blockers.GetSize(); ++iIndex)
        {
            if (BlockIfBase::BlockIf == specificBlockerElement.m_blockers[iIndex]->GetType())
            {
                const BlockIfElement* pBlockIf = dynamic_cast<BlockIfElement*> (specificBlockerElement.m_blockers[iIndex]);

                {  
                    CString section(L" evaluated to false");
                    PUSHLOGSECTIONPOP(m_logger, L"BlockIf", pBlockIf->GetDisplayText(), section);

                    LOG(m_logger, ILogger::Information, pBlockIf->GetDisplayText());
                    if (pBlockIf->Evaluate(dataToOperand))
                    {
                        // Record data point BlockIf/@ID.  If ID is empty GetID returns BlockIf/@DisplayName
                        uxLogger.RecordBlocker(blockerType, pBlockIf->GetID());
                        blocks.Add(BlockInfo(pBlockIf->GetDisplayText(), m_logger));
                        section = L" evaluated to true";
                    }
                }
            }
            else if (BlockIfBase::BlockIfGroup == specificBlockerElement.m_blockers[iIndex]->GetType())
            {
                CSimpleArray<CString> arrayOfSubText;
                LOG(m_logger, ILogger::Information, specificBlockerElement.m_blockers[iIndex]->GetDisplayText());
                BlockIfGroupElement* pBlockIfGroup = dynamic_cast<BlockIfGroupElement*> (specificBlockerElement.m_blockers[iIndex]);
                for(int iBlockIfGroupIndex = 0; iBlockIfGroupIndex < pBlockIfGroup->m_blockers.GetSize(); ++iBlockIfGroupIndex)
                {                    
                    const BlockIfElement* pSubBlockIf = dynamic_cast<BlockIfElement*> (pBlockIfGroup->m_blockers[iBlockIfGroupIndex]);

                    {
                        CString section(L" evaluated to false");
                        PUSHLOGSECTIONPOP(m_logger, L"BlockIf", pSubBlockIf->GetDisplayText(), section);
                        if (pSubBlockIf->Evaluate(dataToOperand))
                        {
                            if (pSubBlockIf->GetDisplayText().GetLength())
                            {
                                // Record data point BlockIf/@ID.  If ID is empty GetID returns BlockIf/@DisplayName
                                uxLogger.RecordBlocker(blockerType, pSubBlockIf->GetID());
                                arrayOfSubText.Add(pSubBlockIf->GetDisplayText());
                                section = L" evaluated to true";
                            }
                        }
                    }
                }

                if (arrayOfSubText.GetSize() > 0)
                {
                    blocks.Add(BlockInfo(specificBlockerElement.m_blockers[iIndex]->GetDisplayText(), arrayOfSubText));
                }
            }
        }
        return blocks.GetSize() > 0;
    }

}; // class Blockers

}
