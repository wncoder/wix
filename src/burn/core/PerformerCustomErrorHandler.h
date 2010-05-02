//-------------------------------------------------------------------------------------------------
// <copyright file="PerformerCustomErrorHandler.h" company="Microsoft">
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
#include "firsterror.h"
#include "common\MSIUtils.h"
#include "Interfaces\IPerformer.h"
#include "Interfaces\ILogger.h"
#include "common\MsgWaitForObject.h"
#include "common\ResultObserver.h"
#include "ExeInstaller.h"
#include "SmartLock.h"

namespace IronMan
{
    class PerformerCustomErrorHandler
    {

    private:
        const CustomErrorHandling* m_pcehError;
        CSimpleMap<CString, UINT> m_ErrorCount;
        Ux& m_uxLogger;
        ILogger& m_logger;  

    public:
        //Constructor
        PerformerCustomErrorHandler( Ux& uxLogger, ILogger& logger)
            : m_pcehError(NULL)
            , m_uxLogger(uxLogger)
            , m_logger(logger)
        {
            m_ErrorCount.RemoveAll();
        }                

        //Destructor
        virtual ~PerformerCustomErrorHandler()
        {            
        }

        //Intializer
        void Initialize(const CustomErrorHandling* pceh)
        {
            m_pcehError = pceh;
        }		

        //Process the CustomErrorHandling block.  It will do 
        bool Execute(FirstError& error, ResultObserver resultObserver)
        {
            if (NULL == m_pcehError)
            {
                LOG(m_logger, ILogger::Information, L"No CustomError defined for this item.");
                return false;
            }

            //Change HRESULT to string for simple processing.
            CString csError;
            int errorCode = HRESULT_CODE(error.GetError());
            if ((errorCode< 1600 || errorCode > 1699) && (errorCode != 0))
            {
                csError.Format(L"0x%x", error.GetError());                
            }
            else
            {
                csError.Format(L"%d", errorCode);
            }

            CString csMapping = GetMapping(csError);
            LOG(m_logger, ILogger::Information, L"Error " + csError + L" is mapped to Custom Error: " + csMapping);
            int iMappingIndex;

            // Mapping == Retry
            if (0 == csMapping.Compare(L"Retry"))
            {
                if (-1 != (iMappingIndex = m_pcehError->m_customError.FindKey(csError)))
                {            
                    const IronMan::CustomErrorRetry *cer = static_cast<const IronMan::CustomErrorRetry *>((m_pcehError->m_customError.GetValueAt(iMappingIndex))->GetMapping());
                    int iRetryCountIndex = m_ErrorCount.FindKey(csError);
                    if (-1 == iRetryCountIndex) 
                    {
                        //If we cannot find the error in the map, it must be the first time we are hitting it.
                        LOG(m_logger, ILogger::Debug, L"New custom error, add to the map");
                        m_ErrorCount.Add(csError, 1);
                    }
                    else
                    {
                        LOG(m_logger, ILogger::Debug, L"Existing custom error found in the map.");

                        UINT uRetryCount = static_cast<UINT>(m_ErrorCount.GetValueAt(iRetryCountIndex));
                        CString cs;
                        cs.Format(L"Retry %u of %u of custom error handling", uRetryCount, cer->GetRetryLimit());
                        LOG(m_logger, ILogger::Information, cs);

                        //If we have gone over the number of time we need to retry, let get out immediately.
                        if (cer->GetRetryLimit() <= uRetryCount)   
                        {
                            LOG(m_logger, ILogger::Information, L"Retry count over existing limit, not going to retry again.");
                            return false;                    
                        }
                        //If not, add the counter and update the map
                        m_ErrorCount.SetAtIndex(iRetryCountIndex, csError, ++uRetryCount);
                    }

                    //If there any fix it exe to execute?
                    if (cer->HelperItemExists())
                    {
                        CString csItemName = cer->GetHelperItem()->GetItemName();
                        CString csExpandedItemName;
                        CSystemUtil::ExpandEnvironmentVariables(csItemName, csExpandedItemName);
                        CString csArgument = cer->GetHelperItem()->GetArgument();
                        CString csLogFile = cer->GetHelperItem()->GetLogFile();
                        const HelperItems *pHelperItems = dynamic_cast<const HelperItems*>(m_pcehError);
                        if (!pHelperItems)
                        {
                            LOG(m_logger, ILogger::Information, L"HelperItems can't be read.");
                            return false;
                        }

                        if (!ExecuteHelper(
                                pHelperItems, 
                                csExpandedItemName, 
                                csArgument, 
                                csLogFile, 
                                m_logger, 
                                m_uxLogger
                                ))
                        {
                            return false;
                        }
                    }

                    //Sleep for as long as authored for this error.
                    if (0 < cer->GetDelay())
                    {
                        CString cs;
                        cs.Format(L"Delaying for %u seconds before retrying.", cer->GetDelay());
                        LOG(m_logger, ILogger::Information, L"Delaying for Starting to delay");
                        ::Sleep(cer->GetDelay()* 1000);
                    }

                    return true;
                }
            }
            // Mapping == Retry
            else if (0 == csMapping.Compare(L"Success"))
            {
                LOG(m_logger, ILogger::Information, L"Overwrite the current error to S_OK.");
                error.OverwriteCurrentError(S_OK);                
            }
            // Mapping == Retry
            else if (0 == csMapping.Compare(L"Failure")) 
            {
                LOG(m_logger, ILogger::Information, L"Overwrite the current error to E_FAIL.");
                error.OverwriteCurrentError(E_FAIL);
            }
            return false;
        }

