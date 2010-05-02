//-------------------------------------------------------------------------------------------------
// <copyright file="PackageData.h" company="Microsoft">
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

#include "interfaces\IOperationData.h"
#include "schema\engineData.h"


//------------------------------------------------------------------------------
// class PackageData
//
// This object is used to encapsulate the data needed by the engine.
// It is added such that we don't have to pass loose variable across objects.
//
//------------------------------------------------------------------------------

namespace IronMan
{
//Note: One of the reasons for using the interface is such that it really help in testing
//      since it is not tie to a specific object.

    class PackageData : public IPackageData
    {
    private:
        const CString m_strName;                  //Name of KB article, or Product
        const CString m_strVersion;               //Version of the Product
        const UxEnum::patchTrainEnum m_servicingTrain;
        const UserExperienceDataCollection::Policy m_policy;
        const CPath m_pthMetricsLoaderExe;        //Name of the Exe that is uploading Metrics data

    public:
        //Constructor
        PackageData(const EngineData& engineData)
            : m_strName(engineData.GetUi().GetName())
            , m_strVersion(engineData.GetUi().GetVersion())
            , m_servicingTrain(engineData.GetConfiguration().GetUserExperienceDataCollectionData().GetServicingTrain())
            , m_policy(engineData.GetConfiguration().GetUserExperienceDataCollectionData().GetPolicy())
            , m_pthMetricsLoaderExe(engineData.GetConfiguration().GetUserExperienceDataCollectionData().GetMetricLoaderExe())
        {}

    public:
        virtual ~PackageData()
        {}

        virtual const CString& GetPackageName() const
        {
            return m_strName;
        }

        virtual const CString& GetVersion() const
        {
            return m_strVersion;
        }

        virtual const UxEnum::patchTrainEnum GetServicingTrain() const
        {
            return m_servicingTrain;
        }

        virtual const UserExperienceDataCollection::Policy GetPolicy() const
        {
            return m_policy;
        }

        virtual const CPath& GetMetricsLoaderExe() const
        {
            return m_pthMetricsLoaderExe;
        }
    };
}
