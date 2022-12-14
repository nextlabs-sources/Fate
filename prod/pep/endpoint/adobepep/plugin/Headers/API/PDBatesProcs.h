/*
** PDBatesProcs.h
** (c) Copyright 2004, Adobe Systems, Inc.  All Rights Reserved.
*/

/**
* Adds Bates Numbering to a PDF document.
*
* @param pdDoc			The PDF document to which Bates Numbering is added.
* @param batesParams	Bates Numbering specific parameters, which include the start number, number of digits, prefix, and suffix.
* @param params			All layout parameters that are related to how Bates Numbering is placed on the page.
* @return <code>true</code> if Bates Numbering was successfully added, <code>false</code> otherwise.
* @exception pdErrBadAction
* @see PDDocRemoveBatesNumbering 
* @since PI_PDMODEL_VERSION >= 0x00090000
*
**/ 
XNPROC(ASBool, PDDocAddBatesNumbering, (PDDoc pdDoc, PDDocBatesNumberingParams batesParams, PDDocLayoutParamsRec params))
/**
* Remove Bates Numbering from a PDF document.
* @param pdDoc   The PDF document from which Bates Numbering is to be removed.
* @return <code>true</code> if Bates Numbering was successfully removed, <code>false</code> otherwise.
* @exception pdErrBadAction
* @see PDDocAddBatesNumbering 
* @since PI_PDMODEL_VERSION >= 0x00090000
*/
XNPROC(ASBool, PDDocRemoveBatesNumbering, (PDDoc pdDoc))