        static bool ExecuteHelper(
            __in const HelperItems *pHelperItems, 
            __in const CString& csExpandedItemName,
            __in const CString& csArgument,
            __in const CString& csLogFile,
            __in ILogger& logger,
            __in Ux& uxLogger
            )
        {
            const ItemBase *pHelperItem = NULL;
            if (!pHelperItems->GetHelper(csExpandedItemName, &pHelperItem))
            {
                LOG(logger, ILogger::Information, L"HelperItems not found : " + csExpandedItemName);
                return false;
            }

            SmartLock lock(pHelperItem);

            if (!lock.VerifyAndLock(false /*varaball - change to true b4 checkin*/, false, ActionTable::Install, logger))
            {
                LOG(logger, ILogger::Warning, L"HelperItem verification failed. Cannot run the retry helper : " + csExpandedItemName);
                return false;
            }

            const ExeBase *pExe = dynamic_cast<const ExeBase *>(pHelperItem);
            if (!pExe)
            {
                LOG(logger, ILogger::Information, L"HelperItem is not Exe item.");
                return false;
            }

            if (!csArgument.IsEmpty())
                pExe->OverrideInstallCommandLine(csArgument);

            if (!csLogFile.IsEmpty())
                pExe->OverrideLogFileHint(csLogFile);

            ExeInstaller exeInstaller(*pExe, logger, uxLogger);
            HRESULT hr = E_FAIL;
            ResultObserver observer(NullProgressObserver::GetNullProgressObserver(), hr);

            LOG(logger, ILogger::Debug, L"Executing Helper item with the following parameters:");   
            LOG(logger, ILogger::Debug, L"Helper Item name: " + csExpandedItemName);
            LOG(logger, ILogger::Debug, L"Argument provided: " + csArgument);
            LOG(logger, ILogger::Debug, L"Log File name: " + csLogFile);
            static_cast<IPerformer&>(exeInstaller).PerformAction(observer);

            if (!MSIUtils::IsSuccess(hr))
            {
                LOG(logger, ILogger::Information, L"Helper item execution failed.");
                return false;
            }
            LOG(logger, ILogger::Information, L"Helper item execution succeed.");
            return true;
        }

    private:
        CString GetMapping(CString csError)
        {
          if (NULL != m_pcehError)
          {          
              int iKeyIndex = 0;
              
              if (-1 != (iKeyIndex = m_pcehError->m_customError.FindKey(csError)))
              {
                  LOG(m_logger, ILogger::Debug, L"Error Mapping found");
                  return (m_pcehError->m_customError.GetValueAt(iKeyIndex))->GetMapping()->GetMappingName();
              }
              LOG(m_logger, ILogger::Debug, L"Error Mapping NOT FOUND.");
          }
          return L"";          
        }
    };
} // namespace IronMan
