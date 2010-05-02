; //-------------------------------------------------------------------------------------------------
; // <copyright file="core.mc" company="Microsoft">
; //    Copyright (c) Microsoft Corporation.  All rights reserved.
; // </copyright>
; //
; // <summary>
; //    Message definitions for the Burn engine.
; // </summary>
; //-------------------------------------------------------------------------------------------------


; // header section

MessageIdTypedef=DWORD

LanguageNames=(English=0x409:MSG00409)


; // message definitions

MessageId=1
Severity=Success
SymbolicName=MSG_SUCCEEDED
Language=English
Success %1.
.

MessageId=2
Severity=Warning
SymbolicName=MSG_WARNING
Language=English
Warning %1.
.

MessageId=4
Severity=Error
SymbolicName=MSG_ERROR
Language=English
Error %1.
.
