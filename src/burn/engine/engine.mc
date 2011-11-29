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

; // MessageId=#
; // Severity=Success
; // SymbolicName=MSG_SUCCESS
; // Language=English
; // Success %1.
; // .
;
; // MessageId=#
; // Severity=Warning
; // SymbolicName=MSG_WARNING
; // Language=English
; // Warning %1.
; // .
;
; // MessageId=#
; // Severity=Error
; // SymbolicName=MSG_ERROR
; // Language=English
; // Error %1.
; // .

MessageId=1
Severity=Success
SymbolicName=MSG_BURN_INFO
Language=English
Burn v%1!hs!, path: %2!ls!, cmdline: '%3!ls!'
.

MessageId=2
Severity=Warning
SymbolicName=MSG_BURN_UNKNOWN_PRIVATE_SWITCH
Language=English
Unknown burn internal commandline switch encountered: '%1!ls!'.
.

MessageId=3
Severity=Success
SymbolicName=MSG_BURN_RUN_BY_RELATED_BUNDLE
Language=English
This bundle is being run by a related bundle as type '%1!hs!'.
.

MessageId=51
Severity=Error
SymbolicName=MSG_FAILED_PARSE_CONDITION
Language=English
Error %1!hs!. Failed to parse condition %2!ls!. Unexpected symbol at position %3!hs!
.

MessageId=52
Severity=Success
SymbolicName=MSG_CONDITION_RESULT
Language=English
Condition '%1!ls!' evaluates to %2!hs!.
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

MessageId=55
Severity=Warning
SymbolicName=MSG_CANNOT_LOAD_STATE_FILE
Language=English
Could not load or read state file: %2!ls!, error: 0x%1!x!.
.

MessageId=56
Severity=Error
SymbolicName=MSG_USER_CANCELED
Language=English
Application canceled operation: %2!ls!, error: %1!ls!
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
Detected package: %1!ls!, state: %2!hs!, cached: %3!hs!
.

MessageId=102
Severity=Success
SymbolicName=MSG_DETECTED_RELATED_BUNDLE
Language=English
Detected related bundle: %1!ls!, scope: %2!hs!, version: %3!hs!, operation: %4!hs!
.

MessageId=103
Severity=Success
SymbolicName=MSG_DETECTED_RELATED_PACKAGE
Language=English
Detected related package: %1!ls!, scope: %2!hs!, version: %3!hs!, language: %4!u! operation: %5!hs!
.

MessageId=104
Severity=Success
SymbolicName=MSG_DETECTED_MSI_FEATURE
Language=English
Detected package: %1!ls!, feature: %2!ls!, state: %3!hs!
.

MessageId=105
Severity=Success
SymbolicName=MSG_DETECT_MSI_FEATURES
Language=English
Detect %1!u! msi features for package: %2!ls!
.

MessageId=151
Severity=Error
SymbolicName=MSG_FAILED_DETECT_PACKAGE
Language=English
Detect failed for package: %2!ls!, error: %1!ls!
.

MessageId=152
Severity=Error
SymbolicName=MSG_FAILED_READ_RELATED_PACKAGE_LANGUAGE
Language=English
Detected related package: %2!ls!, but failed to read language: %3!hs!, error: 0x%1!x!
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
Plan %1!u! packages, action: %2!hs!
.

MessageId=201
Severity=Success
SymbolicName=MSG_PLANNED_PACKAGE
Language=English
Planned package: %1!ls!, state: %2!hs!, default requested: %3!hs!, ux requested: %4!hs!, execute: %5!hs!, rollback: %6!hs!, cache: %7!hs!, uncache: %8!hs!, dependency: %9!hs!
.

MessageId=202
Severity=Success
SymbolicName=MSG_PLANNED_BUNDLE_UX_CHANGED_REQUEST
Language=English
Planned bundle: %1!ls!, UX requested state: %2!hs! over default: %3!hs!
.

MessageId=203
Severity=Success
SymbolicName=MSG_PLANNED_MSI_FEATURE
Language=English
Planned feature: %1!ls!, state: %2!hs!, requested: %3!hs!, execute action: %4!hs!, rollback action: %5!hs!
.

MessageId=204
Severity=Success
SymbolicName=MSG_PLAN_MSI_FEATURES
Language=English
Plan %1!u! msi features for package: %2!ls!
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
Applying package: %1!ls!, action: %2!hs!, path: %3!ls!, arguments: '%4!ls!'
.

MessageId=311
Severity=Error
SymbolicName=MSG_FAILED_ACQUIRE_CONTAINER
Language=English
Failed to acquire container: %2!ls! to working path: %3!ls!, error: %1!ls!.
.

