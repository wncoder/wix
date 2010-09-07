 #include "precomp.h"

#pragma section(".wixburn",read)

// If these defaults ever change, be sure to update GetEmbeddedContainerHeader() in payloads.cpp as well
#pragma data_seg(push, ".wixburn")
static DWORD dwMagic = 0x00f14300;
static DWORD dwVersion = 0x00000001;
static DWORD dwContainerInfo = 0;
static DWORD dwReserved = 0;
static DWORD64 qwManifestOffset = MAXDWORD64;
static DWORD64 qwManifestSize = MAXDWORD64;
static DWORD64 qwAttachedContainerOffset = MAXDWORD64;
static DWORD64 qwAttachedContainerSize = MAXDWORD64;
#pragma data_seg(pop)
