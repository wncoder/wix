// <copyright file="UX.h" company="Microsoft">
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
//  This is class for writting Ux Data
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\ILogger.h"
#include "LogSignatureDecorator.h"
#include "common\Operation.h"
#include "common\logUtils.h"
#include "common\SystemUtil.h"
#include "VersionUtil.h"
#include "CmdLineParser.h"
#include "InTestEnvironment.h"
#include "watson\WatsonDefinition.h"
#include "ModuleUtils.h"
#include "schema\ConfigurationElement.h"
#include "uxEnum.h"
#include "common\StringUtil.h"
#include "common\RegUtil.h"
#include "common\MsiUtils.h"
#include "interfaces\IMetrics.h"
#include "UxEnum.h"
#include "CheckTrust.h"

namespace IronMan
{
    class StreamTypeChecker
    {
        IMetrics& m_uxEngine;

    public:
        // This struct represents the definition of a stream.  It allows the definition of 9 columns 
        // because user experience data allows a max of 9 columns.
        //
        // Each colunmn is represented by 1 bit:
        //    True == Dword
        //    False == CString
        //
        struct TableDescription
        {
            unsigned int count;
            unsigned short column1 : 1;
            unsigned short column2 : 1;
            unsigned short column3 : 1;
            unsigned short column4 : 1;
            unsigned short column5 : 1;
            unsigned short column6 : 1;
            unsigned short column7 : 1;
            unsigned short column8 : 1;
            unsigned short column9 : 1;
        };

        class TableDescriptionArray
        {
        private:
            unsigned int *m_rgDatapointId;
            TableDescription *m_rgTableDescription;
            size_t m_stAllocatedSize;  // Total number of elements available
            size_t m_stHighwaterMark;  // Last element actually used
        public:
            // Default Constructor
            TableDescriptionArray()
                :m_rgDatapointId(NULL)
                ,m_rgTableDescription(NULL)
                ,m_stAllocatedSize(0)
                ,m_stHighwaterMark(0)
            {
            }

            // Destructor
            ~TableDescriptionArray()
            {
                // Free up the memory for the data points
                if (m_rgDatapointId != NULL)
                {
                    delete [] m_rgDatapointId;
                    m_rgDatapointId = NULL;
                }
                // Free up the memory for the table description
                if (m_rgTableDescription != NULL)
                {
                    delete [] m_rgTableDescription;
                    m_rgTableDescription = NULL;
                }

                m_stAllocatedSize = 0;
            }

            // Allocate the space for the data stream tables. This is fixed. Returns size allocated if successful
            size_t SetSize(size_t stNumElements)
            {
                if (m_stAllocatedSize == 0)
                {
                    m_rgDatapointId = new unsigned int[stNumElements];
                    m_rgTableDescription = new TableDescription[stNumElements];
                    if ( (m_rgDatapointId != NULL)  &&  (m_rgTableDescription != NULL))
                    {
                        m_stAllocatedSize = stNumElements;
                        return m_stAllocatedSize;
                    }
                }

                return 0;
            }

            // Return maximum possible number of entries, not all of which will necessarily be used
            size_t GetSize() const
            {
                return m_stAllocatedSize;
            }

            // Return number of entries actually used - these are contiguous
            size_t GetCount() const
            {
                return m_stHighwaterMark;
            }

            // Find index of specified data point, or return -1 if it can't be found
            int FindDataPointIndex(unsigned int iDatapointId) const
            {
                int iIndexOfDataPoint = -1;
                for (unsigned int i = 0; i < GetCount(); ++i)
                {
                    if (m_rgDatapointId[i] == iDatapointId)
                    {
                        iIndexOfDataPoint = i;
                        break;
                    }
                }

                return iIndexOfDataPoint;
            }

            // Add specified data point, or return -1 if it can't be added because of space
            int Add(unsigned int iDatapointId, const TableDescription & newTableDescription)
            {
                int iIndexOfDataPoint = FindDataPointIndex(iDatapointId);
                // Index will be negative if not found
                if (iIndexOfDataPoint < 0)
                {
                    if ( (m_stHighwaterMark < INT_MAX) &&           // On 64bit builds, size_t is larger than int, so limit the ability to add past "int" size
                         (m_stHighwaterMark < m_stAllocatedSize) )
                    {
                        iIndexOfDataPoint = (int)m_stHighwaterMark++;
                        m_rgDatapointId[iIndexOfDataPoint] = iDatapointId;
                        m_rgTableDescription[iIndexOfDataPoint] = newTableDescription;
                    }
                    // If we run out of space the iIndexOfDataPoint will still be negative
                }
                else
                {
                    m_rgTableDescription[iIndexOfDataPoint] = newTableDescription;
                }
                return iIndexOfDataPoint;
            }

            // Return table description at specified index, or a "null table descriptions" full of -1 if index is out of bounds
            TableDescription & GetValueAt(int nIndex) const
            {
                static TableDescription nullTableDescription = {-1};
                if ( nIndex < (int)GetCount()  &&  nIndex >= 0)
                {

                    return m_rgTableDescription[nIndex];
                }
                else
                {
                    return nullTableDescription;
                }
            }

        };

        TableDescriptionArray m_TableDescriptionArray;



    public:
        StreamTypeChecker(IMetrics& uxEngine, const size_t stNumStreamElements = 64)
            : m_uxEngine(uxEngine)
        {

            // Allocate the stream data table - this always has to be called in the same module (.dll) as the Metrics destructor
            // runs because otherwise the statically linked CRTL will try to free memory from the wrong heap
            m_TableDescriptionArray.SetSize(stNumStreamElements);
        }

    //Methods to handle Stream.
    public:
        //Handles 1 column stream.
        template<typename T1> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1)
        {	
            int iIndex = GetOrAddTable(iDatapointId, 1, CheckType<T1>::Type);
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 1) return E_INVALIDARG;

