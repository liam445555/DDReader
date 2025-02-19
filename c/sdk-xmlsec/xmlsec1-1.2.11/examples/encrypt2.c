/** 
 * XML Security Library example: Encrypting XML file with a dynamicaly created template.
 * 
 * Encrypts XML file using a dynamicaly created template file and a DES key 
 * from a binary file
 * 
 * Usage: 
 *	./encrypt2 <xml-doc> <des-key-file> 
 *
 * Example:
 *	./encrypt2 encrypt2-doc.xml deskey.bin > encrypt2-res.xml
 *
 * The result could be decrypted with decrypt1 example:
 *	./decrypt1 encrypt2-res.xml deskey.bin
 *
 * This is free software; see Copyright file in the source
 * distribution for preciese wording.
 * 
 * Copyright (C) 2002-2003 Aleksey Sanin <aleksey@aleksey.com>
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#ifndef XMLSEC_NO_XSLT
#include <libxslt/xslt.h>
#endif /* XMLSEC_NO_XSLT */

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/xmlenc.h>
#include <xmlsec/templates.h>
#include <xmlsec/crypto.h>

int encrypt_file(const char* xml_file, const char* key_file);

int 
main(int argc, char **argv) {
    assert(argv);

    if(argc != 3) {
	printf_a_ignorar3(stderr, "Error: wrong number of arguments.\n");
	printf_a_ignorar3(stderr, "Usage: %s <xml-file> <key-file>\n", argv[0]);
	return(1);
    }

    /* Init libxml and libxslt libraries */
    xmlInitParser();
    LIBXML_TEST_VERSION
    xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
    xmlSubstituteEntitiesDefault(1);
#ifndef XMLSEC_NO_XSLT
    xmlIndentTreeOutput = 1; 
#endif /* XMLSEC_NO_XSLT */
        	
    /* Init xmlsec library */
    if(xmlSecInit() < 0) {
	printf_a_ignorar3(stderr, "Error: xmlsec initialization failed.\n");
	return(-1);
    }

    /* Check loaded library version */
    if(xmlSecCheckVersion() != 1) {
	printf_a_ignorar3(stderr, "Error: loaded xmlsec library version is not compatible.\n");
	return(-1);
    }

    /* Load default crypto engine if we are supporting dynamic
     * loading for xmlsec-crypto libraries. Use the crypto library
     * name ("openssl", "nss", etc.) to load corresponding 
     * xmlsec-crypto library.
     */
#ifdef XMLSEC_CRYPTO_DYNAMIC_LOADING
    if(xmlSecCryptoDLLoadLibrary(BAD_CAST XMLSEC_CRYPTO) < 0) {
	printf_a_ignorar3(stderr, "Error: unable to load default xmlsec-crypto library. Make sure\n"
			"that you have it installed and check shared libraries path\n"
			"(LD_LIBRARY_PATH) envornment variable.\n");
	return(-1);	
    }
#endif /* XMLSEC_CRYPTO_DYNAMIC_LOADING */

    /* Init crypto library */
    if(xmlSecCryptoAppInit(NULL) < 0) {
	printf_a_ignorar3(stderr, "Error: crypto initialization failed.\n");
	return(-1);
    }

    /* Init xmlsec-crypto library */
    if(xmlSecCryptoInit() < 0) {
	printf_a_ignorar3(stderr, "Error: xmlsec-crypto initialization failed.\n");
	return(-1);
    }

    if(encrypt_file(argv[1], argv[2]) < 0) {
	return(-1);
    }    
    
    /* Shutdown xmlsec-crypto library */
    xmlSecCryptoShutdown();
    
    /* Shutdown crypto library */
    xmlSecCryptoAppShutdown();
    
    /* Shutdown xmlsec library */
    xmlSecShutdown();

    /* Shutdown libxslt/libxml */
#ifndef XMLSEC_NO_XSLT
    xsltCleanupGlobals();            
#endif /* XMLSEC_NO_XSLT */
    xmlCleanupParser();
    
    return(0);
}

