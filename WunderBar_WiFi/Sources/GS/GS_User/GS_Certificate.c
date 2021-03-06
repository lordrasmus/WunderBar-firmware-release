/** @file   GS_Certificate.c
 *  @brief  File contains functions to handle SSL certificate with GS module
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */


#include <string.h>
#include <stdint.h>

#include "../API/GS_API.h"
#include <hardware/Hw_modules.h>
#include "../../FTFE/flash_FTFE.h" /* include flash driver header file */


#define CA_CERT "cacert"



	///////////////////////////////////////
	/*         public functions          */
	///////////////////////////////////////



/**
*  @brief  Open SSL connection on desired cid
*
*  Open SSl connection on desired tcp socket with given connection id.
*  Use predefined certificate (loaded with GS_Cert_LoadCert)
*
*  @param  Connection id
*
*  @return True if successful
*/
bool GS_Cert_OpenSLLconn(char cid){
	return GS_API_OpenSSLconnection((uint8_t) cid , (uint8_t *) CA_CERT);
}

/**
*  @brief  Load Cert from Flash into GS module
*
*  Load Certificate from predefined address in flash into GS module.
*  Before loading new certificate delete if any with same name.
*
*  @return True if successful
*/
bool GS_Cert_LoadExistingCert(){
	int size;
	const int* ptr = (void *) FLASH_CERTIFICATE_IMAGE_ADDRESS;

	size = *ptr;
	if (size == 0xFFFFFFFF)			// if there is no cert in flash
		return false;

	GS_API_RemoveCertificate((uint8_t *) CA_CERT);
	return (GS_API_LoadCertificate((uint8_t *) CA_CERT, size, (uint8_t *) FLASH_CERTIFICATE_IMAGE_ADDRESS+4));
}

/**
*  @brief  Save valid certificate into flash memory
*
*  Save certificate on predefined address in flash.
*  First four bytes is certificate length.
*
*  @param  Pointer to a buffer with size (uint32_t) and certificate
*
*  @return void
*/
void GS_Cert_StoreInFlash(char* buff){
	uint32_t* size;

	size = (uint32_t *) &buff[0];              // get certificate size

	Cpu_DisableInt();
	Flash_SectorErase(FLASH_CERTIFICATE_IMAGE_ADDRESS);
	Flash_ByteProgram(FLASH_CERTIFICATE_IMAGE_ADDRESS, (uint32_t *) buff, (*size) + sizeof(uint32_t));
	Cpu_EnableInt();
}



	///////////////////////////////////////
	/*         static functions          */
	///////////////////////////////////////


/**
*  @brief  Load desired certificate
*
*  Load given certificate into RAM of the GS module.
*
*  @param  Certificate size
*  @param  Certificate
*
*  @return True if successful
*/
/*static bool GS_Cert_LoadCert(int size, char* cacert){
	// Load CA Certificate
	return (GS_API_LoadCertificate((uint8_t *) CA_CERT, size, (uint8_t *) cacert));
}*/


