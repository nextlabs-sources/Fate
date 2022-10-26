/*****************************************************************************
*
* File: opclib.cpp
*
* Description:
* This file contains definitions of utility functions that wrap some
* common operations needed when working with most Open Packaging Conventions
* (OPC) conformant files. The OPC specification is ECMA-376 Part 2.
*
* ------------------------------------
*
*  This file is part of the Microsoft Windows SDK Code Samples.
*
*  Copyright (C) Microsoft Corporation.  All rights reserved.
*
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
****************************************************************************/
#include "stdafx.h"
#include "stdio.h"
#include "windows.h"
#include "shlobj.h"
#include "msxml6.h"
#include "opclib.h"
#include "util.h"
#include <atlbase.h>
#include<vector>
#include "nlofficerep_only_debug.h"
//#include "OfficeTagLib.h"
namespace opclib
{
    // The definitive way to find a part of interest in a package, is to  use a
    // relationship type to find the relationship that targets the part and resolve
    // the part name.

    // The relationship type (core-properties relationship type) of the
    // relationship targeting the Core Properties part, as defined by the OPC
    // (ECMA-376 Part 2 Part 1).
    static const WCHAR g_corePropertiesRelationshipType[] = L"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";

    static const WCHAR g_extendedPropertiesRelationshipType[] = L"http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties";

    static const WCHAR g_customPropertiesRelationshipType[] = L"http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties";

    // The expected content type of the Core Properties part, as defined by the OPC
    // (ECMA-376 Part 2, Part 1).

    static const WCHAR g_corePropertiesContentType[] = L"application/vnd.openxmlformats-package.core-properties+xml";
    static const WCHAR g_extendedPropertiesContentType[] = L"application/vnd.openxmlformats-officedocument.extended-properties+xml";
    static const WCHAR g_customPropertiesContentType[] = L"application/vnd.openxmlformats-officedocument.custom-properties+xml";

    // Namespaces used in XPath selection queries for the Core Properties part.
    // This includes a number of namespaces used in the Core Properties 
    // part as specified by the OPC.
    static const WCHAR g_corePropertiesSelectionNamespaces[] =
        L"xmlns:cp='http://schemas.openxmlformats.org/package/2006/metadata/core-properties' "
        L"xmlns:dc='http://purl.org/dc/elements/1.1/' "
        L"xmlns:dcterms='http://purl.org/dc/terms/' "
        L"xmlns:dcmitype='http://purl.org/dc/dcmitype/' "
        L"xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'";

    static const WCHAR g_extendedPropertiesSelectionNamespaces[] =
        L"xmlns:extend='http://schemas.openxmlformats.org/officeDocument/2006/extended-properties' "
        L"xmlns:vt='http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes' ";
    static const WCHAR g_customPropertiesSelectionNamespaces[] =
        L"xmlns:customprop='http://schemas.openxmlformats.org/officeDocument/2006/custom-properties' "
        L"xmlns:vt='http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes'";


