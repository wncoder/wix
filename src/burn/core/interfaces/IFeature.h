//-------------------------------------------------------------------------------------------------
// <copyright file="IFeature.h" company="Microsoft">
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

template<typename T> class ITreeT
{
protected:
    virtual ~ITreeT() {}
public:
    virtual unsigned int GetNumberOfChildren() = 0;
    virtual T* GetChild(unsigned int) = 0;
};

class IFeature : public ITreeT<IFeature>
{
protected:
    virtual ~IFeature() {}
public:
    virtual const CString& GetTextId() = 0;
    virtual const CString& GetDescriptionId() = 0;
    virtual bool GetDefaultSelectionState() = 0;
    virtual void SetSelectionState(bool) = 0;
};

class IFeatureTreeRoot : public ITreeT<IFeature>
{
protected:
    virtual ~IFeatureTreeRoot() {}
};

}
