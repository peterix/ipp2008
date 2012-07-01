//---------------------------------------------------------------------------
//
// Name:        md5.h
// Author:      Kai Krahn
// Created:     11.11.07 12:35

// Description: wxWidgets md5 header
//
// This code implements the MD5 message-digest algorithm.
// The algorithm is due to Ron Rivest. This code is based on the code
// written by Colin Plumb in 1993, no copyright is claimed.
// This code is in the public domain; do with it what you wish.
//
// This version implements the MD5 algorithm for the free GUI toolkit wxWidgets.
// Basic functionality, like MD5 hash creation out of strings and files, or
// to verify strings and files against a given MD5 hash, was added
// by Kai Krahn.
//
// This code is provided "as is" and comes without any warranty!
//---------------------------------------------------------------------------

#ifndef __MD5_h__
#define __MD5_h__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif

//---------------------------------------------------------------------------

/**
    @brief MD5 Class
    
    The MD5Context class encapsulates all steps for MD5 hash creation. The normal 
    procedure is as follows:
    <br>
    - Create a new instance of MD5Context
    - Call MD5Update as often as needed. E.g. you might want to create the MD5 hash 
    of a big file. Therefore you can sequentially read small pieces of the file instead 
    of passing the whole data to MD5Update at once.
    - MD5Final creates the MD5 hash and stores it in it's parameter digest.
    
    @see MD5()
    @see FileMD5()
    @see VerifyMD5()
    @see VerifyFileMD5()   
    
*/
class MD5Context {
    
    private:
        
        uint32 buf[4];
        uint32 bits[2];
        unsigned char in[64];
        
        void MD5Transform(uint32 in[16]);
        void MD5Init();
        
    public:
        
        MD5Context();
        
        void MD5Update(unsigned char *buf, unsigned len);
        void MD5Final(unsigned char digest[16]);
        
};

//---------------------------------------------------------------------------

/**
* MD5 Hash function
* creates the MD5 hash out of a wxString
* @param input wxString for MD5 hash creation
* @return md5 hash as wxString
*/
wxString MD5(wxString input);

/**
* MD5 Hash function
* Creates the MD5 hash of some kind of binary Data
* @param Data char pointer to some input data
* @param Len size of Data
* @see MD5(void* Data, unsigned int Len)
* @return md5 hash as wxString
*/
wxString MD5(unsigned char* Data, unsigned int Len);

/**
* MD5 Hash function
* Creates the MD5 hash of some kind of binary Data
* @param Data void pointer an input buffer
* @param Len size of Data
* @see MD5(unsigned char* Data, unsigned int Len)
* @return md5 hash as wxString
*/
wxString MD5(void* Data, unsigned int Len);

/**
* MD5 Hash function
* Creates the MD5 hash of a file specified in FileName
* @param FileName File to be used for the MD5 algorithm
* @return md5 hash as wxString
*/
wxString FileMD5(wxString FileName);

/**
* MD5 Hash Verification
* Verifies the MD5 Hash of input against a given MD5 Hash specified in md5Hash
* @param MD5Hash Hash to be compared
* @param input wxString to be converted to a MD5Hash
* @return true if equal
* @return false otherwise
*/
bool VerifyMD5(wxString MD5Hash, wxString input);

/**
* MD5 Hash Verification
* Verifies the MD5 Hash of the file FileName against a given MD5 Hash specified in md5Hash
* @param MD5Hash Hash to be compared
* @param FileName path to the file
* @return true if equal
* @return false otherwise
*/
bool VerifyFileMD5(wxString MD5Hash, wxString FileName);


/// for testing purposes
const wxString EMPTY_MD5 = wxT("d41d8cd98f00b204e9800998ecf8427e");

#endif /* !MD5_H */