            return WriteRow(iDatapointId, 1, iIndex, t1);
        }

        //Handles 2 columns stream.
        template<typename T1, typename T2> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 2
                , CheckType<T1>::Type
                , CheckType<T2>::Type);
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 2) return E_INVALIDARG;

            return WriteRow(iDatapointId, 2, iIndex, t1, t2);
        }

        //Handles 3 columns stream.
        template<typename T1, typename T2, typename T3> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2, const T3& t3)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 3
                , CheckType<T1>::Type
                , CheckType<T2>::Type
                , CheckType<T3>::Type);	
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 3) return E_INVALIDARG;

            return WriteRow(iDatapointId, 3, iIndex, t1, t2, t3);
        }

        //Handles 4 columns stream.
        template<typename T1, typename T2, typename T3, typename T4> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 4
                , CheckType<T1>::Type
                , CheckType<T2>::Type
                , CheckType<T3>::Type
                , CheckType<T4>::Type);	
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 4) return E_INVALIDARG;

            return WriteRow(iDatapointId, 4, iIndex, t1, t2, t3, t4);			
        }

        //Handles 5 columns stream.
        template<typename T1, typename T2, typename T3, typename T4, typename T5> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 5
                , CheckType<T1>::Type
                , CheckType<T2>::Type
                , CheckType<T3>::Type
                , CheckType<T4>::Type
                , CheckType<T5>::Type);	
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 5) return E_INVALIDARG;

            return WriteRow(iDatapointId, 5, iIndex, t1, t2, t3, t4, t5);		
        }

        //Handles 6 columns stream.
        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 6
                , CheckType<T1>::Type
                , CheckType<T2>::Type
                , CheckType<T3>::Type
                , CheckType<T4>::Type
                , CheckType<T5>::Type
                , CheckType<T6>::Type);	
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 6) return E_INVALIDARG;

            return WriteRow(iDatapointId, 6, iIndex, t1, t2, t3, t4, t5, t6);		
        }

        //Handles 7 columns stream.
        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 7
                , CheckType<T1>::Type
                , CheckType<T2>::Type
                , CheckType<T3>::Type
                , CheckType<T4>::Type
                , CheckType<T5>::Type
                , CheckType<T6>::Type
                , CheckType<T7>::Type);	
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 7) return E_INVALIDARG;

            return WriteRow(iDatapointId, 7, iIndex, t1, t2, t3, t4, t5, t6, t7);		
        }

        //Handles 8 columns stream.
        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 8
                , CheckType<T1>::Type
                , CheckType<T2>::Type
                , CheckType<T3>::Type
                , CheckType<T4>::Type
                , CheckType<T5>::Type
                , CheckType<T6>::Type
                , CheckType<T7>::Type
                , CheckType<T8>::Type);	
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 8) return E_INVALIDARG;

            return WriteRow(iDatapointId, 8, iIndex, t1, t2, t3, t4, t5, t6, t7, t8);		
        }

        //Handles 9 columns stream.
        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9> 
        HRESULT WriteTuple(unsigned int iDatapointId, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9)
        {
            int iIndex = GetOrAddTable(iDatapointId
                , 9
                , CheckType<T1>::Type
                , CheckType<T2>::Type
                , CheckType<T3>::Type
                , CheckType<T4>::Type
                , CheckType<T5>::Type
                , CheckType<T6>::Type
                , CheckType<T7>::Type
                , CheckType<T8>::Type
                , CheckType<T9>::Type);	
            if (m_TableDescriptionArray.GetValueAt(iIndex).count != 9) return E_INVALIDARG;

            return WriteRow(iDatapointId, 9, iIndex, t1, t2, t3, t4, t5, t6, t7, t8, t9);
        }		

        private:
        template<typename T1> 
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column1 != CheckType<T1>::Type) return E_INVALIDARG;

            return m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t1); //write last value			
        }

        template<typename T1, typename T2> 
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column2 != CheckType<T2>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn,t2); //write last value
            }
            return hr;
        }

        template<typename T1, typename T2, typename T3> 
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2, const T3& t3)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column3 != CheckType<T3>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1, t2);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t3); //write last value
            }
            return hr;
        }

        template<typename T1, typename T2, typename T3, typename T4> 
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column4 != CheckType<T4>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1, t2, t3);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t4); //write last value
            }
            return hr;
        }

        template<typename T1, typename T2, typename T3, typename T4, typename T5>
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column5 != CheckType<T5>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1, t2, t3, t4);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t5); //write last value
            }
            return hr;
        }

        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column6 != CheckType<T6>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1, t2, t3, t4, t5);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t6); //write last value
            }
            return hr;
        }

        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column7 != CheckType<T7>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1, t2, t3, t4, t5, t6);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t7); //write last value
            }
            return hr;
        }

        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column8 != CheckType<T8>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1, t2, t3, t4, t5, t6, t7);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t8); //write last value
            }
            return hr;
        }

        template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
        HRESULT WriteRow(unsigned int iDatapointId, int iNumberofColumn, int iIndex, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9)
        {
            // Ensure that the type matches that defined in the definition
            if (m_TableDescriptionArray.GetValueAt(iIndex).column9 != CheckType<T9>::Type) return E_INVALIDARG;

            HRESULT hr = WriteRow(iDatapointId, iNumberofColumn, iIndex, t1, t2, t3, t4, t5, t6, t7, t8);
            if (SUCCEEDED(hr))
            {
                hr = m_uxEngine.WriteStream(iDatapointId, iNumberofColumn, t9); //write last value
            }
            return hr;
        }		

        //This employed a technique called Template metaprogramming
        template<typename T> struct CheckType
        {
        };
        template<> struct CheckType<DWORD>
        {
            enum { Type = true };
        };
        template<> struct CheckType<CString>
        {
            enum { Type = false };
        };

        

        //Middle tier functions specifically for adding stream table definition
    private:

        int GetOrAddTable(int iDatapointId
            , int iColumnCount
            , bool col1 = false
            , bool col2 = false
            , bool col3 = false
            , bool col4 = false
            , bool col5 = false
            , bool col6 = false
            , bool col7 = false
            , bool col8 = false
            , bool col9 = false)
        {
            int iIndex = 0;
            if (-1 == (iIndex = m_TableDescriptionArray.FindDataPointIndex(iDatapointId)))
            {
                TableDescription td;
                td.count = iColumnCount;
                td.column1 = col1;
                td.column2 = col2;
                td.column3 = col3;
                td.column4 = col4;
                td.column5 = col5;
                td.column6 = col6;
                td.column7 = col7;
                td.column8 = col8;
                td.column9 = col9;			
                iIndex = m_TableDescriptionArray.Add(iDatapointId, td);
            }

            return iIndex;
        }


    };

    class Ux
    {
    private:
        struct ItemTable
        {
            CString csItemName;
            UxEnum::phaseEnum phase;
            UxEnum::actionEnum action;
            DWORD dwStartTime;
            UxEnum::technologyEnum technology;
        };

    private:
        ILogger& m_logger;

    private:
        bool m_bInRollback;
        DWORD m_dwStartUpTick;  // Application Start up tick
        CTime m_startTime;      // Record starting time to use in elapsed time report for final result
        CString m_csPackageName;

        UxEnum::uiModeEnum m_uiMode;

        //Download Related Variable
        CSimpleMap<CString, ItemTable*> m_ItemMap;
        
        // This variable is needed to ensure we write unique 
        // products to the stream.
        // Used CSimpleArray instead of CAtlArray because 
        // the expected size is expected to be small.
        CSimpleArray<CString> m_ApplicableProductsArray;

        //Pre-Req Related Variables
        DWORD m_dwStartApplicableIfTick;

        IMetrics& m_uxEngine;
        StreamTypeChecker m_steamWriter;
        CPath m_pthLoaderFileName;

    //Protected so that it can be unit tested:
    protected:
        DWORD m_dwStartInstallTick;

    private:
        //It is a static because we need to persist this information when we crash.
        static bool& Accept()
        {
            static bool bAccept = false;
            return bAccept;
        }

        //It is a static because we need to persist this information when we crash. 
        //Note: Accept() and AlwaysUpload() can potentially be 1 function, but I am 
        //leaving as it is for now.
        static bool& AlwaysUpload()
        {
            static bool bAlwaysUpload = false;
            return bAlwaysUpload;
        }

    public:

        //------------------------------------------------------------------------------
        // Ux
        //
        // The constructor for UX.  
        //
        //-------------------------------------------------------------------------------
        Ux(ILogger& logger, IMetrics& metricsEngine)
            : m_logger(logger)
            , m_uxEngine(metricsEngine)
            , m_steamWriter(metricsEngine)
            , m_dwStartUpTick(GetTickCount())
            , m_dwStartInstallTick(GetTickCount())
            , m_dwStartApplicableIfTick(0)	
            , m_bInRollback(false) //We are not in rollback
            , m_csPackageName(L"")
            , m_uiMode(UxEnum::smInteractive)
        {
            // Record starting time to use in elapsed time report for final result
            m_startTime = CTime::GetCurrentTime();
        }

        virtual ~Ux()
        {
            // The send report will already have been called in EndAndSendUxReport in the main program
        }

        void GetDefaultUxLogger()
        {
            HRESULT hr;
            hr = m_uxEngine.ContinueSession();
            WriteErrorLog(hr, L"Failed to Continue Session");
        }

        void StartAndInitializeUxLogger(Operation::euiOperation eOperation
            , bool bSilent
            , bool bPassive
            , UserExperienceDataCollection::Policy policy
            , const CPath& pthLoaderExe)
        {
            if (UserExperienceDataCollection::Disabled !=  policy)
            {
                //Verify if the file exists
                if (!pthLoaderExe.FileExists())
                {
                    CInvalidXmlException ixe(L"schema validation failure: loader exe does not exists.");
                    LOG(m_logger, ILogger::Error, ixe.GetMessage());
                    throw ixe;
                }

                //Verify that the file is signed.
                CString strHash = L"";
                CString strLoaderExe = L"Loader Exe";
                FileAuthenticity fileAuthenticity(strLoaderExe, pthLoaderExe, strHash, 0, m_logger);
                HRESULT hr = fileAuthenticity.Verify();
                if (FAILED(hr))
                {
                    CInvalidXmlException ixe(L"Loader Exe does not meeting the signing requirements.");
                    LOG(m_logger, ILogger::Error, ixe.GetMessage());
                    throw ixe;
                }

                m_pthLoaderFileName = pthLoaderExe;
            }
            HRESULT hr;
            hr = m_uxEngine.StartSession();
            WriteErrorLog(hr, L"Failed to record StartSession");

            RecordReportMode(UxEnum::srmNoError);
            RecordCurrentState(UxEnum::uiInitialization);

            RecordOperationRequested(eOperation);
            RecordOperationUiMode(bSilent, bPassive);
            RecordChainerPackage();

            RecordHardwareDatapoints();
            RecordSystemConfigurationDatapoints();
            RecordUserRelatedDatapoints();

            RecordRebootCount(0);

            AlwaysUpload() = false;
            if (UserExperienceDataCollection::AlwaysUploaded == policy)
            {
                AlwaysUpload() = true;
            }
        }

        void RecordEngineData(const CString& csPackageName
                                , const CString& csPackageVersion
                                , const DWORD dwLcid
                                , const UxEnum::patchTrainEnum train)
        {
            m_csPackageName = csPackageName;
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpPackageName, csPackageName);
            WriteErrorLog(hr, L"Failed to record PackageName");

            hr = m_uxEngine.Write(UxEnum::sdpPackageVersion, csPackageVersion);
            WriteErrorLog(hr, L"Failed to record PackageVersion");

            hr = m_uxEngine.Write(UxEnum::sdpDisplayedLcidId, dwLcid);
            WriteErrorLog(hr, L"Failed to record DisplayedLcidId");

            hr = m_uxEngine.Write(UxEnum::sdpInstallerVersion, CVersionUtil::GetExeFileVersion());
            WriteErrorLog(hr, L"Failed to record InstallerVersion");

            hr = m_uxEngine.Write(UxEnum::sdpPatchType, static_cast<DWORD>(train));
            WriteErrorLog(hr, L"Failed to record PatchType");

            RecordPackagePatchType(train);

#ifdef DEBUG
            hr = m_uxEngine.Write(UxEnum::sdpIsRetailBuild, false);
#else
            hr = m_uxEngine.Write(UxEnum::sdpIsRetailBuild, true);
#endif
            WriteErrorLog(hr, L"Failed to record IsRetailBuild");
        }

        void RecordPackagePatchType(const UxEnum::patchTrainEnum train)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpPatchType, static_cast<DWORD>(train));
            WriteErrorLog(hr, L"Failed to record PatchType");
        }

        void RecordRebootCount(DWORD dwCount)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpRebootCount, dwCount);
            WriteErrorLog(hr, L"Failed to record MPC");
        }

        void RecordRebootPending(UxEnum::rebootPendingEnum pending)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpRebootPending, static_cast<DWORD>(pending));
            WriteErrorLog(hr, L"Failed to record RebootPending");
        }

        void RecordMPC(const CString& csMPC)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpMPC, csMPC);
            WriteErrorLog(hr, L"Failed to record MPC");
        }

        void RecordPIDEdition(DWORD dwPIDEdition)
        {    
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpPIDEdition, dwPIDEdition);
            WriteErrorLog(hr, L"Failed to record PIDEdition");
        }

        void RecordPIDRange(const CString& csPIDRange)
        {    
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpPIDRange, csPIDRange);
            WriteErrorLog(hr, L"Failed to record PIDRange");
        }

        void RecordPIDInstalledSkus(const CString& csPIDInstalledSkus)
        {    
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpPIDInstalledSkus, csPIDInstalledSkus);
            WriteErrorLog(hr, L"Failed to record PIDInstalledSkus");
        }

        CPath EndAndSendUxReport()
        {
            if (GetUxApproval())
            {
                //Need to record stop time only when we are sending the data.
                StopRecordingInstallTime();

                CAtlTemporaryFile tempFile;
                tempFile.Create();
                CPath pthUxLogFileName(tempFile.TempFileName());
                tempFile.Close();

                //Verify that the loader file is signed before attempt to use it
                CString strHash = L"";
                HRESULT hr = m_uxEngine.EndAndSend(pthUxLogFileName, m_pthLoaderFileName);
                WriteErrorLog(hr, L"Failed to Close and Send Ux Report");

                return pthUxLogFileName;
            }
            return L"";
        }

