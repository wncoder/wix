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
SymbolicName=MSG_BURN_INFO
Language=English
Burn v%1!s!, path: %2!ls!, cmdline: '%3!ls!'
.

; // MessageId=2
; // Severity=Warning
; // SymbolicName=MSG_WARNING
; // Language=English
; // Warning %1.
; // .
;
; // MessageId=4
; // Severity=Error
; // SymbolicName=MSG_ERROR
; // Language=English
; // Error %1.
; // .

MessageId=51
Severity=Error
SymbolicName=MSG_FAILED_PARSE_CONDITION
Language=English
Error %1. Failed to parse condition %2!ls!. Unexpected symbol at position %3
.

MessageId=52
Severity=Success
SymbolicName=MSG_CONDITION_RESULT
Language=English
Condition '%1!ls!' evaluates to %2.
.

MessageId=53
Severity=Error
SymbolicName=MSG_FAILED_CONDITION_CHECK
Language=English
Bundle global condition check didn't succeed - aborting without loading application.
.

MessageId=54
Severity=Error
SymbolicName=MSG_PAYLOAD_FILE_NOT_PRESENT
Language=English
Failed to resolve source for file: %2!ls!, error: %1!ls!.
.

MessageId=100
Severity=Success
SymbolicName=MSG_DETECT_BEGIN
Language=English
Detect %1!u! packages
.

MessageId=101
Severity=Success
SymbolicName=MSG_DETECTED_PACKAGE
Language=English
Detected package: %1!ls!, state: %2, cached: %3
.

MessageId=102
Severity=Success
SymbolicName=MSG_DETECTED_RELATED_BUNDLE
Language=English
Detected related bundle: %1!ls!, scope: %2, version: %3, operation: %4
.

MessageId=103
Severity=Success
SymbolicName=MSG_DETECTED_RELATED_PACKAGE
Language=English
Detected related package: %1!ls!, scope: %2, version: %3, operation: %4
.

MessageId=151
Severity=Error
SymbolicName=MSG_FAILED_DETECT_PACKAGE
Language=English
Detect failed for package: %2!ls!, error: 0x%1!x!
.

MessageId=199
Severity=Success
SymbolicName=MSG_DETECT_COMPLETE
Language=English
Detect complete, result: 0x%1!x!
.

MessageId=200
Severity=Success
SymbolicName=MSG_PLAN_BEGIN
Language=English
Plan %1!u! packages, action: %2
.

MessageId=201
Severity=Success
SymbolicName=MSG_PLANNED_PACKAGE
Language=English
Planned package: %1!ls!, state: %2, default requested: %3, ux requested: %4, execute: %5, rollback: %6, cache: %7, uncache: %8
.

MessageId=202
Severity=Success
SymbolicName=MSG_PLANNED_BUNDLE_UX_CHANGED_REQUEST
Language=English
Planned bundle: %1!ls!, UX requested state: %2 over default: %3
.

MessageId=299
Severity=Success
SymbolicName=MSG_PLAN_COMPLETE
Language=English
Plan complete, result: 0x%1!x!
.

MessageId=300
Severity=Success
SymbolicName=MSG_APPLY_BEGIN
Language=English
Apply begin
.

MessageId=301
Severity=Success
SymbolicName=MSG_APPLYING_PACKAGE
Language=English
Applying package: %1!ls!, action: %2, path: %3!ls!, arguments: '%4!ls!'
.

MessageId=350
Severity=Warning
SymbolicName=MSG_APPLY_CONTINUING_NONVITAL_PACKAGE
Language=English
Applying non-vital package: %1!ls!, encountered error: 0x%2!x!. Continuing...
.

MessageId=399
Severity=Success
SymbolicName=MSG_APPLY_COMPLETE
Language=English
Apply complete, result: 0x%1!x! restart: %2
.

MessageId=500
Severity=Success
SymbolicName=MSG_QUIT
Language=English
Shutting down, exit code: 0x%1!x!
.