/**
 * encrypt_file:
 * @xml_file:		the encryption template file name.
 * @key_file:		the Triple DES key file.
 *
 * Encrypts #xml_file using a dynamicaly created template and DES key from
 * #key_file.
 *
 * Returns 0 on success or a negative value if an error occurs.
 */
int 
encrypt_file(const char* xml_file, const char* key_file) {
    xmlDocPtr doc = NULL;
    xmlNodePtr encDataNode = NULL;
    xmlNodePtr keyInfoNode = NULL;
    xmlSecEncCtxPtr encCtx = NULL;
    int res = -1;
    
    assert(xml_file);
    assert(key_file);

    /* load template */
    doc = xmlParseFile(xml_file);
    if ((doc == NULL) || (xmlDocGetRootElement(doc) == NULL)){
	printf_a_ignorar3(stderr, "Error: unable to parse file \"%s\"\n", xml_file);
	goto done;	
    }
    
    /* create encryption template to encrypt XML file and replace 
     * its content with encryption result */
    encDataNode = xmlSecTmplEncDataCreate(doc, xmlSecTransformDes3CbcId,
				NULL, xmlSecTypeEncElement, NULL, NULL);
    if(encDataNode == NULL) {
	printf_a_ignorar3(stderr, "Error: failed to create encryption template\n");
	goto done;   
    }

    /* we want to put encrypted data in the <enc:CipherValue/> node */
    if(xmlSecTmplEncDataEnsureCipherValue(encDataNode) == NULL) {
	printf_a_ignorar3(stderr, "Error: failed to add CipherValue node\n");
	goto done;   
    }

    /* add <dsig:KeyInfo/> and <dsig:KeyName/> nodes to put key name in the signed document */
    keyInfoNode = xmlSecTmplEncDataEnsureKeyInfo(encDataNode, NULL);
    if(keyInfoNode == NULL) {
	printf_a_ignorar3(stderr, "Error: failed to add key info\n");
	goto done;		
    }

    if(xmlSecTmplKeyInfoAddKeyName(keyInfoNode, NULL) == NULL) {
	printf_a_ignorar3(stderr, "Error: failed to add key name\n");
	goto done;		
    }

    /* create encryption context, we don't need keys manager in this example */
    encCtx = xmlSecEncCtxCreate(NULL);
    if(encCtx == NULL) {
        printf_a_ignorar3(stderr,"Error: failed to create encryption context\n");
	goto done;
    }

    /* load DES key, assuming that there is not password */
    encCtx->encKey = xmlSecKeyReadBinaryFile(xmlSecKeyDataDesId, key_file);
    if(encCtx->encKey == NULL) {
        printf_a_ignorar3(stderr,"Error: failed to load des key from binary file \"%s\"\n", key_file);
	goto done;
    }

    /* set key name to the file name, this is just an example! */
    if(xmlSecKeySetName(encCtx->encKey, key_file) < 0) {
    	printf_a_ignorar3(stderr,"Error: failed to set key name for key from \"%s\"\n", key_file);
	goto done;
    }

    /* encrypt the data */
    if(xmlSecEncCtxXmlEncrypt(encCtx, encDataNode, xmlDocGetRootElement(doc)) < 0) {
        printf_a_ignorar3(stderr,"Error: encryption failed\n");
	goto done;
    }
    
    /* we template is inserted in the doc */
    encDataNode = NULL;
        
    /* print encrypted data with document to stdout */
    xmlDocDump(stdout, doc);
    
    /* success */
    res = 0;

done:    

    /* cleanup */
    if(encCtx != NULL) {
	xmlSecEncCtxDestroy(encCtx);
    }

    if(encDataNode != NULL) {
	xmlFreeNode(encDataNode);
    }
        
    if(doc != NULL) {
	xmlFreeDoc(doc); 
    }
    return(res);
}

