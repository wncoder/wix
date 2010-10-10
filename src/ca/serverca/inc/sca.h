#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="sca.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    Shared header between scheduling and execution CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#define MAGIC_MULTISZ_CHAR 127

// Generic action enum.
enum SCA_ACTION
{
    SCA_ACTION_NONE,
    SCA_ACTION_INSTALL,
    SCA_ACTION_UNINSTALL
};


// IIS Metabase actions
enum METABASE_ACTION
{
    MBA_UNKNOWNACTION = 0,
    MBA_CREATEKEY,
    MBA_DELETEKEY,
    MBA_WRITEVALUE,
    MBA_DELETEVALUE,
    MBA_CREATEAPP,
    MBA_DELETEAPP,
};

// IIS 7 Config actions
enum IIS_CONFIG_ACTION
{
    IIS_CREATE,
    IIS_CREATE_NEW,
    IIS_DELETE,
    IIS_SITE,
    IIS_APPLICATION,
    IIS_APPPOOL,
    IIS_APPPOOL_RECYCLE_MIN,
    IIS_APPPOOL_RECYCLE_REQ,
    IIS_APPPOOL_RECYCLE_TIMES,
    IIS_APPPOOL_RECYCLE_VIRMEM,
    IIS_APPPOOL_RECYCLE_PRIVMEM,
    IIS_APPPOOL_RECYCLE_IDLTIMEOUT,
    IIS_APPPOOL_RECYCLE_QUEUELIMIT,
    IIS_APPPOOL_RECYCLE_CPU_PCT,
    IIS_APPPOOL_RECYCLE_CPU_REFRESH,
    IIS_APPPOOL_RECYCLE_CPU_ACTION,    
    IIS_APPPOOL_MAXPROCESS,
    IIS_APPPOOL_IDENTITY,
    IIS_APPPOOL_USER,
    IIS_APPPOOL_PWD,
    IIS_APPPOOL_32BIT,
    IIS_APPPOOL_INTEGRATED,
    IIS_APPPOOL_MANAGED_RUNTIME_VERSION,
    IIS_APPPOOL_END,
    IIS_APPEXT_BEGIN,
    IIS_APPEXT,
    IIS_APPEXT_END,
    IIS_VDIR,
    IIS_BINDING,
    IIS_MIMEMAP_BEGIN,
    IIS_MIMEMAP,
    IIS_MIMEMAP_END,
    IIS_DIRPROP_BEGIN, 
    IIS_DIRPROP_ACCESS,
    IIS_DIRPROP_AUTH,
    IIS_DIRPROP_USER,
    IIS_DIRPROP_PWD,
    IIS_DIRPROP_PWDCTRL,
    IIS_DIRPROP_LOG,
    IIS_DIRPROP_DEFDOCS,
    IIS_DIRPROP_SSLFLAGS,
    IIS_DIRPROP_AUTHPROVID,
    IIS_DIRPROP_ASPERROR,
    IIS_DIRPROP_HTTPEXPIRES,
    IIS_DIRPROP_MAXAGE,
    IIS_DIRPROP_CACHECUST,
    IIS_DIRPROP_NOCUSTERROR,
    IIS_DIRPROP_END,
    IIS_WEBLOG,
    IIS_FILTER_BEGIN,
    IIS_FILTER_GLOBAL_BEGIN,
    IIS_FILTER,
    IIS_FILTER_END,
    IIS_HTTP_HEADER_BEGIN,
    IIS_HTTP_HEADER,
    IIS_HTTP_HEADER_END,
    IIS_WEBERROR_BEGIN,
    IIS_WEBERROR,
    IIS_WEBERROR_END,
    IIS_WEB_SVC_EXT,
    IIS_PROPERTY,
    IIS_PROPERTY_MAXBAND,
    IIS_PROPERTY_LOGUTF8,
    IIS_WEBDIR,
    IIS_ASP_BEGIN,
    IIS_ASP_SESSIONSTATE,
    IIS_ASP_SESSIONTIMEOUT,
    IIS_ASP_BUFFER,
    IIS_ASP_PARENTPATHS,
    IIS_ASP_SCRIPTLANG,
    IIS_ASP_SCRIPTTIMEOUT,
    IIS_ASP_SCRIPTSERVERDEBUG,
    IIS_ASP_SCRIPTCLIENTDEBUG,
    IIS_ASP_END,
    IIS_SSL_BINDING
};


// user creation attributes definitions
enum SCAU_ATTRIBUTES
{
    SCAU_DONT_EXPIRE_PASSWRD = 0x00000001,
    SCAU_PASSWD_CANT_CHANGE = 0x00000002,
    SCAU_PASSWD_CHANGE_REQD_ON_LOGIN = 0x00000004,
    SCAU_DISABLE_ACCOUNT = 0x00000008,
    SCAU_FAIL_IF_EXISTS = 0x00000010,
    SCAU_UPDATE_IF_EXISTS = 0x00000020,
    SCAU_ALLOW_LOGON_AS_SERVICE = 0x00000040,

    SCAU_DONT_REMOVE_ON_UNINSTALL = 0x00000100,
    SCAU_DONT_CREATE_USER = 0x00000200,
};

// sql database attributes definitions
enum SCADB_ATTRIBUTES
{
    SCADB_CREATE_ON_INSTALL = 0x00000001,
    SCADB_DROP_ON_UNINSTALL = 0x00000002,
    SCADB_CONTINUE_ON_ERROR = 0x00000004,
    SCADB_DROP_ON_INSTALL = 0x00000008,
    SCADB_CREATE_ON_UNINSTALL = 0x00000010,
    SCADB_CONFIRM_OVERWRITE = 0x00000020,
    SCADB_CREATE_ON_REINSTALL = 0x00000040,
    SCADB_DROP_ON_REINSTALL = 0x00000080,
};

// sql string/script attributes definitions
enum SCASQL_ATTRIBUTES
{
    SCASQL_EXECUTE_ON_INSTALL = 0x00000001,
    SCASQL_EXECUTE_ON_UNINSTALL = 0x00000002,
    SCASQL_CONTINUE_ON_ERROR = 0x00000004,
    SCASQL_ROLLBACK = 0x00000008,
    SCASQL_EXECUTE_ON_REINSTALL = 0x00000010,
};
