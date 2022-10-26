// basetype.h : Declaration of some base types

#pragma once

typedef enum tagEnWordHDCategory
{
	WordHDCat_NULL		=0x00,	
	WordHDCat_COMMENTS	=0x01,	// comments and annotations
	WordHDCat_PROP		=0x02,	// document properties and personal information
	WordHDCat_XML		=0x04,	// 2K7 only. Custom XML data.
	WordHDCat_HEADFOOT	=0x08,	// headers, footers, wartermarkers
	WordHDCat_TEXT		=0x10,	// 2K7 only. Hidden text
	WordHDCat_ALL		=0x1F
} EnWordHDCategory;

typedef enum tagEnExcelHDCategory
{
	ExcelHDCat_NULL		=0x00,	
	ExcelHDCat_COMMENTS	=0x01, 
	ExcelHDCat_PROP		=0x02,
	ExcelHDCat_XML		=0x04,	// 2K7 only
	ExcelHDCat_HEADFOOT	=0x08, 
	ExcelHDCat_ROWCOL	=0x10,	// hidden row and columns
	ExcelHDCat_WORKSHEET=0x20,	// hidden worksheets
	ExcelHDCat_INVISCONTENT=0x40,	// 2K7 only. Invisible content
	ExcelHDCat_FILTER	=0x80,		// Filters in spreadsheet
	ExcelHDCat_ALL		=0xFF
} EnExcelHDCategory;

typedef enum tagEnPptHDCategory
{
	PptHDCat_NULL		=0x00,	
	PptHDCat_COMMENTS	=0x01, 
	PptHDCat_PROP		=0x02,
	PptHDCat_XML		=0x04,	// 2K7 only
	PptHDCat_ONSLIDE	=0x08,	// 2K7 only. Invisible On-Slide Content
	PptHDCat_OFFSLIDE	=0x10,	// 2K7 only. Off-Slide Content
	PptHDCat_NOTE		=0x20,	// 2K7 only. Presentation Notes
	PptHDCat_ALL		=0x3F
} EnPptHDCategory;

#define RBWordHDCat_ALL			(WordHDCat_ALL)
#define RBWordHDCat_COMMENTS	(WordHDCat_COMMENTS)
#define RBWordHDCat_PROP		(WordHDCat_PROP)
#define RBWordHDCat_XML			(WordHDCat_XML)
#define RBWordHDCat_HEADFOOT	(WordHDCat_HEADFOOT)
#define RBWordHDCat_TEXT		(WordHDCat_TEXT)

#define RBExcelHDCat_ALL		(ExcelHDCat_ALL<<8)
#define RBExcelHDCat_COMMENTS	(ExcelHDCat_COMMENTS<<8)
#define RBExcelHDCat_PROP		(ExcelHDCat_PROP<<8)
#define RBExcelHDCat_XML		(ExcelHDCat_XML<<8)
#define RBExcelHDCat_HEADFOOT	(ExcelHDCat_HEADFOOT<<8)
#define RBExcelHDCat_ROWCOL		(ExcelHDCat_ROWCOL<<8)
#define RBExcelHDCat_WORKSHEET	(ExcelHDCat_WORKSHEET<<8)
#define RBExcelHDCat_INVISCONTENT	(ExcelHDCat_INVISCONTENT<<8)
#define RBExcelHDCat_FILTER		(ExcelHDCat_FILTER<<8)

#define RBPptHDCat_ALL			(PptHDCat_ALL<<16)
#define RBPptHDCat_COMMENTS		(PptHDCat_COMMENTS<<16)
#define RBPptHDCat_PROP			(PptHDCat_PROP<<16)
#define RBPptHDCat_XML			(PptHDCat_XML<<16)
#define RBPptHDCat_ONSLIDE		(PptHDCat_ONSLIDE<<16)
#define RBPptHDCat_OFFSLIDE		(PptHDCat_OFFSLIDE<<16)
#define RBPptHDCat_NOTE			(PptHDCat_NOTE<<16)
