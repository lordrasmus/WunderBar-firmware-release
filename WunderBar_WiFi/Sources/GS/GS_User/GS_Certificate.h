/** @file   GS_Certificate.h
 *  @brief  File contains functions to handle SSL certificate with GS module
 *
 *  @author MikroElektronika
 *  @bug    No known bugs.
 */

extern const char cacert[];

// public functions

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
void GS_Cert_StoreInFlash(char* buff);

/**
*  @brief  Load Cert from Flash into GS module
*
*  Load Certificate from predefined address in flash into GS module.
*  Before loading new certificate delete if any with same name.
*
*  @return True if successful
*/
bool GS_Cert_LoadExistingCert();

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
bool GS_Cert_OpenSLLconn(char cid);
