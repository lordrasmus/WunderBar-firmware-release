/**
 * @file GS_API_platform.c
 *
 * Methods for handling platform specific functionality 
 */
#include <stdarg.h>
#include <stdio.h>

#include "GS_API.h"
#include "GS_API_private.h"
#include "../AT/AtCmdLib.h"
#include <hardware/Hw_modules.h>



/** Public Method Implementation **/
void GS_API_Printf(const char *format, ...){

}

void GS_API_Init(void){

	 GS_Api_SetResponseTimeoutHandle(5000);

	 // Send a \r\n to sync communication
     GS_HAL_send((uint8_t *) "\r\n", 2);

     // Flush buffer until we get a valid response
     AtLib_FlushIncomingMessage();
        
     // Try to reset
     if(AtLibGs_Reset() == HOST_APP_MSG_ID_APP_RESET)
          GS_API_Printf("Reset OK");
     else
          GS_API_Printf("Reset Fail");

     // Turn off echo
     if (AtLibGs_SetEcho(0) != HOST_APP_MSG_ID_OK)
    	 return;
     // Turn on Bulk Data
     if (AtLibGs_BData(1) != HOST_APP_MSG_ID_OK)
    	 return;

     AtLibGs_EnableRadio(1);
     AtLib_SetAntennaConf(1);

}
