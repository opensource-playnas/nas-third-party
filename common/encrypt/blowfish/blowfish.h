// blowfish.h     interface file for blowfish.cpp
// _THE BLOWFISH ENCRYPTION ALGORITHM_
// by Bruce Schneier
// Revised code--3/20/94
// Converted to C++ class 5/96, Jim Conger
//
// modify H.Shirouzu 07/2002 (add change_order(), CBC mode)

#pragma once

#define MAXKEYBYTES		56		// 448 bits max
#define NPASS			16		// SBox passes

#define uint32_t	unsigned int
#define WORD	unsigned short
#define BYTE	unsigned char

class CBlowFish
{
public:
    enum Mode { ECB, CBC /*, OFB, CFB */ };

private:
    uint32_t	*PArray;
    uint32_t	(*SBoxes)[256];
    Mode	mode;
    void	Blowfish_encipher(uint32_t *xl, uint32_t *xr);
    void	Blowfish_decipher(uint32_t *xl, uint32_t *xr);

public:
    CBlowFish();
    ~CBlowFish();
    void Initialize(const BYTE key[], int keybytes, Mode _mode=ECB);
    uint32_t	GetOutputLength(uint32_t lInputLong);
    uint32_t	Encode(BYTE * pInput, BYTE * pOutput, uint32_t lSize);
    uint32_t	Decode(BYTE * pInput, BYTE * pOutput, uint32_t lSize);
};

// choose a byte order for your hardware
#define ORDER_DCBA	// chosing Intel in this case

union aword {
    uint32_t	dword;
    BYTE	byte[4];
    struct {
#ifdef ORDER_ABCD  	// ABCD - big endian - motorola
        BYTE byte0;
        BYTE byte1;
        BYTE byte2;
        BYTE byte3;
#endif
#ifdef ORDER_DCBA  	// DCBA - little endian - intel
        BYTE byte3;
        BYTE byte2;
        BYTE byte1;
        BYTE byte0;
#endif
#ifdef ORDER_BADC  	// BADC - vax
        BYTE byte1;
        BYTE byte0;
        BYTE byte3;
        BYTE byte2;
#endif
    } w;
};