    static const WCHAR g_CustomPropertiesPartUrl[] = L"/docProps/custom.xml";
    //////////////////////////////////////////////////////////////////////////////
    // Description:
    // Load a package file into a package object to be read.
    //////////////////////////////////////////////////////////////////////////////
    HRESULT
        LoadPackage(
        IOpcFactory* pFactory,
        IStream* pFileStream,
        IOpcPackage** pOutPackage
        )
    {
            // Note: Do not use a writable stream to overwrite the data of a package
            // that is read.
            // Create a read-only stream over the package to prevent errors caused by
            // simultaneously writing and reading from a package.
            HRESULT hr = pFactory->ReadPackageFromStream(
                pFileStream,
                OPC_VALIDATE_ON_LOAD,
                pOutPackage
                );

            return hr;
    }
    HRESULT LoadPackage(IOpcFactory *pFactory, LPCWSTR lpPackageName, IOpcPackage **pOutPackage)
    {
        IStream * pSourceFileStream = NULL;
        // Note: Do not use a writable stream to overwrite the data of a package
        // that is read.

        // Create a read-only stream over the package to prevent errors caused by
        // simultaneously writing and reading from a package.
        //HRESULT hr = pFactory->CreateStreamOnFile(lpPackageName, OPC_STREAM_IO_READ, NULL, 0, &pSourceFileStream);
        HRESULT hr = SHCreateStreamOnFile(lpPackageName, STGM_SHARE_DENY_NONE, &pSourceFileStream);
        if (SUCCEEDED(hr)&&pSourceFileStream!=NULL)
        {
            // Note: If a part is modified, it is accessed at least twice for
            // reading and writing. Use the OPC_CACHE_ON_ACCESS flag to reduce
            // overhead incurred by accessing a package component (in this case, a
            // part) multiple times.

            // Read the package into a package object.
            // Note: A stream used to read a package is active for the lifetime of
            // the package object into which it is read. 
            hr = pFactory->ReadPackageFromStream(
                pSourceFileStream,
                OPC_CACHE_ON_ACCESS,
                pOutPackage
                );
            if (FAILED(hr))
            {
                NLPRINT_DEBUGVIEWLOGEX(true, L"ReadPackageFromStream Faild %x", hr);
            }
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"CreateStreamOnFile Faild %s %x", lpPackageName, hr);
        }
        // Release resources
        if (pSourceFileStream)
        {
            pSourceFileStream->Release();
            pSourceFileStream = NULL;
        }
        return hr;
    }
    //////////////////////////////////////////////////////////////////////////////
    // Description:
    // Save a package with the specified package file name.
    //
    // Note: Changes made to a package through a package object are not saved until
    // the package is written.
    //////////////////////////////////////////////////////////////////////////////
    HRESULT
        SavePackage(
        IOpcFactory* pFactory,
        IOpcPackage* pPackage,
        IStream* pFileStream
        )
    {
            // Note: Do not use a writable stream to overwrite the data of a package
            // that is read.

            // Create a writable stream over the specified target file name.

            // After a stream over the specified file is created successfully,
            // write package data to the file.
            HRESULT hr = pFactory->WritePackageToStream(
                pPackage,
                OPC_WRITE_DEFAULT,
                pFileStream
                );
            if (FAILED(hr))
            {
                NLPRINT_DEBUGVIEWLOGEX(true, L"WritePackageToStream Faild %x", hr);
            }

            // Release resources
            if (pFileStream)
            {
                pFileStream->Release();
                pFileStream = NULL;
            }

            return hr;
        }
    HRESULT
        SavePackage(
        IOpcFactory *pFactory,
        IOpcPackage *pPackage,
        LPCWSTR lpTargetFileName
        )
    {
            IStream * pTargetFileStream = NULL;
            // Note: Do not use a writable stream to overwrite the data of a package
            // that is read.

            // Create a writable stream over the specified target file name.
            HRESULT hr = pFactory->CreateStreamOnFile(
                lpTargetFileName,
                OPC_STREAM_IO_WRITE,
                NULL,
                0,
                &pTargetFileStream
                );

            if (SUCCEEDED(hr)&&pTargetFileStream!=NULL)
            {
                // After a stream over the specified file is created successfully,
                // write package data to the file.
                hr = pFactory->WritePackageToStream(
                    pPackage,
                    OPC_WRITE_DEFAULT,
                    pTargetFileStream
                    );
                if (FAILED(hr))
                {
                    NLPRINT_DEBUGVIEWLOGEX(true, L"WritePackageToStream Faild %x", hr);
                }
            }
            else
            {
                NLPRINT_DEBUGVIEWLOGEX(true, L"CreateStreamOnFile Faild %x", hr);
            }

            // Release resources
            if (pTargetFileStream)
            {
                pTargetFileStream->Release();
                pTargetFileStream = NULL;
            }
            return hr;
        }
    //////////////////////////////////////////////////////////////////////////////
    // Description:
    // If the target of the relationship is a part, get the relative URI of the
    // target and resolve this URI to the part name using the URI of the 
    // relationship's source as the base URI.
    //////////////////////////////////////////////////////////////////////////////
    HRESULT
        ResolveTargetUriToPart(
        IOpcRelationship* pRelationship,
        IOpcPartUri** pResolvedUri
        )
    {
            IOpcUri * pSourceUri = NULL;
            IUri * pTargetUri = NULL;
            OPC_URI_TARGET_MODE targetMode;

            // Get the target mode of the relationship.
            HRESULT hr = pRelationship->GetTargetMode(&targetMode);
            if (SUCCEEDED(hr) && targetMode != OPC_URI_TARGET_MODE_INTERNAL)
            {
                // The target mode of the relationship was not internal.
                // Function fails because the relationship does not target a part
                // therefore, no part name can be resolved.
                NLPRINT_DEBUGVIEWLOGEX(true, L"The target mode of the relationship was not internal.\n Function fails because the relationship does not target a part\n therefore, no part name can be resolved.");
                hr = E_FAIL;
            }
            // Get the segments of the URI and turn it into a valid part URI.
            // The target should be resolved against the source URI of the 
            // relationship to expand relative URIs into part URIs with absolute 
            // paths.
            if (SUCCEEDED(hr))
            {
                // Get a URI for the relationship's target. This URI might be
                // relative to the source of the relationship.
                hr = pRelationship->GetTargetUri(&pTargetUri);
            }
            if (SUCCEEDED(hr))
            {
                // Get the URI for the relationship's source.
                hr = pRelationship->GetSourceUri(&pSourceUri);
            }
            if (SUCCEEDED(hr)&&pSourceUri!=NULL)
            {
                // Form the API representation of the resultant part name.
                hr = pSourceUri->CombinePartUri(pTargetUri, pResolvedUri);
            }

            // Release resources
            if (pSourceUri)
            {
                pSourceUri->Release();
                pSourceUri = NULL;
            }

            if (pTargetUri)
            {
                pTargetUri->Release();
                pTargetUri = NULL;
            }

            return hr;
        }

    //////////////////////////////////////////////////////////////////////////////
    // Description:
    // Find a part that is the target of a package relationship based on the
    // specified relationship type. If a content type is specified, check the
    // content type of the part to ensure that only a part with the specified
    // is retrieved.
    // 
    // Note: This function finds the first, arbitrary part that is the target of
    // a relationship of the specified type and has the specified content type, if
    // a content type is provided.
    //
    // Example of the relationship markup for a relationship that targets a part:
    //   <Relationship Id="rId1" 
    //      Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" 
    //      Target="word/document.xml" />
    //
    // The "Type" attribute (relationship type) is the definitive way to find a 
    // part of interest in a package. The relationship type is defined by the
    // package designer or in the OPC and is therefore consistent and predictable.
    // 
    // In contrast, the "Id" and "Target" attributes are arbitrary, and not recommended
    // for finding specific parts of interest.
    //
    // Using a relationship type to find a part of interest requires a few steps.
    // 1. Identify the relationship type, as defined by the package designer or the
    //    OPC, of the relationship(s) that has the part of interest as the target.
    // 2. Get relationships of that relationship type from a relationship set.
    //    Note: It may be necessary to check that the number of retrieved 
    //          relationships conforms to an expected number of relationships
    //          specified by the format designer or in the OPC.
    // 3. If the target of a retrieved relationship is a part, resolve the part
    //    name.
    // 4. Get the part.
    // 5. If an expected content type is defined by the package designer or the OPC,
    //    ensure that the part has the correct content type.
    // 6. If the found part has the expected content type, or if no expected
    //    content type is defined, return the part.
    //////////////////////////////////////////////////////////////////////////////
    HRESULT FindPartByRelationshipType(
        IOpcPackage* pPackage,
        LPCWSTR lpRelationshipType, // Relationship type used to find the part.
        LPCWSTR lpContentType, // Expected content type of part (optional).
        IOpcPart** pPart
        )
    {
        *pPart = NULL; // Enable checks against value of *part.

        IOpcRelationshipSet * pPackageRels = NULL;
        IOpcRelationshipEnumerator * pPackageRelsEnum = NULL;
        IOpcPartSet * pPartSet = NULL;
        BOOL hasNext = false;

        HRESULT hr = pPackage->GetPartSet(&pPartSet);
        if (SUCCEEDED(hr))
        {
            // Get package relationships stored in the package's Relationships part.
            hr = pPackage->GetRelationshipSet(&pPackageRels);
        }

        if (SUCCEEDED(hr)&&pPackageRels!=NULL)
        {
            // Get package relationships of the specified relationship type.
            hr = pPackageRels->GetEnumeratorForType(
                lpRelationshipType,
                &pPackageRelsEnum
                );
        }

        // Note: Though not performed by this sample, it may be necessary to check
        // that the number of retrieved relationships conforms to the expected
        // number of relationships specified by the format designer or in the OPC.

        if (SUCCEEDED(hr)&&pPackageRelsEnum!=NULL)
        {
            // Move pointer to first package relationship.
            hr = pPackageRelsEnum->MoveNext(&hasNext);
        }


        // Find the first, arbitrary part that is the target of a relationship
        // of the specified type and has the specified content type, if a content
        // type is provided. Abandon search when an error is encountered, when
        // there are no more relationships in the enumerator, or when a part is
        // found.
        while (SUCCEEDED(hr) && hasNext && *pPart == NULL)
        {
            IOpcPartUri * pPartUri = NULL;
            IOpcRelationship * pCurrentRel = NULL;
            BOOL partExists = FALSE;

            hr = pPackageRelsEnum->GetCurrent(&pCurrentRel);
            if (SUCCEEDED(hr)&&pCurrentRel!=NULL)
            {
                // There was a relationship of the specified type.
                // Try to resolve the part name of the relationship's target.
                hr = ResolveTargetUriToPart(pCurrentRel, &pPartUri);
            }
            if (SUCCEEDED(hr)&&pPartUri!=NULL)
            {
                // Part name resolved. Check that a part with that part name
                // exists in the package.
                hr = pPartSet->PartExists(pPartUri, &partExists);
            }
            if (SUCCEEDED(hr) && partExists)
            {
                // A part with the resolved part name exists in the package, so
                // get a pointer to that part.

                LPWSTR currentContentType = NULL;
                IOpcPart * pCurrentPart = NULL;

                hr = pPartSet->GetPart(pPartUri, &pCurrentPart);
                if (SUCCEEDED(hr) && lpContentType != NULL)
                {
                    // Content type specified.
                    // Get the content type of the part.
                    hr = pCurrentPart->GetContentType(&currentContentType);

                    // Compare the content type of the part with the specified
                    // content type.
                    if (SUCCEEDED(hr) &&
                        0 == wcscmp(lpContentType, currentContentType))
                    {
                        // Part content type matches specified content type.
                        // Part found.
                        *pPart = pCurrentPart;
                        pCurrentPart = NULL;
                    }
                }
                if (SUCCEEDED(hr) && lpContentType == NULL)
                {
                    // Content type not specified.
                    // Part found.
                    *pPart = pCurrentPart;
                    pCurrentPart = NULL;
                }

                // Release resources
                CoTaskMemFree(static_cast<LPVOID>(currentContentType));

                if (pCurrentPart)
                {
                    pCurrentPart->Release();
                    pCurrentPart = NULL;
                }
            }
            // Get the next relationship of the specified type.
            if (SUCCEEDED(hr)&&pPackageRelsEnum!=NULL)
            {
                hr = pPackageRelsEnum->MoveNext(&hasNext);
            }

            // Release resources
            if (pPartUri)
            {
                pPartUri->Release();
                pPartUri = NULL;
            }

            if (pCurrentRel)
            {
                pCurrentRel->Release();
                pCurrentRel = NULL;
            }
        }

        if (SUCCEEDED(hr) && *pPart == NULL)
        {
            // Loop complete without errors and no part found.
            hr = E_FAIL;
        }

        // Release resources
        if (pPackageRels)
        {
            pPackageRels->Release();
            pPackageRels = NULL;
        }

        if (pPackageRelsEnum)
        {
            pPackageRelsEnum->Release();
            pPackageRelsEnum = NULL;
        }

        if (pPartSet)
        {
            pPartSet->Release();
            pPartSet = NULL;
        }

        return hr;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Description:
    // Read part content into a new MSXML DOM document. If selection namespaces
    // are provided, set up DOM document for XPath queries. Changes made to
    // content from the DOM document must be written to the part explicitly.
    //////////////////////////////////////////////////////////////////////////////
    HRESULT DOMFromPart(IOpcPart * pPart, LPCWSTR lpSelectionNamespaces, IXMLDOMDocument2 **pDocument)
    {

        IXMLDOMDocument2 * pPartContentXmlDocument = NULL;
        IStream * pPartContentStream = NULL;

        HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, __uuidof(IXMLDOMDocument2), (LPVOID*)&pPartContentXmlDocument);

        // If selection namespaces were provided, configure the document 
        // for XPath queries.
        if (SUCCEEDED(hr) && lpSelectionNamespaces&&pPartContentXmlDocument!=NULL)
        {
            // Note: This sample uses a custom variant wrapper because
            // ATL variants may throw.
            AutoVariant v;
            hr = v.SetBSTRValue(L"XPath");
            if (SUCCEEDED(hr))
            {
                hr = pPartContentXmlDocument->setProperty(L"SelectionLanguage", v);
                if (SUCCEEDED(hr))
                {
                    AutoVariant v;
                    hr = v.SetBSTRValue(lpSelectionNamespaces);
                    if (SUCCEEDED(hr))
                    {
                        hr = pPartContentXmlDocument->setProperty(L"SelectionNamespaces", v);
                    }
                    else
                    {
                        NLPRINT_DEBUGVIEWLOGEX(true, L"SetBSTRValue Faild %x", hr);
                    }
                }
            }
            else
            {
                NLPRINT_DEBUGVIEWLOGEX(true, L"SetBSTRValue Faild %x", hr);
            }
        }

        // Content stream from a part is read-write, though the DOM tree will only
        // read from it.
        if (SUCCEEDED(hr))
        {
            // Note: The DOM may hold the content stream open for the life of the
            // DOM tree, and the content stream may hold a reference to the package.
            hr = pPart->GetContentStream(&pPartContentStream);
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"GetContentStream Faild %x", hr);
        }

        if (SUCCEEDED(hr)&&pPartContentStream!=NULL)
        {
            VARIANT_BOOL isSuccessful = VARIANT_FALSE;
            AutoVariant vStream;

            vStream.SetObjectValue(pPartContentStream);
            hr = pPartContentXmlDocument->load(vStream, &isSuccessful);
            if (SUCCEEDED(hr) && isSuccessful == VARIANT_FALSE)
            {
                // DOM load failed. This check intentionally simple.
                hr = E_FAIL;
                // For more about DOM load failure, see the MSXML documentation.
            }
        }

        if (SUCCEEDED(hr))
        {
            // DOM loaded from part content
            *pDocument = pPartContentXmlDocument;
            pPartContentXmlDocument = NULL;
        }

        // Release resources
        if (pPartContentXmlDocument)
        {
            pPartContentXmlDocument->Release();
            pPartContentXmlDocument = NULL;
        }

        if (pPartContentStream)
        {
            pPartContentStream->Release();
            pPartContentStream = NULL;
        }

        return hr;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Description:
    // Helper function that finds the Core Properties part of a package.
    //////////////////////////////////////////////////////////////////////////////
    HRESULT FindCorePropertiesPart(IOpcPackage *pPackage, IOpcPart **pPart)
    {
        return FindPartByRelationshipType(pPackage, g_corePropertiesRelationshipType, g_corePropertiesContentType, pPart);
    }

    HRESULT FindExtendedPropertiesPart(IOpcPackage *pPackage, IOpcPart **pPart)
    {
        return FindPartByRelationshipType(pPackage, g_extendedPropertiesRelationshipType, g_extendedPropertiesContentType, pPart);
    }

    HRESULT FindCustomPropertiesPart(IOpcPackage *pPackage, IOpcPart **pPart)
    {

        return FindPartByRelationshipType(
            pPackage,
            g_customPropertiesRelationshipType,
            g_customPropertiesContentType,
            pPart
            );
    }

    //////////////////////////////////////////////////////////////////////////////
    // Description:
    // Write core properties information to the console.
    //////////////////////////////////////////////////////////////////////////////
    HRESULT GetCoreProperties(IOpcPackage *pPackage, std::vector<struTag_Pair>& verProperties)
    {
        ATL::CComPtr<IOpcPart>    pCorePropertiesPart;

        HRESULT hr = FindCorePropertiesPart(pPackage, &pCorePropertiesPart);
        if (SUCCEEDED(hr))
        {
            ATL::CComPtr<IXMLDOMDocument2>    pCorePropertiesDom;
            hr = DOMFromPart(pCorePropertiesPart, g_corePropertiesSelectionNamespaces, &pCorePropertiesDom);
            if (SUCCEEDED(hr))
            {
                ATL::CComPtr<IXMLDOMNode> creatorNode;
                BSTR text = NULL;
                struTag_Pair tagPair;
                hr = pCorePropertiesDom->selectSingleNode(L"//dc:creator", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Author";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }


                hr = pCorePropertiesDom->selectSingleNode(L"//dc:title", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Title";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }

                hr = pCorePropertiesDom->selectSingleNode(L"//dc:subject", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Subject";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }

                hr = pCorePropertiesDom->selectSingleNode(L"//cp:keywords", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Keywords";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }

                hr = pCorePropertiesDom->selectSingleNode(L"//dc:description", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Comments";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }

                hr = pCorePropertiesDom->selectSingleNode(L"//cp:category", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Category";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }
            }
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"FindCorePropertiesPart Faild %x", hr);
        }

        return hr;
    }

    HRESULT GetExtendedProperties(IOpcPackage *pPackage, std::vector<struTag_Pair>& verProperties)
    {
        ATL::CComPtr<IOpcPart>    pExtendedPropertiesPart;

        HRESULT hr = FindExtendedPropertiesPart(pPackage, &pExtendedPropertiesPart);

        if (SUCCEEDED(hr))
        {
            ATL::CComPtr<IXMLDOMDocument2>    pExtendedPropertiesDom;
            hr = DOMFromPart(pExtendedPropertiesPart, g_extendedPropertiesSelectionNamespaces, &pExtendedPropertiesDom);

            if (SUCCEEDED(hr))
            {
                ATL::CComPtr<IXMLDOMNode> creatorNode;
                BSTR text = NULL;
                struTag_Pair tagPair;
                hr = pExtendedPropertiesDom->selectSingleNode(L"//extend:Template", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {

                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Template";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }


                hr = pExtendedPropertiesDom->selectSingleNode(L"//extend:Manager", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Manager";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }

                hr = pExtendedPropertiesDom->selectSingleNode(L"//extend:Company", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"Company";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }

                hr = pExtendedPropertiesDom->selectSingleNode(L"//extend:HyperlinkBase", &creatorNode);
                if (SUCCEEDED(hr) && creatorNode != NULL)
                {
                    hr = creatorNode->get_text(&text);
                    if (SUCCEEDED(hr) && text != NULL)
                    {
                        tagPair.strTagName = L"HyperlinkBase";
                        tagPair.strTagValue = text;
                        tagPair.bSuccess = true;
                        verProperties.push_back(tagPair);
                        ::SysFreeString(text);
                    }
                    creatorNode = NULL;
                }
            }
            else
            {
                NLPRINT_DEBUGVIEWLOGEX(true, L"DOMFromPart Faild %x", hr);
            }
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"FindExtendedPropertiesPart Faild %x", hr);
        }
        return hr;
    }

    HRESULT InitCustomPropPart(IOpcPart** pCustomPropertiesPart)
    {
        CComPtr<IStream> pStreamCustomProp = NULL;
        HRESULT hr = (*pCustomPropertiesPart)->GetContentStream(&pStreamCustomProp);
        if (SUCCEEDED(hr) && pStreamCustomProp != NULL)
        {
            const char* pInitData = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" "
                "xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\"></Properties>";
            ULONG lWriteCount = 0;
            hr = pStreamCustomProp->Write(pInitData, strlen(pInitData), &lWriteCount);
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"GetContentStream Faild %x \n", hr);
        }
        return hr;
    }
    HRESULT SetCustomProperties(IOpcPart *pCustomPropertiesPart, IXMLDOMDocument2* pCustomPropertiesDom, std::vector<struTag_Pair>& vectPropers)
    {
        CComPtr<IXMLDOMNode> pNodeProperties;
        HRESULT hr = pCustomPropertiesDom->selectSingleNode(L"//customprop:Properties", &pNodeProperties);
        if (SUCCEEDED(hr)&&pNodeProperties!=NULL)
        {
            long nNextPidValue = 2;
            std::vector<struTag_Pair>::iterator itProp = vectPropers.begin();
            wchar_t szXPath[4096] = { 0 };
            std::wstring strText = L"";
            std::wstring strOldText = L"";
            while (itProp != vectPropers.end())
            {
                CComPtr<IXMLDOMNode> nodePropExist;
                wsprintfW(szXPath, L"customprop:property[@name='%s']", itProp->strTagName.c_str());
                hr = pNodeProperties->selectSingleNode(szXPath, &nodePropExist);
                if (SUCCEEDED(hr) && nodePropExist != NULL)
                {
                    //modify property value
                    CComPtr<IXMLDOMNode> valueNode = NULL;
                    CComBSTR temp = NULL;
                    hr = nodePropExist->get_xml(&temp);
                    hr = nodePropExist->get_firstChild(&valueNode);
                    if (valueNode)
                    {
                        CComBSTR bstrText(itProp->strTagValue.c_str());
                        CComBSTR bstrOldText = L"";
                        hr = valueNode->get_text(&bstrOldText);
                        if (SUCCEEDED(hr) && bstrOldText != NULL)
                        {
                            strText = bstrText;
                            strOldText = bstrOldText;
                            bstrText = (strOldText + L"|" + strText).c_str();
                        }
                        valueNode->put_text(bstrText);
                    }
                }
                else
                {
                    NLPRINT_DEBUGVIEWLOG(L"selectSingleNode Faild %s %x", szXPath, hr);
                    //create new property
                    CComPtr<IXMLDOMNode> pNewNode;
                    hr = pCustomPropertiesDom->createNode(CComVariant((short)NODE_ELEMENT), L"property", (L"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties"), &pNewNode);
                    if (SUCCEEDED(hr)&&pNewNode!=NULL)
                    {
                        CComPtr<IXMLDOMElement> pNewPropNode;
                        pNewNode->QueryInterface(IID_IXMLDOMElement, (void**)&pNewPropNode);
                        pNewPropNode->setAttribute(L"fmtid", CComVariant(L"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}"));
                        pNewPropNode->setAttribute(L"pid", CComVariant(nNextPidValue++));
                        pNewPropNode->setAttribute(L"name", CComVariant(itProp->strTagName.c_str()));

                        CComPtr<IXMLDOMElement> pValueElement;
                        pCustomPropertiesDom->createElement(L"vt:lpwstr", &pValueElement);
                        pValueElement->put_nodeTypedValue(CComVariant(itProp->strTagValue.c_str()));

                        // Add the Tag Value Node
                        CComPtr<IXMLDOMNode> pNewChildNode = NULL;
                        pNewPropNode->appendChild(pValueElement, &pNewChildNode);
                        pNewChildNode.Release();

                        pNodeProperties->appendChild(pNewPropNode, &pNewChildNode);
                    }

                }

                itProp++;
            }
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"selectSingleNode faild %x", hr);
        }
        //save	
        CComPtr<IStream> streamCustomProp = NULL;
        hr = pCustomPropertiesPart->GetContentStream(&streamCustomProp);
        if (SUCCEEDED(hr) && streamCustomProp)
        {
            ULARGE_INTEGER zero = { 0 };
            hr = streamCustomProp->SetSize(zero);

            AutoVariant vStream;
            vStream.SetObjectValue(streamCustomProp);
            hr = pCustomPropertiesDom->save(vStream);
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"GetContentStream Faild %x", hr);
        }


        return hr;
    }


    HRESULT GetCustomProperties(IOpcPackage *pPackage, std::vector<struTag_Pair>& vectProperties)
    {
        NLONLY_DEBUG;
        CComPtr<IOpcPart>  pCustomPropertiesPart;

        HRESULT hr = FindCustomPropertiesPart(pPackage, &pCustomPropertiesPart);
        if (SUCCEEDED(hr)&&pCustomPropertiesPart!=NULL)
        {
            CComPtr<IXMLDOMDocument2> pCustomPropertiesDom;
            hr = DOMFromPart(pCustomPropertiesPart, g_customPropertiesSelectionNamespaces, &pCustomPropertiesDom);
            if (SUCCEEDED(hr))
            {
                hr = GetCustomPropertiesFromDom(pCustomPropertiesDom, vectProperties);
            }
            else
            {
                NLPRINT_DEBUGVIEWLOGEX(true, L"GetCustomPropertiesFromDom Faild %x", hr);
            }
        }
        else
        {
            NLPRINT_DEBUGVIEWLOGEX(true, L"FindCustomPropertiesPart Faild %x", hr);
        }



        return hr;
    }
    void MergeProperties(const std::vector<struTag_Pair> &vecSource, std::vector<struTag_Pair> &vecDest, bool bOverWrite, std::vector<struTag_Pair> &vecResult)
    {
        for (size_t i = 0; i < vecDest.size(); i++)
        {
            if (!PropertiesContain(vecResult, vecDest[i]))
            {
                vecResult.push_back(vecDest[i]);
                vecDest[i].bSuccess = true;
            }
        }

        if (!bOverWrite)
        {
            for (size_t i = 0; i < vecSource.size(); i++)
            {
                if (!PropertiesContain(vecResult, vecSource[i]))
                {
                    vecResult.push_back(vecSource[i]);
                }
            }

        }
        else
        {
            for (size_t i = 0; i < vecSource.size(); i++)
            {
                bool bExits = false;
                for (size_t j = 0; j < vecDest.size(); j++)
                {
                    if (_wcsicmp(vecSource[i].strTagName.c_str(), vecDest[j].strTagName.c_str()) == 0)
                    {
                        bExits = true;
                        break;
                    }
                }
                if (!bExits)
                {
                    if (!PropertiesContain(vecResult, vecSource[i]))
                    {
                        vecResult.push_back(vecSource[i]);
                    }
                }
            }
        }


    }

    void RemoveProperties(const std::vector<struTag_Pair> &vecSource, std::vector<struTag_Pair> &vecDest, std::vector<struTag_Pair> &vecResult)
    {
        bool bExits = false;
        for (size_t i = 0; i < vecSource.size(); i++)
        {
           bExits = false;
            for (size_t j = 0; j < vecDest.size(); j++)
            {
                if (_wcsicmp(vecSource[i].strTagName.c_str(), vecDest[j].strTagName.c_str()) == 0)
                {
                    bExits = true;
                    vecDest[j].bSuccess = true;
                    break;
                }
            }
            if (!bExits)
            {
                if (!PropertiesContain(vecResult, vecSource[i]))
                {
                    vecResult.push_back(vecSource[i]);

                }
            }
        }
    }


    bool PropertiesContain(const std::vector<struTag_Pair> &vectProperties, struTag_Pair tag_Pair)
    {
        bool bresult = false;
        for (size_t i = 0; i < vectProperties.size(); i++)
        {
            if (_wcsicmp(vectProperties[i].strTagName.c_str(), tag_Pair.strTagName.c_str()) == 0)
            {
                if (_wcsicmp(vectProperties[i].strTagValue.c_str(), tag_Pair.strTagValue.c_str()) == 0)
                {
                    bresult = true;
                    break;
                }
            }
        }
        return bresult;
    }
    HRESULT CreateCustomPropertiesPart(IOpcFactory *pOpcFactory, IOpcPackage *pPackage, IOpcPart** pCustomPropertiesPart)
    {
        NLONLY_DEBUG;
        CComPtr<IOpcPartUri> pOpcCurtomPropPartUri = NULL;
        HRESULT hr = pOpcFactory->CreatePartUri(g_CustomPropertiesPartUrl, &pOpcCurtomPropPartUri);
        if (SUCCEEDED(hr) && pOpcCurtomPropPartUri != NULL)
        {
            NLPRINT_DEBUGVIEWLOG(L"CreatePartUri Success");
            CComPtr<IOpcPartSet> pPartSet = NULL;
            hr = pPackage->GetPartSet(&pPartSet);
            if (SUCCEEDED(hr) && pPartSet != NULL)
            {
                NLPRINT_DEBUGVIEWLOG(L"GetPartSet Success");
                hr = pPartSet->CreatePart(pOpcCurtomPropPartUri, g_customPropertiesContentType, OPC_COMPRESSION_NORMAL, pCustomPropertiesPart);
                if (SUCCEEDED(hr) && pCustomPropertiesPart != NULL)
                {
                    NLPRINT_DEBUGVIEWLOGEX(true, L"CreatePart Success");
                    hr = InitCustomPropPart(pCustomPropertiesPart);
                    if (SUCCEEDED(hr))
                    {
                        NLPRINT_DEBUGVIEWLOGEX(true, L"InitCustomPropPart Success");
                        CComPtr<IOpcRelationshipSet> pRelSet = NULL;
                        hr = pPackage->GetRelationshipSet(&pRelSet);
                        if (SUCCEEDED(hr) && pRelSet != NULL)
                        {
                            NLPRINT_DEBUGVIEWLOGEX(true, L"GetRelationshipSet Success");
                            CComPtr<IOpcRelationship> pRel = NULL;

                            int iRealshipCount = 1;
                            wchar_t strErr[32] = { 0 };
                            while (true)
                            {
                                BOOL bRealshipExists = FALSE;
                                wsprintfW(strErr, L"rId%d", iRealshipCount);
                                NLPRINT_DEBUGVIEWLOGEX(true, strErr);
                                hr = pRelSet->RelationshipExists(strErr, &bRealshipExists);
                                OutputDebugString(strErr);
                                if (bRealshipExists)
                                {
                                    NLPRINT_DEBUGVIEWLOGEX(true, L"%s is exites", strErr);
                                    iRealshipCount++;
                                }
                                else
                                {
                                    NLPRINT_DEBUGVIEWLOGEX(true, L"%s is not exites", strErr);
                                    hr = pRelSet->CreateRelationship(strErr,
                                        g_customPropertiesRelationshipType,
                                        pOpcCurtomPropPartUri, OPC_URI_TARGET_MODE_INTERNAL, &pRel);
                                    if (SUCCEEDED(hr))
                                    {
                                        NLPRINT_DEBUGVIEWLOG(L"CreateRelationship Success");
                                    }
                                    else
                                    {
                                        NLPRINT_DEBUGVIEWLOG(L"CreateRelationship faild");
                                    }
                                    break;
                                }
                            }

                        }
                        else
                        {
                            NLPRINT_DEBUGVIEWLOGEX(true, L"GetRelationshipSet Faild %x \n", hr);
                        }
                    }
                    else
                    {
                        NLPRINT_DEBUGVIEWLOGEX(true, L"InitCustomPropPart Faild %x \n", hr);
                    }
                }
                else
                {
                    NLPRINT_DEBUGVIEWLOGEX(true, L"CreatePart Faild. %x \n", hr);
                }
            }
            else
            {
                NLPRINT_DEBUGVIEWLOGEX(true, L"GetPartSet Faild or GetPartSet emply PartSet. %x \n", hr);
            }

        }
        return hr;
    }
    HRESULT GetCustomPropertiesFromDom(IXMLDOMDocument2* pCustomPropertiesDom, std::vector<struTag_Pair>& vectProperties)
    {
		NLONLY_DEBUG;
        bool bGetSingleTag = false;
        CComPtr<IXMLDOMNodeList> pCustomProperNodeList;
        HRESULT hr = pCustomPropertiesDom->selectNodes(L"//customprop:property", &pCustomProperNodeList);


        if (SUCCEEDED(hr) && pCustomProperNodeList != NULL)
        {
            if (vectProperties.size() > 0)
            {
                bGetSingleTag = true;
                vectProperties[0].bSuccess = false;
            }
            CComPtr<IXMLDOMNode> pCustomProper = NULL;
            while (SUCCEEDED(pCustomProperNodeList->nextNode(&pCustomProper)) && pCustomProper)
            {
                CComPtr<IXMLDOMNamedNodeMap> pNamedNodeMap = NULL;
                hr = pCustomProper->get_attributes(&pNamedNodeMap);

                CComPtr<IXMLDOMNode> pNameNode = NULL;
                pNamedNodeMap->getNamedItem(L"name", &pNameNode);
                if (pNameNode)
                {
                    VARIANT varName;
                    VariantInit(&varName);
                    pNameNode->get_nodeValue(&varName);

                    BSTR bstrValue = NULL;
                    CComPtr<IXMLDOMNode> pValueNode = NULL;
                    hr = pCustomProper->selectSingleNode(L"./vt:lpwstr", &pValueNode);
                    if (SUCCEEDED(hr)&&pValueNode!=NULL)
                    {
                        pValueNode->get_text(&bstrValue);

                    }
                    else
                    {
                        hr = pCustomProper->selectSingleNode(L"./vt:i4", &pValueNode);
                        if (SUCCEEDED(hr)&&pValueNode!=NULL)
                        {
                            pValueNode->get_text(&bstrValue);

                        }
                        else
                        {
                            hr = pCustomProper->selectSingleNode(L"./vt:bool", &pValueNode);
                            if (SUCCEEDED(hr)&&pValueNode!=NULL)
                            {
                                pValueNode->get_text(&bstrValue);

                            }
                            else
                            {
                                hr = pCustomProper->selectSingleNode(L"./vt:filetime", &pValueNode);
                                if (SUCCEEDED(hr)&&pValueNode!=NULL)
                                {
                                    pValueNode->get_text(&bstrValue);

                                }
                            }
                        }
                    }
                    if (varName.bstrVal && bstrValue)
                    {
						struTag_Pair tagPair;
						tagPair.strTagName = varName.bstrVal;
						tagPair.strTagValue = bstrValue;
						tagPair.bSuccess = true;
						vectProperties.push_back(tagPair);

                    }
                    if (bstrValue != NULL)
                    {
                        ::SysFreeString(bstrValue);
                        bstrValue = NULL;
                    }
                    VariantClear(&varName);
                }
                if (bGetSingleTag&&vectProperties[0].bSuccess)
                {
                    break;
                }
                pCustomProper.Release();
            }

        }
        return hr;
    }
    HRESULT AddCustomProperties(IOpcPackage *pPackage, IOpcPart *pCustomPropertiesPart, std::vector<struTag_Pair>& verProperties, bool bOverWrite)
    {
        CComPtr<IXMLDOMDocument2> pCustomPropertiesDom;

        HRESULT hr = DOMFromPart(pCustomPropertiesPart, g_customPropertiesSelectionNamespaces, &pCustomPropertiesDom);
        if (SUCCEEDED(hr)&&pCustomPropertiesDom!=NULL)
        {
            std::vector<struTag_Pair> sourceProperties;
            std::vector<struTag_Pair> resultProperties;
            hr = GetCustomPropertiesFromDom(pCustomPropertiesDom, sourceProperties);
            if (SUCCEEDED(hr))
            {
                if (sourceProperties.size() != 0)
                {
                    MergeProperties(sourceProperties, verProperties, bOverWrite, resultProperties);
                }
                else
                {
                    resultProperties = verProperties;
                }
                hr = DeleteExitesCustomProperties(pCustomPropertiesDom);
                hr = SetCustomProperties(pCustomPropertiesPart, pCustomPropertiesDom, resultProperties);

            }
        }
        return hr;

    }

    HRESULT RemoveCustomProperties(IOpcPackage *pPackage, IOpcPart *pCustomPropertiesPart, std::vector<struTag_Pair>& verProperties)
    {
        CComPtr<IXMLDOMDocument2> pCustomPropertiesDom;

        HRESULT hr = DOMFromPart(pCustomPropertiesPart, g_customPropertiesSelectionNamespaces, &pCustomPropertiesDom);
        if (SUCCEEDED(hr)&&pCustomPropertiesDom!=NULL)
        {
            std::vector<struTag_Pair> sourceProperties;
            std::vector<struTag_Pair> resultProperties;
            hr = GetCustomPropertiesFromDom(pCustomPropertiesDom, sourceProperties);
            if (SUCCEEDED(hr))
            {
                if (sourceProperties.size() != 0)
                {
                    if (verProperties.size() != 0)
                    {
                        RemoveProperties(sourceProperties, verProperties, resultProperties);
                    }
                    hr = DeleteExitesCustomProperties(pCustomPropertiesDom);
                    hr = SetCustomProperties(pCustomPropertiesPart, pCustomPropertiesDom, resultProperties);
                }
            }
        }
        return hr;

    }


    HRESULT DeleteExitesCustomProperties(IXMLDOMDocument2* pCustomPropertiesDom)
    {
        CComPtr<IXMLDOMNode> nodeProperties;
        CComPtr<IXMLDOMNodeList> customProperNodeList;
        HRESULT hr = pCustomPropertiesDom->selectNodes(L"//customprop:property", &customProperNodeList);
        if (SUCCEEDED(hr) && customProperNodeList != NULL)
        {
            long lNodeListLength = 0;
            hr = customProperNodeList->get_length(&lNodeListLength);
            if (SUCCEEDED(hr))
            {
                hr = pCustomPropertiesDom->selectSingleNode(L"//customprop:Properties", &nodeProperties);
                if (SUCCEEDED(hr) && nodeProperties != NULL)
                {
                    for (int i = 0; i < lNodeListLength; i++)
                    {
                        CComPtr<IXMLDOMNode> pDeleteNode;
                        customProperNodeList->get_item(i, &pDeleteNode);
                        if (SUCCEEDED(hr) && pDeleteNode != NULL)
                        {
                            CComPtr<IXMLDOMNode> pDeleteResult;
                            hr = nodeProperties->removeChild(pDeleteNode, &pDeleteResult);

                        }
                    }
                }
            }
        }
        return hr;
    }


    std::vector<std::wstring> split(const std::wstring &ss, wchar_t sep)
    {
        std::vector<std::wstring> elems;
        size_t start = 0;
        size_t end;
        while ((start < ss.length()) && (end = ss.find(sep, start)) != ss.npos)
        {
            elems.push_back(ss.substr(start, end - start));
            start = end + 1;
        }
        if (start < ss.length())
        {
            elems.push_back(ss.substr(start));
        }
        return elems;
    }
} // namespace opclib