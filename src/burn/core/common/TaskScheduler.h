//-------------------------------------------------------------------------------------------------
// <copyright file="ToString.h" company="Microsoft">
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

class TaskSchedulerUtils
{
public:
    TaskSchedulerUtils() {}
    virtual ~TaskSchedulerUtils() {}

    HRESULT LaunchDeElevated(LPCWSTR wszTaskName, LPCWSTR wstrExecutablePath, LPCWSTR wstrArgs)
    {
        // COM must be initialized outside
        CComPtr<ITaskService> spService;
        HRESULT hr = CoCreateInstance(__uuidof(TaskScheduler), NULL, CLSCTX_INPROC_SERVER, __uuidof(ITaskService), reinterpret_cast<LPVOID*>(&spService));
        if (SUCCEEDED(hr))
        {
            //  Connect to the task service.
            hr = spService->Connect(CComVariant(), CComVariant(), CComVariant(), CComVariant());
            if (SUCCEEDED(hr))
            {
                //  Get the pointer to the root task folder.  This folder will hold the
                //  new task that is registered.
                CComPtr<ITaskFolder> spRootFolder;
                hr = spService->GetFolder(CComBSTR(L"\\"), &spRootFolder);
                if (SUCCEEDED(hr))
                {
                    //  Check if the same task already exists. If the same task exists, remove it.
                    spRootFolder->DeleteTask(CComBSTR(wszTaskName), 0);

                    //  Create the task builder object to create the task.
                    CComPtr<ITaskDefinition> spTaskDefinition;
                    hr = spService->NewTask(0, &spTaskDefinition);
                    if (SUCCEEDED(hr))
                    {
                        //  Get the trigger collection to insert the registration trigger.
                        CComPtr<ITriggerCollection> spTriggerCollection;
                        hr = spTaskDefinition->get_Triggers(&spTriggerCollection);
                        if (SUCCEEDED(hr))
                        {
                            //  Add the registration trigger to the task.
                            CComPtr<ITrigger> spTrigger;
                            hr = spTriggerCollection->Create(TASK_TRIGGER_REGISTRATION, &spTrigger);
                            if (SUCCEEDED(hr))
                            {
                                //  Get the task action collection pointer.
                                CComPtr<IActionCollection> spActionCollection;
                                hr = spTaskDefinition->get_Actions(&spActionCollection);
                                if (SUCCEEDED(hr))
                                {
                                    //  Create the action, specifying that it is an executable action.
                                    CComPtr<IAction> spAction;
                                    hr = spActionCollection->Create(TASK_ACTION_EXEC, &spAction);
                                    if (SUCCEEDED(hr))
                                    {
                                        //  Add an Action to the task.     
                                        CComPtr<IExecAction> spExecAction;
                                        hr = spAction->QueryInterface(__uuidof(IExecAction), (void**)&spExecAction);
                                        if (SUCCEEDED(hr))
                                        {
                                            //  Set the path of the executable to the user supplied executable.
                                            hr = spExecAction->put_Path(CComBSTR(wstrExecutablePath));
                                            if (SUCCEEDED(hr))
                                            {
                                                // add args
                                                hr = spExecAction->put_Arguments(CComBSTR(wstrArgs));
                                                if (SUCCEEDED(hr))
                                                {
                                                    //  Save the task in the root folder.
                                                    CComPtr<IRegisteredTask> spRegisteredTask;
                                                    hr = spRootFolder->RegisterTaskDefinition(CComBSTR(wszTaskName),
                                                        spTaskDefinition,
                                                        TASK_CREATE, 
                                                        CComVariant(CComBSTR(L"S-1-5-32-545")),//Well Known SID for \\Builtin\Users group
                                                        CComVariant(), 
                                                        TASK_LOGON_GROUP,
                                                        CComVariant(L""),
                                                        &spRegisteredTask);
                                                    if (SUCCEEDED(hr))
                                                    {
                                                        // poll for 10 seconds for the task to start
                                                        int i;
                                                        for (i=0; i<100; ++i)
                                                        {
                                                            TASK_STATE taskState;
                                                            spRegisteredTask->get_State(&taskState);
                                                            if (taskState == TASK_STATE_RUNNING)
                                                                break;
                                                            Sleep(100);
                                                        }
                                                        if (i == 100)
                                                            hr = SCHED_E_TASK_NOT_RUNNING;

                                                        // in either case, clean up NOT changing the HRESULT
                                                        
                                                        // Delete the task when done
                                                        spRootFolder->DeleteTask(CComBSTR(wszTaskName), NULL);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return hr;
    }

private: // subclass-and-override test hooks
    virtual HRESULT CoCreateInstance(__in REFCLSID rclsid, __in_opt LPUNKNOWN pUnkOuter, __in DWORD dwClsContext, __in REFIID riid, __deref_out LPVOID FAR* ppv)
    {
        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }
    virtual VOID Sleep(__in DWORD dwMilliseconds)
    {
        ::Sleep(dwMilliseconds);
    }
};

}