/*
        void RecordUxDataFromUI(const bool bIsSilentOrPassive, IProvideDataToEngine& uiData)
        {
            RecordRefreshCount(uiData.RefreshCount());
        }
*/
        void RecordCrashErrorDatapoints(HRESULT hrReturnCode, bool bIsCrash)
        {
            //P1 Package Name
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpPackageName, CString(WatsonData::WatsonDataStatic()->GetGeneralAppName()));
            WriteErrorLog(hr, L"Failed to record PackageName");

            //P2 - Package Version
            hr = m_uxEngine.Write(UxEnum::sdpPackageVersion, CString(WatsonData::WatsonDataStatic()->GetPackageVersion()));
            WriteErrorLog(hr, L"Failed to record Package Version");

            //P3 - Application Version
            hr = m_uxEngine.Write(UxEnum::sdpInstallerVersion, CVersionUtil::GetExeFileVersion());
            WriteErrorLog(hr, L"Failed to record Application Version");

            //P4 - Operation Requested
            RecordOperationRequested(WatsonData::WatsonDataStatic()->GetOperation());

            //P5
            CString csCurrentFlag = GetCurrentFlag(bIsCrash, WatsonData::WatsonDataStatic()->GetCurrentAction(), WatsonData::WatsonDataStatic()->GetCurrentPhase());
            hr = m_uxEngine.Write(UxEnum::sdpCurrentFlag, csCurrentFlag);
            WriteErrorLog(hr, L"Failed to record CurrentFlag");
            
            //P6

            //P7
            RecordReturnCode(hrReturnCode);

            //P8 - Result Detail
            RecordMsiErrorMessage(WatsonData::WatsonDataStatic()->GetInternalErrorString());

            //P9 -- Current Step
            RecordCurrentItemStep(WatsonData::WatsonDataStatic()->GetCurrentStep());

            RecordReportMode(true == bIsCrash ? UxEnum::srmCrash : UxEnum::srmError);
        }

        //Set the user selected state.
        //Make it virtual for testing purpose.
        virtual void RecordUxApprovalState(bool fUpload)
        {
            Accept() = fUpload;
        }

        //Determine if we should send report.
        //Make it virtual for testing purpose.
        virtual bool GetUxApproval()
        {
            return (true == Accept() || IsTestEnvironment() || AlwaysUpload());
        }

        // Get start time when Ux logger was created
        virtual const CTime & GetStartingTime()
        {
            return m_startTime;
        }

        void SetInRollback()
        {
            m_bInRollback = true;
        }

        void RecordBlocker(UxEnum::blockerEnum blockerType, CString csText)
        {
            HRESULT hr = m_steamWriter.WriteTuple(UxEnum::sdpBlocker
                                    , static_cast<DWORD>(blockerType)
                                    , csText);
            WriteErrorLog(hr, L"Failed to record blocker");
        }

        //*******************************************************************************
        //                          General Operation Data Points
        //*******************************************************************************
        void RecordReturnCode(HRESULT hrReturnCode)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpReturnCode, StringUtil::FromHresult(hrReturnCode));
            WriteErrorLog(hr, L"Failed to record return code");
        }
    
        void RecordMsiErrorMessage(const CString& cs)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpResultDetail, cs);	
            WriteErrorLog(hr, L"Failed to record msi error message");
        }

        void RecordCurrentItemStep(const CString& csCurrentItemStep)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpCurrentItemStep, csCurrentItemStep);
            WriteErrorLog(hr, L"Failed to record Current Item Step");
        }

        void RecordReportMode(UxEnum::spigotReportModeEnum reportMode)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpReportMode, static_cast<DWORD>(reportMode));
            WriteErrorLog(hr, L"Failed to record msi error message");
        }

        void RecordSKU(UxEnum::SkuEnum sku)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpSKU, static_cast<DWORD>(sku));
            WriteErrorLog(hr, L"Failed to record SKU");
        }

        void RecordPhaseAndCurrentItemNameOnError(const UxEnum::phaseEnum phase
                                                    , const UxEnum::actionEnum action
                                                    , const CString& csItemName)
        {
            // These should only be written if there is an error, otherwise the download/install can overwrite each other 
            // in a non-deterministic way
            // The Metrics and the Watson phase and item need to be the same to match up the metrics data with a watson report
            // If there are both Download and Install errors, report the Install error
            if ( UxEnum::pUI == WatsonData::WatsonDataStatic()->GetCurrentPhase()
                || ( UxEnum::pDownload == WatsonData::WatsonDataStatic()->GetCurrentPhase() && UxEnum::pInstall == phase ))
            {
                WatsonData::WatsonDataStatic()->SetCurrentPhase(phase);
                HRESULT hr = m_uxEngine.Write(UxEnum::sdpFaultPhase, WatsonData::WatsonDataStatic()->GetCurrentPhaseString());
                WriteErrorLog(hr, L"Failed to record Current Phase (sdpFaultPhase) ");

                //Ensure that we only record the first item.
                CString csSingleItemName = csItemName;
                int iSeparatorPosition = csItemName.Find(L";");
                if (-1 != iSeparatorPosition)
                    csSingleItemName = csItemName.Left(iSeparatorPosition);

                hr = m_uxEngine.Write(UxEnum::sdpFaultItem, csSingleItemName);
                WriteErrorLog(hr, L"Failed to record current Item Name");
                WatsonData::WatsonDataStatic()->SetCurrentItemName(csSingleItemName);

                WatsonData::WatsonDataStatic()->SetCurrentAction(action);
            }
        }

        //Record the internal error with the followings:
        // a. CurrentItem = A string that described what is failing.
        // b. ResultDetail = The real HRESULT
        // c. Result = 5101.
        void RecordInternalFailure(CString csAction, HRESULT hrError)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpFaultItem, csAction);
            WriteErrorLog(hr, L"Failed to record current Item Name");

            hr = m_uxEngine.Write(UxEnum::sdpResultDetail, hrError);
            WriteErrorLog(hr, L"Failed to record result detail");

            hr = m_uxEngine.Write(UxEnum::sdpReturnCode, StringUtil::FromHresult(5101));
            WriteErrorLog(hr, L"Failed to record return code");
        }
        
        //*******************************************************************************
        //								Install Data Points
        //*******************************************************************************

        //------------------------------------------------------------------------------
        // AddApplicableProduct
        //
        // This method is called to add new product to the stream.  
        //
        // Note
        // ====
        // This method is expected to ensure that only unique products are add to the 
        // stream.
        //-------------------------------------------------------------------------------
        void AddApplicableProduct(const CString& csGuid, const CString& csProductName)
        {				
            if (-1 == m_ApplicableProductsArray.Find(csGuid))
            {
                m_ApplicableProductsArray.Add(csGuid);
                m_steamWriter.WriteTuple(UxEnum::sdpApplicableSKU, csGuid, csProductName);
            }
        }

        //This is total taken to download + install the package.
        void StopRecordingInstallTime()
        {
            if (0 < m_dwStartInstallTick)
            {
                HRESULT hr = m_uxEngine.Write(UxEnum::sdpInstallTime, GetTickCount() - m_dwStartInstallTick);
                WriteErrorLog(hr, L"Failed to record install time");
            }
        }
        
        void RecordInstallTime(DWORD dwTicks)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpInstallTime, dwTicks);
            WriteErrorLog(hr, L"Failed to record install time");
        }

        // Cache it to be later written to the item stream.
        // This method abtract the CmdLineSwitches 
        void StartRecordingItem(const CString& csItemName //canonical Name
                                , UxEnum::phaseEnum phase
                                , const UxEnum::actionEnum action
                                , UxEnum::technologyEnum technology)
        {
            StartRecordingItemT<CCmdLineSwitches>(csItemName, phase, action, technology);
        }

        // Cache it to be later written to the item stream.
        // This method does not abstract the CmdLineSwitches, it is to be used by unit tests.
        template <typename CmdLineSwitches>
        void StartRecordingItemT(const CString& csItemName //canonical Name
                                , UxEnum::phaseEnum phase
                                , const UxEnum::actionEnum action
                                , UxEnum::technologyEnum technology)
        {            
            CString csReallyItemNameOnly = ComputeItemName<CmdLineSwitches>(CPath(csItemName));
            if (-1 == m_ItemMap.FindKey(csReallyItemNameOnly))
            {
                ItemTable* it = new ItemTable();
                it->csItemName = csReallyItemNameOnly;
                it->phase = phase;
                it->action = action;
                it->dwStartTime = GetTickCount();
                it->technology = technology;
                m_ItemMap.Add(csReallyItemNameOnly, it);
            }
        }

        // This overloaded method require action enum to be specified. The original action that was written in 
        // StartRecordingItem() will be overwritten with this one.
        void StopRecordingItem(const CString& csItemName //canonical Name
                                , const UxEnum::actionEnum action
                                , const ULONGLONG ulSize
                                , const HRESULT hrExitCode
                                , const CString& csResultDetail
                                , const DWORD dwRetryCount)
        {
            StopRecordingItemT<CCmdLineSwitches>(csItemName, action, ulSize, hrExitCode, csResultDetail, dwRetryCount);
        }

        // This overloaded method does not require action enum to be specified. The original action that was writte in 
        // StartRecordingItem() will not be changed.
        void StopRecordingItem(const CString& csItemName //canonical Name
            , const ULONGLONG ulSize
            , const HRESULT hrExitCode
            , const CString& csResultDetail
            , const DWORD dwRetryCount)
        {
            StopRecordingItemT<CCmdLineSwitches>(csItemName, UxEnum::aNone, ulSize, hrExitCode, csResultDetail, dwRetryCount);
        }

        template <typename CmdLineSwitches>
        void StopRecordingItemT(const CString csItemName //canonical Name
                                , const UxEnum::actionEnum action
                                , ULONGLONG ulSize
                                , HRESULT hrExitCode
                                , const CString csResultDetail
                                , const DWORD dwRetryCount)
        {
            int iIndex = 0;
            CString csReallyItemNameOnly = ComputeItemName<CmdLineSwitches>(CPath(csItemName));
            if (-1 != (iIndex = m_ItemMap.FindKey(csReallyItemNameOnly)))
            {
                UxEnum::actionEnum actionToWrite = action;
                // If no action is provided, wrte the cached action when Start was called.
                if (actionToWrite == UxEnum::aNone)
                {
                    actionToWrite = m_ItemMap.GetValueAt(iIndex)->action;
                }
                m_steamWriter.WriteTuple(UxEnum::sdpItemStream
                                      , csReallyItemNameOnly
                                      , static_cast<DWORD>(m_ItemMap.GetValueAt(iIndex)->phase)
                                      , static_cast<DWORD>(actionToWrite)
                                      , StringUtil::FromHresult(hrExitCode)
                                      , static_cast<DWORD>(ulSize)
                                      , GetTickCount() - m_ItemMap.GetValueAt(iIndex)->dwStartTime
                                      , static_cast<DWORD>(m_ItemMap.GetValueAt(iIndex)->technology)
                                      , csResultDetail
                                      , dwRetryCount);
                delete m_ItemMap.GetValueAt(iIndex);
                m_ItemMap.RemoveAt(iIndex);
            }
        }

    public:
        //*******************************************************************************
        //								Pre-Req Data Points
        //*******************************************************************************

        void StartApplicableIfTime()
        {
            m_dwStartApplicableIfTick = GetTickCount();
        }

        void StopApplicableIfTime()
        {			
            DWORD dwTotalTicks = GetTickCount() -  m_dwStartApplicableIfTick;
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpApplicableIfTime, dwTotalTicks);
            WriteErrorLog(hr, L"Failed to record InstallIfTime");
        }
    
    public:
        //Make it virtual for test purpose
        virtual void RecordSystemRequirementCheckTime(DWORD dwTicks)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpSystemRequirementCheckTime, dwTicks);
            WriteErrorLog(hr, L"Failed to record SystemRequirementCheckTime");
        }

    public:
        //*******************************************************************************
        //                         Patch Stream Data Points
        //*******************************************************************************
        void RecordPatchStream(const CString& strPatchGuids
                                , const CString& strProductGuid
                                , UxEnum::patchTrainEnum train)
        {
            RecordPatchStream(strPatchGuids
                                , strProductGuid
                                , train
                                , MSIUtils::GetMspDisplayNameFromLocalPackage
                                , MSIUtils::GetProductNameFromProductGuid);
        }

        template <typename F, typename G>
        void RecordPatchStream(const CString& strPatchGuids
                                , const CString& strProductGuid
                                , UxEnum::patchTrainEnum train
                                , G GetMspDisplayNameFromLocalPackage
                                , F GetProductNameFromProductGuid)
        {
            HRESULT hr;
            if (-1 == strPatchGuids.Find(L";"))
            {
                hr = m_steamWriter.WriteTuple(  UxEnum::sdpPatchStream
                                        , GetMspDisplayNameFromLocalPackage(strPatchGuids, m_logger)
                                        , GetProductNameFromProductGuid(strProductGuid)
                                        , static_cast<DWORD>(train));
                WriteErrorLog(hr, L"Failed to record PatchStream");
            }
            else
            {
                int iStart = 0;
                for(;;)
                {
                    CString strPatchGuid = strPatchGuids.Tokenize(L";", iStart);
                    if (-1 == iStart)
                    {
                        break;
                    }
                    hr =m_steamWriter.WriteTuple(  UxEnum::sdpPatchStream
                                            , GetMspDisplayNameFromLocalPackage(strPatchGuid, m_logger)
                                            , GetProductNameFromProductGuid(strProductGuid)
                                            , static_cast<DWORD>(train));
                    WriteErrorLog(hr, L"Failed to record PatchStream");
                }
            }
        }

        //*******************************************************************************
        //								UI Data Points
        //******************************************************************************
        void RecordTimeToFirstScreen(DWORD dwCurrentTick)
        {
            DWORD dwTick = 0;
            if (0 != m_dwStartUpTick)
            {
                dwTick = dwCurrentTick - m_dwStartUpTick;
            
                HRESULT hr = m_uxEngine.Write(UxEnum::sdpTimeToFirstWindow, dwTick);
                WriteErrorLog(hr, L"Failed to record TimeToFirstWindow");	
                
                m_dwStartUpTick = 0;  //Reset it so that we don't write any more.
            }
        }

        void RecordProfileFeatureMap(const CString& csProfileFeatureMap)
        {
             HRESULT hr = m_uxEngine.Write(UxEnum::sdpProfileFeatureMap, csProfileFeatureMap);
             WriteErrorLog(hr, L"Failed to record Customize");
        }

        void RecordCustomizeControl(DWORD dwClicked)
        {
             HRESULT hr = m_uxEngine.Write(UxEnum::sdpCustomize, dwClicked);
             WriteErrorLog(hr, L"Failed to record Customize");
        }

        void RecordCancelPage(const CString& csCancelPageName)
        {
             HRESULT hr = m_uxEngine.Write(UxEnum::sdpCancelPage, csCancelPageName);
             WriteErrorLog(hr, L"Failed to record Customize");
        }

    private:
        void RecordRefreshCount(const DWORD dwRefreshCount)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpNumberOfRefresh, dwRefreshCount);
            WriteErrorLog(hr, L"Failed to record RefreshCount");
        }

    public:
        void RecordCurrentState(UxEnum::UiStateType state)
        {			
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpCurrentState, static_cast<DWORD>(state));
            WriteErrorLog(hr, L"Failed to record current state name");
        }

    
        //*******************************************************************************
        //								User Provided Data Points
        //*******************************************************************************
        void RecordOperationRequested(Operation::euiOperation eOperation)
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpOperationRequested, static_cast<DWORD>(eOperation));
            WriteErrorLog(hr, L"Failed to record Operation Requested");
        }

    private:
        void RecordOperationUiMode(bool bSilent, bool bPassive)
        {
            if (bSilent) 
            {
                m_uiMode = IronMan::UxEnum::smSilent;
            }
            else if (bPassive)
            {
                m_uiMode = IronMan::UxEnum::smPassive;
            }
            else
            {
                m_uiMode = IronMan::UxEnum::smInteractive;
            }

            HRESULT hr = m_uxEngine.Write(UxEnum::sdpOperationUi, static_cast<DWORD>(m_uiMode));
            WriteErrorLog(hr, L"Failed to record Operation UI Mode");
        }

        void RecordChainerPackage()
        {
            CCmdLineSwitches switches;
            if (IronMan::CCmdLineParser(::GetCommandLine()).ContainsOption(L"ChainingPackage"))
            {
                CString csChainerPackage = IronMan::CCmdLineParser(::GetCommandLine()).GetOptionValue(L"ChainingPackage");
                HRESULT hr = m_uxEngine.Write(UxEnum::sdpChainingPackage, csChainerPackage);
                WriteErrorLog(hr, L"Failed to record Operation UI Mode");
            }
        }

        //*******************************************************************************
        //								Hardware Data Points
        //*******************************************************************************
        void RecordHardwareDatapoints()
        {
            SYSTEM_INFO si = {0};
            CSystemUtil::GetSystemInformation(si);

            HRESULT hr = m_uxEngine.Write(UxEnum::sdpCpuArchitecture, static_cast<DWORD>(si.dwProcessorType));
            WriteErrorLog(hr, L"Failed to record CpuArchitecture");	

            hr = m_uxEngine.Write(UxEnum::sdpNumberOfProcessor, static_cast<DWORD>(si.dwNumberOfProcessors));
            WriteErrorLog(hr, L"Failed to record NumberOfProcessor");	

            //Get Cpu Information
#define REG_KEY_CPU0            L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"
            LPBYTE ProcSpeed;
            DWORD buflen, ret;
            HKEY hKey;
            if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_KEY_CPU0, 0, KEY_READ, &hKey))
            {
                ProcSpeed = 0;
                buflen = sizeof( ProcSpeed );

                ret = RegQueryValueEx(hKey, L"~MHz", NULL, NULL, reinterpret_cast<LPBYTE>(&ProcSpeed), &buflen);
                // If we don't succeed, try some other spellings.
                if (ret != ERROR_SUCCESS)
                {
                    buflen = sizeof( ProcSpeed );
                    ret = RegQueryValueEx(hKey, L"~Mhz", NULL, NULL, reinterpret_cast<LPBYTE>(&ProcSpeed), &buflen);
                }
                if (ret != ERROR_SUCCESS)
                {
                    buflen = sizeof( ProcSpeed );
                    ret = RegQueryValueEx(hKey, L"~mhz", NULL, NULL, reinterpret_cast<LPBYTE>(&ProcSpeed), &buflen);
                }

                RegCloseKey(hKey);

                if (ret == ERROR_SUCCESS)
                {
                    hr = m_uxEngine.Write(UxEnum::sdpCpuSpeed, reinterpret_cast<DWORD>(ProcSpeed));
                    WriteErrorLog(hr, L"Failed to record NumberOfProcessor");
                }
            }

            //Gather system information.
            MEMORYSTATUSEX memoryStatus = {0};
            ::SecureZeroMemory(&memoryStatus, sizeof( memoryStatus ) );

            memoryStatus.dwLength = sizeof (memoryStatus);

            //No need to check return because it does return a value.
            if (0 != GlobalMemoryStatusEx( &memoryStatus ))
            {
                // dwTotalPhys has total size of memory in bytes
                // Dividing it by 1024 * 1024 to get in MBs
                DWORD dwTotalPhyiscalMemory = static_cast<DWORD>(memoryStatus.ullTotalPhys / (1024 * 1024));
                hr = m_uxEngine.Write(UxEnum::sdpSystemMemory, dwTotalPhyiscalMemory);
                WriteErrorLog(hr, L"Failed to record SystemMemory");	
            }
            else
            {   
                WriteErrorLog(::GetLastError(), L"GlobalMemoryStatusEx failed");
            }

            RecordSystemDriveFreeDiskSpace();
        }

        void RecordSystemDriveFreeDiskSpace()
        {
            //Gather System disk space free space
            CString csSystemDirectory;
            int iSystemDirectoryLength = GetSystemDirectory(csSystemDirectory.GetBuffer(MAX_PATH), MAX_PATH);
            if (MAX_PATH < iSystemDirectoryLength)
            {
                csSystemDirectory._ReleaseBuffer();
                GetSystemDirectory(csSystemDirectory.GetBuffer(iSystemDirectoryLength), iSystemDirectoryLength);
            }

            csSystemDirectory._ReleaseBuffer();
            CPath pthSystemDrive(csSystemDirectory);
            pthSystemDrive.StripToRoot();

            ULARGE_INTEGER totalNumberOfBytes = {0};
            ULARGE_INTEGER freeBytesAvailable = {0};
            ULARGE_INTEGER totalNumberOfFreeBytes = {0};
            if ( !::GetDiskFreeSpaceEx( pthSystemDrive
                                , &freeBytesAvailable
                                , &totalNumberOfBytes
                                , &totalNumberOfFreeBytes) )
            {
                freeBytesAvailable.QuadPart = 0;
            }

            HRESULT hr = m_uxEngine.Write(UxEnum::sdpSystemFreeDiskSpace
                , static_cast<DWORD>(freeBytesAvailable.QuadPart/(1024 * 1024)));	
            WriteErrorLog(hr, L"Failed to record SystemMemory");
        }

        //*******************************************************************************
        //                          System Configuation Data Points
        //*******************************************************************************
        void RecordSystemConfigurationDatapoints()
        {
            typedef void (WINAPI *pftGetNativeSystemInfo)(LPSYSTEM_INFO);
            HMODULE hmodule = GetModuleHandle(TEXT("kernel32.dll"));
            if (NULL == hmodule)
            {
                WriteErrorLog(::GetLastError(), L"Failed to record OSFullBuildNumber");	
                return;
            }

            pftGetNativeSystemInfo pGNSI = reinterpret_cast<pftGetNativeSystemInfo>(GetProcAddress(hmodule, "GetNativeSystemInfo"));
            if (NULL == pGNSI)
            {
                pGNSI = &::GetSystemInfo;
            }

            OSHelper os(pGNSI);
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpOSFullBuildNumber, os.GetOSFullBuild());
            WriteErrorLog(hr, L"Failed to record OSFullBuildNumber");

            hr = m_uxEngine.Write(UxEnum::sdpOSAbbr, os.GetOS());
            WriteErrorLog(hr, L"Failed to record OSAbbr");

            hr = m_uxEngine.Write(UxEnum::sdpOsSpLevel, static_cast<DWORD>(os.GetSP()));
            WriteErrorLog(hr, L"Failed to record OsSpLevel");

            //SystemLocale
            //Vista has GetSystemDefaultLocaleName but it is good enough to use same call on all platforms
            hr = m_uxEngine.Write(UxEnum::sdpSystemLocale, GetSystemDefaultLCID());
            WriteErrorLog(hr, L"Failed to record SystemLocale");

            OSVERSIONINFOEX osvi = {0};
            SYSTEM_INFO si = {0};
            os.PopulateOSVersionInfo(osvi);
            CSystemUtil::GetSystemInformation(si);
            hr = m_uxEngine.Write(UxEnum::sdpOSComplete, GetOSComplete(osvi, si));
            WriteErrorLog(hr, L"Failed to record OSComplete");

            //Record Windows Installer verison
            hr = m_uxEngine.Write(UxEnum::sdpWindowInstallerVersion, MSIUtils::GetWindowsInstallerVersion());
            WriteErrorLog(hr, L"Failed to record WindowsInstallerVersion");
        }

        //*******************************************************************************
        //								User Related Data Points
        //*******************************************************************************
        void RecordUserRelatedDatapoints()
        {
            HRESULT hr = m_uxEngine.Write(UxEnum::sdpIsInternal, IsInternalOrPerfLab());
            WriteErrorLog(hr, L"Failed to record IsInternal");

            hr = m_uxEngine.Write(UxEnum::sdpIsAdmin, IsAdmin());
            WriteErrorLog(hr, L"Failed to record IsAdmin");
        }

        //*******************************************************************************
        //								Misc Data Points
        //*******************************************************************************
    protected:
        //This method does belong here because we are trying to extract item name based on a custom logic.
        template <typename F>
        static CString ExtractItemNameFromDestinationPath(const CPath& destinationPath, F GetCurrentDirectory)
        {
            CString strCurrentDirectory;
            GetCurrentDirectory(MAX_PATH, strCurrentDirectory.GetBuffer(MAX_PATH));
            strCurrentDirectory._ReleaseBuffer();

            return ExtractItemNameFromDestinationPath(destinationPath, strCurrentDirectory); 
        }

        static CString ExtractItemNameFromDestinationPath(const CPath& destinationPath, CString csReferenceDirectory)
        {
            CString csItemName = CString(destinationPath);
            csItemName.Replace(csReferenceDirectory, L"");
            csItemName = csItemName.Trim(L'\\');
            return csItemName;	
        }

    //make it protected so that I can unit test it.
    protected: 
        // This function builds up the CString for CurrentFlag datapoint (517) 
        CString GetCurrentFlag(bool bIsCrash
                               , UxEnum::actionEnum action
                               , UxEnum::phaseEnum phase)
        {
            CString csCurrentFlag;

            //action
            csCurrentFlag = UxEnum::GetActionString(action);
            csCurrentFlag += L"_";

            //phase
            csCurrentFlag += UxEnum::GetPhaseString(phase);
            csCurrentFlag += L"_";

            //uimode
            csCurrentFlag += UxEnum::GetUiModeString(m_uiMode);
            csCurrentFlag += L"_";
            
            //crashmode
            csCurrentFlag += bIsCrash ? L"Crash" : L"Error";

            return csCurrentFlag;
        }      

        //This function build up the bitmap for the OSComplete datapoint (493)
        // 1111 1111     1111 1111     1111         1111                1111                    11............1        ........1  (LSB)
        // MajorVersion  MinorVersion  ProductType  ServicePackMajor    ProcessorArchitecture   Reserved   .Compt Mode.....OSPreRelease
        static DWORD GetOSComplete(OSVERSIONINFOEX& osvi, SYSTEM_INFO& si)
        {
            UINT uOs = (osvi.dwMajorVersion & 255) << 24;
            uOs += ((osvi.dwMinorVersion & 255) << 16);
            uOs += ((osvi.wProductType & 15) << 12);
            uOs += ((osvi.wServicePackMajor & 15) << 8);
            uOs += ((si.wProcessorArchitecture & 15) << 4);

            //Determine if we are running in OS Compatibility Mode
            if (CSystemUtil::IsInOSCompatibilityMode())
            {
                uOs += (1 << 1);
            }

            //Get OS Pre-release
            DWORD dwKeyValue;
            if (RegUtil::GetHKLMRegDWORD(L"SYSTEM\\CurrentControlSet\\Control\\Windows", L"CSDReleaseType", dwKeyValue))
            {
                //Non-Zero means it is a pre-release.
                if (0 < dwKeyValue)
                {
                    uOs += 1;
                }
            }
            return uOs;
        }

    private:
        template <typename CmdLineSwitches>
        CString ComputeItemName(const CPath& csPath)
        {
            CString csItemNameOrig(csPath);
            CString csItemName;

            if (csItemNameOrig == (csItemName = ExtractItemNameFromDestinationPath(csPath, CString(ModuleUtils::GetExecutablePathSpecification()))))
            {
                if (csItemNameOrig == (csItemName = ExtractItemNameFromDestinationPath(csPath, ::GetCurrentDirectory)))
                {
                    if (csItemNameOrig == (csItemName = ExtractItemNameFromDestinationPath(csPath, ::GetTempPath)))
                    {
                        CmdLineSwitches switches;
                        CString csLayout = switches.CreateLayout();
                        if (!csLayout.IsEmpty())
                        {
                            csItemName.Replace(csLayout, L"");
                            csItemName = csItemName.Trim(L'\\');
                        }
                    }
                }
            }           

            if ((!m_csPackageName.IsEmpty()) && (0 == csItemName.Find(m_csPackageName)))
            {
                csItemName = csItemName.Mid(m_csPackageName.GetLength()+1);
            }
            csItemName.Replace(L";" + m_csPackageName + L"\\", L";");
                
            return csItemName.Trim(L'\\');
        }
    
        //Non static helper function.
    private:
        void WriteErrorLog(HRESULT hr, const CString& strText)
        {
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(strText + L": 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
        }

        //Static helper function
    private:
        DWORD IsInternalOrPerfLab()
        {
            DWORD dwResult = 0;
            DWORD dwRegValue = 0;
            if (RegUtil::GetHKLMRegDWORD(L"Software\\Microsoft\\DevDiv", L"PerfLab", dwRegValue))
            {
                if (1 == dwRegValue)
                {
                    dwResult = 2;
                }
            }

            if (2 != dwResult)
            {
                dwResult = IsTestEnvironment();
            }

            return dwResult;
        }

        bool IsAdmin()
        {		
            /*++ 
            Routine Description: This routine returns TRUE if the caller's
            process is a member of the Administrators local group. Caller is NOT
            expected to be impersonating anyone and is expected to be able to
            open its own process and process token. 
            Arguments: None. 
            Return Value: 
            TRUE - Caller has Administrators local group. 
            FALSE - Caller does not have Administrators local group. --
            */ 
            BOOL bIsAdmin;  //BIG BOOL because CheckTokenMembership() requires it.
            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
            PSID AdministratorsGroup; 
            bIsAdmin = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup); 

            if(bIsAdmin) 
            {
                if (!CheckTokenMembership( NULL, AdministratorsGroup, &bIsAdmin)) 
                {
                    bIsAdmin = FALSE;
                    LOGGETLASTERROR(m_logger, ILogger::Error, L"CheckTokenMembership");	
                } 
                FreeSid(AdministratorsGroup); 
            }
            else
            {
                LOGGETLASTERROR(m_logger, ILogger::Error, L"AllocateAndInitializeSid");
            }
            return(bIsAdmin);
        }

        // Determines if machine is in Microsoft's(corp.microsoft.com) domain
        virtual bool IsTestEnvironment() // virtual for testing purposes.
        {
            return InTestEnvironment::IsTestEnvironment();
        }

        //Virtual function for testing purpose.
    private:
        virtual DWORD GetTickCount(void) 
        {
            return ::GetTickCount();
        }
        virtual BOOL CheckTokenMembership(
            __in_opt HANDLE TokenHandle,
            __in     PSID SidToCheck,
            __out    PBOOL IsMember
            )
        {
            return ::CheckTokenMembership(TokenHandle, SidToCheck, IsMember); 
        }

        virtual BOOL AllocateAndInitializeSid (
            __in        PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority
            , __in        BYTE nSubAuthorityCount
            , __in        DWORD nSubAuthority0
            , __in        DWORD nSubAuthority1
            , __in        DWORD nSubAuthority2
            , __in        DWORD nSubAuthority3
            , __in        DWORD nSubAuthority4
            , __in        DWORD nSubAuthority5
            , __in        DWORD nSubAuthority6
            , __in        DWORD nSubAuthority7
            , __deref_out PSID *pSid
            )
        {
            return ::AllocateAndInitializeSid(pIdentifierAuthority
                , nSubAuthorityCount
                , nSubAuthority0 
                , nSubAuthority1 
                , nSubAuthority2
                , nSubAuthority3
                , nSubAuthority4 
                , nSubAuthority5 
                , nSubAuthority6
                , nSubAuthority7
                , pSid);
        }

        virtual PVOID FreeSid(__in PSID pSid)
        {
            return ::FreeSid( pSid);
        }

        virtual FARPROC GetProcAddress (
            __in HMODULE hModule,
            __in LPCSTR lpProcName)
        {
            return ::GetProcAddress(hModule, lpProcName);
        }

        virtual VOID GetSystemInfo(	__out LPSYSTEM_INFO lpSystemInfo)
        {
            ::GetSystemInfo(lpSystemInfo);
        }

        virtual VOID GlobalMemoryStatus( __out LPMEMORYSTATUS lpBuffer)
        {
            ::GlobalMemoryStatus(lpBuffer);
        }

        virtual LCID GetSystemDefaultLCID()
        {
            return ::GetSystemDefaultLangID();
        }

        virtual UINT GetSystemDirectory(__out_ecount_opt(uSize) LPWSTR lpBuffer, __in UINT uSize)
        {
            return ::GetSystemDirectory(lpBuffer, uSize);
        }
    };

} // namespace IronMan
