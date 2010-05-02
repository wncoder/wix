//-------------------------------------------------------------------------------------------------
// <copyright file="BlockChecker.h" company="Microsoft">
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
//    Determine if a block has been hit.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "Interfaces\IBlockedDialog.h"
#include "Interfaces\ILogger.h"

namespace IronMan
{
    class BlockChecker : public IBlockChecker
    {
        class Results : public IBlockChecker::IResult
        {
            HRESULT m_hrStopBlockerReturnCode;
            bool m_bSuccessBlockerWasHit;
            bool m_bStopBlockerWasHit;
            bool m_bWarnBlockerWasHitAndUserCanceled;
            bool m_bWarnBlockerWasHit;
        public:
            Results()
                : m_hrStopBlockerReturnCode(S_OK)
                , m_bSuccessBlockerWasHit(false)
                , m_bStopBlockerWasHit(false)
                , m_bWarnBlockerWasHitAndUserCanceled(false)
                , m_bWarnBlockerWasHit(false)
            {
            }
            void SetSuccessBlockerWasHit()
            {
                m_bSuccessBlockerWasHit = true;
            }
            void SetStopBlockerWasHit(HRESULT hrReturnCode)
            {
                m_bStopBlockerWasHit = true;
                m_hrStopBlockerReturnCode = hrReturnCode;
            }
            void SetWarnBlockerWasHitAndUserCanceled()
            {
                m_bWarnBlockerWasHitAndUserCanceled = true;
            }

            void SetWarnBlockerWasHit()
            {
                m_bWarnBlockerWasHit = true;
            }
            
            // IBlockChecker
            virtual bool SuccessBlockerWasHit()
            {
                return m_bSuccessBlockerWasHit;
            }
            virtual bool StopBlockerWasHit(HRESULT& hrReturnCode)
            {
                hrReturnCode = m_hrStopBlockerReturnCode;
                return m_bStopBlockerWasHit;
            }
            virtual bool WarnBlockerWasHitAndUserCanceled()
            {
                return m_bWarnBlockerWasHitAndUserCanceled;
            }

            virtual bool WarnBlockerWasHit()
            {
                return m_bWarnBlockerWasHit;
            }
        
        } m_results;
     
        const BlockersElement& m_blockersElement;
        const IProvideDataToOperand& m_dataToOperand;
        const CString m_csPackageName;
        ILogger& m_logger;
        Ux& m_uxLogger;
        
    public:
        BlockChecker(const BlockersElement& blockersElement,
                      const IProvideDataToOperand& dataToOperand,
                      const CString csPackageName,
                      ILogger& logger,
                      Ux& uxLogger) 
            : m_blockersElement(blockersElement)
            , m_dataToOperand(dataToOperand)
            , m_csPackageName(csPackageName)
            , m_logger(logger)
            , m_uxLogger(uxLogger)
        {
        }

        IBlockChecker::IResult& ProcessBlocks(HWND hWndParent=NULL)
        {
            CString section(L" no blocking conditions found"); // the default
            PUSHLOGSECTIONPOP(m_logger, L"Global Block Checks", L"Checking for global blockers", section);
            HRESULT hr = S_OK;

            Blockers blockers(m_logger);
            blockers.Evaluate(m_blockersElement, m_dataToOperand, m_uxLogger);
            
            if (blockers.m_blocks.HasBlocks())
            {
                LogBlocks(blockers.m_blocks, m_logger);

                if (blockers.m_blocks.HasSuccessBlocks())
                {
                    m_results.SetSuccessBlockerWasHit();
                    section = L": SuccessBlockers evaluated to true.";
                    hr = S_OK;
                }
                else if (blockers.m_blocks.HasStopBlocks())
                {
                    m_results.SetStopBlockerWasHit(m_blockersElement.m_StopBlocker.GetStopBlockersReturnCode());
                    section = L": StopBlockers evaluated to true.";
                }
                else 
                {

                    m_results.SetWarnBlockerWasHit();
                    section = L": WarnBlockers evaluated to true.";

                }
            }

            return m_results;
        }

        IBlockChecker::IResult& GetResult()
        {
            return m_results;
        }

    private:
        static void LogBlocks(const Blocks& blocks, ILogger& logger)
        {
            class CLogBlocks
            {
                ILogger& m_logger;
            public:
                CLogBlocks(ILogger& logger) : m_logger(logger) {}

                void Log(const CSimpleArray<BlockInfo>& blocks) const
                {
                    m_logger.LogStartList();
                    for (int i=0; i<blocks.GetSize(); ++i)
                    {
                        m_logger.LogListItem(blocks[i].strText);
                        UINT cSubTexts = blocks[i].arrayOfSubText.GetSize();
                        if (cSubTexts > 0)
                        {
                            m_logger.LogStartList();
                            for (UINT j=0; j<cSubTexts; ++j)
                            {
                                m_logger.LogListItem(blocks[i].arrayOfSubText[j]);
                            }
                            m_logger.LogEndList();
                        }
                    }
                    m_logger.LogEndList();
                }
            };

            ILogger::LoggingLevel lvl = blocks.HasStopBlocks() ? ILogger::Result : ILogger::Warning;

            logger.BeginLogAsIs(lvl, L"Logging all the global blocks");

            CLogBlocks logBlocks(logger);

            if (blocks.HasSuccessBlocks())
            {
                logger.LogLine(L"Success Blockers:"); 
                logBlocks.Log(blocks.SuccessBlocks);
            }
            if (blocks.HasStopBlocks())
            {
                logger.LogLine(L"Installation Blockers:"); 
                logBlocks.Log(blocks.StopBlocks);
            }
            if (blocks.HasWarnBlocks())
            {
                logger.LogLine(L"Pre-Installation Warnings:"); 
                logBlocks.Log(blocks.WarnBlocks);
            }

            logger.EndLogAsIs();
        }
    };
}
