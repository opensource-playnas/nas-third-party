/*
* MODP_B64 - High performance base64 encoder/decoder
* Version 1.3 -- 17-Mar-2006
* http://modp.com/release/base64
* Released under bsd license.  See modp_b64.c for details.
*
* Copyright (c) 2005, 2006  Nick Galbreath -- nickg [at] modp [dot] com
* All rights reserved.
*/

#ifndef __XTLBASE64_H__2011_01_30_
#define __XTLBASE64_H__2011_01_30_
#pragma once

/* if on motoral, sun, ibm; uncomment this */
/* #define WORDS_BIGENDIAN 1 */
/* else for Intel, Amd; uncomment this */
/* #undef WORDS_BIGENDIAN */

/***********************************************
Usage 1:

	std::string strInput = "12345";
	std::string strOutput, strOutput2;
	XTL::CXtlBase64::encode(strInput, &strOutput);

	XTL::CXtlBase64::decode(strOutput, &strOutput2);

************************************************/

#include <string>


using std::string;

namespace XTL
{

class CXtlBase64  
{
public:
	/**
	* Given a source string of length len, this returns the amount of
	* memory the destination string should have.
	*
	* remember, this is integer math
	* 3 bytes turn into 4 chars
	* ceiling[len / 3] * 4 + 1
	*
	* +1 is for any extra null.
	*/
	#define modp_b64_encode_len(A) ((A+2)/3 * 4 + 1)

	/**
	* Given a base64 string of length len,
	*   this returns the amount of memory required for output string
	*  It maybe be more than the actual number of bytes written.
	* NOTE: remember this is integer math
	* this allocates a bit more memory than traditional versions of b64
	* decode  4 chars turn into 3 bytes
	* floor[len * 3/4] + 2
	*/
	#define modp_b64_decode_len(A) (A / 4 * 3 + 2)

	/**
	* Will return the strlen of the output from encoding.
	* This may be less than the required number of bytes allocated.
	*
	* This allows you to 'deserialized' a struct
	* \code
	* char* b64encoded = "...";
	* int len = strlen(b64encoded);
	*
	* struct datastuff foo;
	* if (modp_b64_encode_strlen(sizeof(struct datastuff)) != len) {
	*    // wrong size
	*    return false;
	* } else {
	*    // safe to do;
	*    if (modp_b64_decode((char*) &foo, b64encoded, len) == -1) {
	*      // bad characters
	*      return false;
	*    }
	* }
	* // foo is filled out now
	* \endcode
	*/
	#define modp_b64_encode_strlen(A) ((A + 2)/ 3 * 4)

	// Returns > 0 if successful and < 0 otherwise. 
	static int encode(char* output, const char* input, size_t length);
	// Returns > 0 if successful and < 0 otherwise. 
	static int decode(char* output, const char* input, size_t length);

    static bool encode(string& output, const char* input, size_t length);
    static bool decode(char* output, size_t& output_length, const string& input);

    static bool encode(string& output, const string& input);
    static bool decode(string& output, const string& input);

private:
	CXtlBase64(const CXtlBase64&);
	void operator=(const CXtlBase64&);
};


}; // namespace XTL



#endif // __XTLBASE64_H__2011_01_30_
