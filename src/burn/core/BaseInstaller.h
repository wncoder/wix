//-------------------------------------------------------------------------------------------------
// <copyright file="BaseInstaller.h" company="Microsoft">
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
// This class acts as an is a base class for the different types of installers that
// may be required. It provides default Abort behavior.  
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IPerformer.h"


//------------------------------------------------------------------------------
// IronMan::BaseInstaller header
//
// This class acts as an is a base class for the different types of installers that
// may be required. It provides default Abort behavior.  
//------------------------------------------------------------------------------

namespace IronMan
{
    class BaseInstaller : public AbortBaseT<IPerformer>
    {
        bool m_bAborted;
    public:
        BaseInstaller() : m_bAborted(false) {}
        virtual ~BaseInstaller() {}
        virtual void Abort()
        {
            m_bAborted = true;
        }
        virtual void PerformAction(IProgressObserver& observer)
        {
        }

    protected:
        bool HasAborted() const
        {
            return m_bAborted;
        }
        const bool &GetRefOfAbortedFlag() const
        {
            return m_bAborted;
        }
    private:
        void ResetAbort()
        {
            // use judiciously!
            // this method only exists (in conjunction with the class below),
            // so that I can temporarily turn off the abort flag during rollback
            m_bAborted = false;
        }
    protected:
        class TemporaryAbortResetter
        {
            bool m_bAbort;
            BaseInstaller& m_installer;
        public:
            TemporaryAbortResetter(BaseInstaller& installer)
                : m_installer(installer)
                , m_bAbort(installer.HasAborted())
            {
                m_installer.ResetAbort(); // inner (nested) class has access to outer class's private methods
            }
            ~TemporaryAbortResetter()
            {
                if (m_bAbort)
                    m_installer.Abort();
            }
        };
    };
}
