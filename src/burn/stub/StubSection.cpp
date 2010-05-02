 #include "precomp.h"

#pragma section(".wixburn",read)

// If these defaults ever change, be sure to update GetEmbeddedContainerHeader() in payloads.cpp as well
#pragma data_seg(push, ".wixburn")
static DWORD dwMagic = 0x00f14300;
static DWORD dwVersion = 0x00000001;
static DWORD dwManifestOffset = MAXDWORD;
static DWORD dwManifestSize = MAXDWORD;
static DWORD dwAttachedContainerOffset = MAXDWORD;
static DWORD dwAttachedContainerSize = MAXDWORD;
#pragma data_seg(pop)
