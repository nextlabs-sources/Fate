/*____________________________________________________________________________
	Copyright (C) 2005 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpNISTtests_h	/* [ */
#define Included_pgpNISTtests_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

/* NIST RSA Validation System (RSAVS)  Tests */
 
 
enum PGPRSAVSTestType_
{
	kPGPRSAVSTest_KeyGen				= 1,
  	kPGPRSAVSTest_SigGen,
	kPGPRSAVSTest_SigVer	,
 
	PGP_ENUM_FORCE( PGPRSAVSTestType_ )
};
PGPENUM_TYPEDEF( PGPRSAVSTestType_, PGPRSAVSTestType );


typedef struct _PGPRSAVSTestParam  {
	PGPRSAVSTestType	test;
		PGPUInt32		bits;
	 PGPHashAlgorithm	hashAlgor;
	  
		/* in */
	   PGPByte const*	E;	/* public exponent */
	   PGPSize 			ELen;
	   
	   PGPByte const*	Xp;
	   PGPSize 			XpLen;
	   
	   PGPByte const*	Xp1;
	   PGPSize 			Xp1Len;
	   
	   PGPByte const*	Xp2;
	   PGPSize 			Xp2Len;
	   
	   PGPByte const*	Xq;	
	   PGPSize 			XqLen;
	   
	   PGPByte const*	Xq1;
	   PGPSize 			Xq1Len;
	   
	   PGPByte const*	Xq2;
	   PGPSize 			Xq2Len;
	   
		/* in, out */
	   PGPByte*			P;
	   PGPSize			PLen;
	   
	   PGPByte*			Q;
	   PGPSize			QLen;
	   
	   PGPByte*			N;
	   PGPSize 			NLen;
	   
	   PGPByte*			D;
	   PGPSize			DLen;
 	   
	   PGPByte*			Hash;			/* Hashed Message  */
	   PGPSize			HashLen;
	   
	   PGPByte*			S;				/* Signature value S */
	   PGPSize			SLen;

} PGPRSAVSTestParam;


PGPError 	PGPRSAVSTest(
				   PGPContextRef			context,
				   PGPRSAVSTestParam*		params );


/* NIST DSA Validation System (DSAVS)  Tests */

 
/* DSAVS Tests 		*/
enum PGPDSAVSTestType_
{
	kPGPDSAVSTest_PQGGen				= 1,
 	kPGPDSAVSTest_PQGVer,
	kPGPDSAVSTest_KeyPair,
	kPGPDSAVSTest_SigGen,
	kPGPDSAVSTest_SigVer	,
 
	PGP_ENUM_FORCE( PGPDSAVSTestType_ )
};
PGPENUM_TYPEDEF( PGPDSAVSTestType_, PGPDSAVSTestType );
		
	
typedef struct _PGPDSAVSTestParam  {
	  PGPDSAVSTestType	test;
	   PGPUInt32		bits;			/* always 1024 */
   
		/* in, out */
	   PGPByte*			P;			 	/* Prime Modulus */
	   PGPSize			PLen;
	   
	   PGPByte*			Q;				/* Prime Divisor of p-1 */
	   PGPSize			QLen;
	   
	   PGPByte*			G;				/* A group element of the order Q */
	   PGPSize			GLen;
	   
	   PGPByte*			H;				/* the value used to generate G */
	   PGPSize			HLen;

	   PGPByte*			X;				/* Private Key */
	   PGPSize			XLen;
	   
	   PGPByte*			Y;				/* Public Key */
	   PGPSize			YLen;
	   
	   PGPByte*			R;				/* Signature value R */
	   PGPSize			RLen;
	   
	   PGPByte*			S;				/* Signature value S */
	   PGPSize			SLen;

	   PGPByte*			Hash;			/* Hashed Message  */
	   PGPSize			HashLen;

	   PGPByte*			Seed;			/* Seed used to generate Q */
	   PGPSize			SeedLen;

	   PGPUInt32		counter;		/* the value of the counter output from 
										the generation of P   */
 } PGPDSAVSTestParam;

PGPError 	PGPDSAVSTest(
				   PGPContextRef			context,
				   PGPDSAVSTestParam*	params );


/* NIST ECDSA Validation System (ECDSAVS)  Tests */

 
/* ECDSAVS Tests 		*/
enum PGPECDSAVSTestType_
{
	kPGPECDSAVSTest_PKV				= 1,
 	kPGPECDSAVSTest_KeyPair,
	kPGPECDSAVSTest_SigGen,
	kPGPECDSAVSTest_SigVer	,
 
	PGP_ENUM_FORCE( PGPECDSAVSTestType_ )
};
PGPENUM_TYPEDEF( PGPECDSAVSTestType_, PGPECDSAVSTestType );
	
enum PGPECType_ 
{
	kCurve_P_192 = 1,	// secp192r* ?
	kCurve_P_224,		// secp224r* ?
	kCurve_P_256,		// secp256r1
	kCurve_P_384,		// secp384r1
	kCurve_P_409,		// secp409r* ?
	kCurve_P_521,		// secp521r1
	
	kCurve_K_163,		// sect163k1
	kCurve_K_233,		// sect233k* ?
	kCurve_K_283,		// sect283k1
	kCurve_K_409,		// sect409k1
	kCurve_K_571,		// sect571k* ?

	kCurve_B_163,		// sect163r2
	kCurve_B_233,		// sect233r* ?
	kCurve_B_283,		// sect283r1
	kCurve_B_409,		// sect409r1
	kCurve_B_571,		// sect571r* ?
	
	PGP_ENUM_FORCE( PGPECType_ )
};

PGPENUM_TYPEDEF( PGPECType_, PGPECType );
	
typedef struct _PGPECDSAVSTestParam  {
	   PGPECDSAVSTestType	test;
	   PGPECType		curve;			/* the specific curve to use */
   
		/* in, out */
	   PGPByte*			d;			 	/* Private key */
	   PGPSize			dLen;
	   
	   PGPByte*			Qx;				/* X-coordinate of public key */
	   PGPSize			QxLen;
	   
	   PGPByte*			Qy;				/* Y-coordinate of public key */
	   PGPSize			QyLen;
	   
	   PGPByte*			R;				/* Signature value R */
	   PGPSize			RLen;
	   
	   PGPByte*			S;				/* Signature value S */
	   PGPSize			SLen;

	   PGPByte*			Hash;			/* Hashed Message  */
	   PGPSize			HashLen;
	   		
	   PGPByte*			Seed;			/* Seed */
	   PGPSize			SeedLen;

} PGPECDSAVSTestParam;

PGPError 	PGPECDSAVSTest(
				   PGPContextRef			context,
				   PGPECDSAVSTestParam*	params );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpNISTtests_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