MessageId=312
Severity=Error
SymbolicName=MSG_FAILED_EXTRACT_CONTAINER
Language=English
Failed to extract payloads from container: %2!ls! to working path: %3!ls!, error: %1!ls!.
.

MessageId=313
Severity=Error
SymbolicName=MSG_FAILED_ACQUIRE_PAYLOAD
Language=English
Failed to acquire payload: %2!ls! to working path: %3!ls!, error: %1!ls!.
.

MessageId=314
Severity=Error
SymbolicName=MSG_FAILED_CACHE_PAYLOAD
Language=English
Failed to cache payload: %2!ls! from working path: %3!ls!, error: %1!ls!.
.

MessageId=315
Severity=Error
SymbolicName=MSG_FAILED_LAYOUT_BUNDLE
Language=English
Failed to layout bundle: %2!ls! to layout directory: %3!ls!, error: %1!ls!.
.

MessageId=316
Severity=Error
SymbolicName=MSG_FAILED_LAYOUT_PAYLOAD
Language=English
Failed to layout payload: %2!ls! to layout directory: %3!ls!, error: %1!ls!.
.

MessageId=320
Severity=Success
SymbolicName=MSG_DEPENDENCY_BUNDLE_REGISTER
Language=English
Registering bundle dependency provider: %1!ls!, version: %2!ls!
.

MessageId=321
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_SKIP_NOPROVIDERS
Language=English
Skipping dependency registration on package with no dependency providers: %1!ls!
.

MessageId=322
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_SKIP_WRONGSCOPE
Language=English
Skipping cross-scope dependency registration on package: %1!ls!, bundle scope: %2!hs!, package scope: %3!hs!
.

MessageId=323
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_REGISTER
Language=English
Registering package dependency provider: %1!ls!, version: %2!ls!, package: %3!ls!
.

MessageId=324
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_SKIP_MISSING
Language=English
Skipping dependency registration on missing package provider: %1!ls!, package: %2!ls!
.

MessageId=325
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_REGISTER_DEPENDENCY
Language=English
Registering dependency: %1!ls! on package provider: %2!ls!, package: %3!ls!
.

MessageId=326
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_UNREGISTER_DEPENDENCY
Language=English
Removing dependency: %1!ls! on package provider: %2!ls!, package %3!ls!
.

MessageId=327
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_HASDEPENDENTS
Language=English
Will not uninstall package: %1!ls!, found dependents: %2!d!
.

MessageId=328
Severity=Warning
SymbolicName=MSG_DEPENDENCY_PACKAGE_DEPENDENT
Language=English
Found dependent: %1!ls!, name: %2!ls!
.

MessageId=329
Severity=Success
SymbolicName=MSG_DEPENDENCY_PACKAGE_UNREGISTER
Language=English
Removing package dependency provider: %1!ls!, package: %2!ls!
.

MessageId=330
Severity=Success
SymbolicName=MSG_DEPENDENCY_BUNDLE_UNREGISTER
Language=English
Removing bundle dependency provider: %1!ls!
.

MessageId=348
Severity=Warning
SymbolicName=MSG_APPLY_RETRYING_PACKAGE
Language=English
Application requested retry of package: %1!ls!, encountered error: 0x%2!x!. Retrying...
.

MessageId=349
Severity=Warning
SymbolicName=MSG_APPLY_RETRYING_PAYLOAD
Language=English
Application requested retry of payload: %1!ls!, encountered error: 0x%2!x!. Retrying...
.

MessageId=350
Severity=Warning
SymbolicName=MSG_APPLY_CONTINUING_NONVITAL_PACKAGE
Language=English
Applying non-vital package: %1!ls!, encountered error: 0x%2!x!. Continuing...
.

MessageId=351
Severity=Success
SymbolicName=MSG_UNCACHE_PACKAGE
Language=English
Removing cached package: %1!ls!, from path: %2!ls!
.

MessageId=352
Severity=Success
SymbolicName=MSG_UNCACHE_BUNDLE
Language=English
Removing cached bundle: %1!ls!, from path: %2!ls!
.

MessageId=399
Severity=Success
SymbolicName=MSG_APPLY_COMPLETE
Language=English
Apply complete, result: 0x%1!x! restart: %2!hs!
.

MessageId=400
Severity=Success
SymbolicName=MSG_SYSTEM_SHUTDOWN
Language=English
Received system request to shut down the process: critical: %1!hs!, elevated: %2!hs!, allowed: %3!hs!
.

MessageId=500
Severity=Success
SymbolicName=MSG_QUIT
Language=English
Shutting down, exit code: 0x%1!x!
.

MessageId=501
Severity=Warning
SymbolicName=MSG_STATE_NOT_SAVED
Language=English
The state file could not be saved, error: 0x%1!x!. Continuing...
.
