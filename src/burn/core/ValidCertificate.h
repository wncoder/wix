//-------------------------------------------------------------------------------------------------
// <copyright file="ValidCertificate.h" company="Microsoft">
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
//------------------------------------------------------------------------------
// Class: IronMan::ValidCertificate
//
// Contains static function to determine if a file is signed with a valid
// certificate. This is also the class which we allows the 3rd party
// certificates specified in ParameterInfo.xml to be used to check the
// signing certificate.
//------------------------------------------------------------------------------
class ValidCertificate
{
    static const DWORD SHA1_HASH_LEN = 20;

public:
    static const bool IsValidCertificate(const AcceptableCertificates & acceptableCertificates,
                                         const BYTE* pbSigningThumbprint,
                                         const BYTE *rgbKeyId,
                                         const DWORD cbKeyId)
    {
        bool fWasValid = false;

        // Check with all acceptable certificates
        for (int i = 0; i < acceptableCertificates.GetSize(); ++i)
        {
            if (ValidCertificate::CompareKeyIds(rgbKeyId, cbKeyId, acceptableCertificates[i].GetAuthorityKeyIdentifier(), SHA1_HASH_LEN) )
            {
                // If a specific Thumbprint for the signing certificate is necessary then check that
                if (acceptableCertificates[i].GetThumbprintString().IsEmpty())
                {
                    // Flag as valid and exit loop
                    fWasValid = true;
                    break;
                }
                else
                {
                    // This authoring insists on the signing thumbprint matching as well as the root being OK
                    if (pbSigningThumbprint  &&  ValidCertificate::CompareKeyIds(pbSigningThumbprint, SHA1_HASH_LEN, acceptableCertificates[i].GetThumbprint(), SHA1_HASH_LEN) )
                    {
                        // Flag as valid and exit loop
                        fWasValid = true;
                        break;
                    }
                }
            }
        }

        // Return result
        return fWasValid;
    }
private:
    static const bool CompareKeyIds(const BYTE *rgbKeyId1, const DWORD cbKeyId1, const BYTE *rgbKeyId2, const DWORD cbKeyId2)
    {
            // key lengths must match
            if( cbKeyId1 != cbKeyId2)
            {
                return false;
            }
            if ( memcmp(rgbKeyId1, rgbKeyId2, cbKeyId1) == 0 )
                return true;
            else
                return false;
    }
};
}