/*
*  CACERT file in der format.
*  needed for server authentication
*/
const char cacert[1213] = {
	0xB9, 0x04, 0x00, 0x00,
	0x30, 0x82, 0x04, 0xB5, 0x30, 0x82, 0x03, 0x9D, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x10, 0x4D,
	0xF2, 0x4E, 0x95, 0xA1, 0x5B, 0xA5, 0x53, 0x49, 0x9A, 0x9A, 0x89, 0x6F, 0x02, 0xBE, 0x75, 0x30,
	0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x6F,
	0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x53, 0x45, 0x31, 0x14, 0x30,
	0x12, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x13, 0x0B, 0x41, 0x64, 0x64, 0x54, 0x72, 0x75, 0x73, 0x74,
	0x20, 0x41, 0x42, 0x31, 0x26, 0x30, 0x24, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x13, 0x1D, 0x41, 0x64,
	0x64, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x45, 0x78, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C, 0x20,
	0x54, 0x54, 0x50, 0x20, 0x4E, 0x65, 0x74, 0x77, 0x6F, 0x72, 0x6B, 0x31, 0x22, 0x30, 0x20, 0x06,
	0x03, 0x55, 0x04, 0x03, 0x13, 0x19, 0x41, 0x64, 0x64, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x45,
	0x78, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C, 0x20, 0x43, 0x41, 0x20, 0x52, 0x6F, 0x6F, 0x74, 0x30,
	0x1E, 0x17, 0x0D, 0x31, 0x31, 0x31, 0x32, 0x30, 0x32, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5A,
	0x17, 0x0D, 0x32, 0x30, 0x30, 0x35, 0x33, 0x30, 0x31, 0x30, 0x34, 0x38, 0x33, 0x38, 0x5A, 0x30,
	0x3D, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x10,
	0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x13, 0x07, 0x53, 0x53, 0x4C, 0x2E, 0x63, 0x6F, 0x6D,
	0x31, 0x1C, 0x30, 0x1A, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x13, 0x53, 0x53, 0x4C, 0x2E, 0x63,
	0x6F, 0x6D, 0x20, 0x46, 0x72, 0x65, 0x65, 0x20, 0x53, 0x53, 0x4C, 0x20, 0x43, 0x41, 0x30, 0x82,
	0x01, 0x22, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05,
	0x00, 0x03, 0x82, 0x01, 0x0F, 0x00, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0x81,
	0xAC, 0x46, 0xD5, 0xFB, 0xB6, 0xD6, 0xCC, 0x62, 0xF6, 0x3B, 0x73, 0x50, 0x2F, 0xC3, 0x78, 0xEB,
	0xA5, 0x2C, 0x6C, 0x50, 0x6D, 0xE5, 0x2F, 0xCE, 0x87, 0xB2, 0x48, 0x12, 0xBC, 0x7F, 0x30, 0x40,
	0x8B, 0x2E, 0x02, 0x74, 0xC3, 0x84, 0xD1, 0x86, 0xCF, 0xB1, 0x7B, 0xDC, 0xA3, 0x86, 0xBF, 0xB9,
	0x87, 0xC3, 0xF2, 0x83, 0x1D, 0xC7, 0xA4, 0x85, 0x06, 0x59, 0x15, 0x24, 0x94, 0x7D, 0x77, 0x33,
	0x8C, 0x77, 0x33, 0x5A, 0x76, 0x71, 0xD4, 0x95, 0x7A, 0xC3, 0xF0, 0x1A, 0x9A, 0xD5, 0xE8, 0x16,
	0xD2, 0xEC, 0x41, 0x1F, 0x2B, 0x6E, 0x00, 0x4F, 0xB3, 0xCA, 0xB7, 0xFF, 0x66, 0xAC, 0x17, 0x1A,
	0x81, 0x9D, 0xA0, 0xF5, 0xB7, 0x59, 0xA5, 0x31, 0x4A, 0x50, 0x94, 0x43, 0x69, 0x56, 0x27, 0x17,
	0x36, 0x1B, 0xC1, 0x26, 0x25, 0xFB, 0x68, 0xEE, 0x11, 0x3B, 0x75, 0x08, 0x6B, 0xC9, 0xA7, 0x2B,
	0xFC, 0x16, 0x69, 0xBC, 0x44, 0x1C, 0xEF, 0x94, 0x4C, 0x85, 0x80, 0xC9, 0x26, 0xAE, 0x2D, 0xA8,
	0xCA, 0xA9, 0x11, 0x47, 0xEB, 0xA6, 0xB4, 0x7D, 0x3D, 0x7B, 0x69, 0xAB, 0xD1, 0xE6, 0x50, 0x4B,
	0x35, 0x18, 0xDB, 0xDD, 0xD0, 0x4B, 0x19, 0xCB, 0xF9, 0x9E, 0x7F, 0x47, 0x7C, 0xB2, 0xC9, 0x66,
	0xAF, 0x0E, 0x5C, 0xCF, 0x53, 0x37, 0x05, 0x22, 0x80, 0xA5, 0xB3, 0x12, 0x9C, 0xA6, 0x05, 0x8C,
	0x14, 0x7E, 0x06, 0xC5, 0x72, 0xD9, 0xBF, 0xBD, 0x3C, 0x3B, 0xD1, 0xDF, 0xA8, 0x68, 0x8D, 0xC8,
	0x2A, 0x49, 0x79, 0xD0, 0x78, 0xD5, 0x89, 0xC9, 0x9B, 0x4D, 0xEF, 0xB4, 0x99, 0x10, 0xD2, 0xFB,
	0x5F, 0xB6, 0xE0, 0x94, 0xFD, 0x15, 0xEA, 0x0F, 0xEB, 0xE5, 0xC1, 0xED, 0x22, 0x0F, 0x85, 0x41,
	0x8D, 0xB2, 0xF4, 0x16, 0x94, 0xC6, 0xCA, 0x1C, 0x80, 0x3A, 0x86, 0x41, 0x01, 0x09, 0x37, 0x02,
	0x03, 0x01, 0x00, 0x01, 0xA3, 0x82, 0x01, 0x7D, 0x30, 0x82, 0x01, 0x79, 0x30, 0x1F, 0x06, 0x03,
	0x55, 0x1D, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xAD, 0xBD, 0x98, 0x7A, 0x34, 0xB4, 0x26,
	0xF7, 0xFA, 0xC4, 0x26, 0x54, 0xEF, 0x03, 0xBD, 0xE0, 0x24, 0xCB, 0x54, 0x1A, 0x30, 0x1D, 0x06,
	0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x4E, 0x59, 0x66, 0xBF, 0xF3, 0x25, 0x08, 0x01,
	0xB2, 0x8A, 0xC2, 0xB1, 0x33, 0xE1, 0x3E, 0x01, 0x51, 0x7B, 0x82, 0x29, 0x30, 0x0E, 0x06, 0x03,
	0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x12, 0x06, 0x03,
	0x55, 0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xFF, 0x02, 0x01, 0x00,
	0x30, 0x17, 0x06, 0x03, 0x55, 0x1D, 0x20, 0x04, 0x10, 0x30, 0x0E, 0x30, 0x0C, 0x06, 0x0A, 0x2B,
	0x06, 0x01, 0x04, 0x01, 0x82, 0xA9, 0x30, 0x01, 0x01, 0x30, 0x44, 0x06, 0x03, 0x55, 0x1D, 0x1F,
	0x04, 0x3D, 0x30, 0x3B, 0x30, 0x39, 0xA0, 0x37, 0xA0, 0x35, 0x86, 0x33, 0x68, 0x74, 0x74, 0x70,
	0x3A, 0x2F, 0x2F, 0x63, 0x72, 0x6C, 0x2E, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72, 0x75, 0x73, 0x74,
	0x2E, 0x63, 0x6F, 0x6D, 0x2F, 0x41, 0x64, 0x64, 0x54, 0x72, 0x75, 0x73, 0x74, 0x45, 0x78, 0x74,
	0x65, 0x72, 0x6E, 0x61, 0x6C, 0x43, 0x41, 0x52, 0x6F, 0x6F, 0x74, 0x2E, 0x63, 0x72, 0x6C, 0x30,
	0x81, 0xB3, 0x06, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x01, 0x04, 0x81, 0xA6, 0x30,
	0x81, 0xA3, 0x30, 0x3F, 0x06, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x02, 0x86, 0x33,
	0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x63, 0x72, 0x74, 0x2E, 0x75, 0x73, 0x65, 0x72, 0x74,
	0x72, 0x75, 0x73, 0x74, 0x2E, 0x63, 0x6F, 0x6D, 0x2F, 0x41, 0x64, 0x64, 0x54, 0x72, 0x75, 0x73,
	0x74, 0x45, 0x78, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C, 0x43, 0x41, 0x52, 0x6F, 0x6F, 0x74, 0x2E,
	0x70, 0x37, 0x63, 0x30, 0x39, 0x06, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x02, 0x86,
	0x2D, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x63, 0x72, 0x74, 0x2E, 0x75, 0x73, 0x65, 0x72,
	0x74, 0x72, 0x75, 0x73, 0x74, 0x2E, 0x63, 0x6F, 0x6D, 0x2F, 0x41, 0x64, 0x64, 0x54, 0x72, 0x75,
	0x73, 0x74, 0x55, 0x54, 0x4E, 0x53, 0x47, 0x43, 0x43, 0x41, 0x2E, 0x63, 0x72, 0x74, 0x30, 0x25,
	0x06, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x86, 0x19, 0x68, 0x74, 0x74, 0x70,
	0x3A, 0x2F, 0x2F, 0x6F, 0x63, 0x73, 0x70, 0x2E, 0x75, 0x73, 0x65, 0x72, 0x74, 0x72, 0x75, 0x73,
	0x74, 0x2E, 0x63, 0x6F, 0x6D, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01,
	0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x1F, 0x73, 0xD4, 0xC8, 0xBF, 0x1D, 0xB4,
	0xEB, 0xDF, 0x5F, 0xEA, 0xB5, 0x2A, 0x83, 0x69, 0x00, 0x01, 0xA8, 0xDF, 0x1A, 0xEA, 0x17, 0xA7,
	0x57, 0x6E, 0x54, 0xB2, 0x69, 0x3D, 0xE6, 0x46, 0x2B, 0x4C, 0xA5, 0x7A, 0x5A, 0xE5, 0x2C, 0xB0,
	0x11, 0xB4, 0x78, 0xF5, 0x74, 0xFE, 0xAE, 0x0B, 0x08, 0x63, 0x7F, 0x5E, 0x75, 0x6A, 0xAA, 0x39,
	0x4D, 0x2C, 0x4D, 0x49, 0x5A, 0x87, 0x94, 0x0C, 0xF7, 0x8A, 0x9A, 0x11, 0xBD, 0x3A, 0xE0, 0x9D,
	0xCC, 0xCD, 0xB4, 0xFB, 0x69, 0xCF, 0x3F, 0x0E, 0xFF, 0x21, 0x32, 0x97, 0x75, 0xBC, 0x1A, 0x84,
	0x22, 0xD4, 0x4D, 0x59, 0x7F, 0x9B, 0xD5, 0xF1, 0xC5, 0x69, 0x69, 0x0C, 0xD8, 0x70, 0xFF, 0xDE,
	0xAB, 0x88, 0x94, 0x6E, 0x07, 0xBD, 0x97, 0x32, 0x4F, 0x06, 0x2A, 0xAA, 0x31, 0xC1, 0x83, 0x1D,
	0xA1, 0x1E, 0xA1, 0x74, 0x97, 0x52, 0xFD, 0x21, 0xC8, 0x5D, 0xE3, 0x71, 0xA8, 0x32, 0xFF, 0x30,
	0x15, 0x79, 0x0F, 0x0B, 0x72, 0x0F, 0x3B, 0xE4, 0x21, 0xA8, 0xDA, 0xA9, 0x88, 0x1F, 0x85, 0xA8,
	0x8F, 0x57, 0xE4, 0x87, 0x40, 0x50, 0x19, 0xE6, 0xA0, 0x55, 0x15, 0xB5, 0x91, 0x4D, 0xD1, 0x0F,
	0xC3, 0x20, 0x3D, 0xAF, 0xF2, 0xD5, 0xA5, 0x08, 0x8C, 0xE9, 0xB6, 0xD7, 0xF2, 0xC5, 0xBA, 0xB6,
	0x54, 0xCD, 0x91, 0x69, 0x8D, 0xA4, 0x33, 0x0F, 0xA3, 0xE0, 0xBF, 0x8B, 0x44, 0x24, 0x79, 0x1D,
	0x81, 0x98, 0xC7, 0x27, 0x09, 0x53, 0x27, 0x91, 0xEF, 0x76, 0x9B, 0x75, 0xD3, 0x1C, 0x79, 0xF2,
	0x27, 0x88, 0x25, 0x52, 0xB6, 0x5C, 0xAC, 0xE7, 0x37, 0xF4, 0x7D, 0xA7, 0x64, 0xC3, 0xD4, 0xD3,
	0x95, 0x1E, 0xCE, 0x7E, 0xD5, 0x54, 0xF8, 0x63, 0x2D, 0xA7, 0x82, 0xFD, 0xD8, 0x31, 0x0B, 0x0A,
	0xA4, 0x39, 0x7A, 0xF0, 0x23, 0xBC, 0x33, 0x71, 0x5C
} ;
