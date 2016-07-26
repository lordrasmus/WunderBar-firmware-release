/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
******************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
* THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************//*!
*
* @file usb_framework.c
*
* @author
*
* @version
*
* @date
*
* @brief The file contains USB stack framework module implementation.
*
*****************************************************************************/
/******************************************************************************
* Includes
*****************************************************************************/
#include "types.h"          /* Contains User Defined Data Types */
#include "usb_class.h"      /* USB class Header File */
#include "usb_framework.h"  /* USB Framework Header File */
#include "usb_descriptor.h" /* USB descriptor Header File */
#if defined(__IAR_SYSTEMS_ICC__)
#include <intrinsics.h>
#endif
#ifdef OTG_BUILD
#include "usb_otg_main.h"   /* OTG header file */
#endif
#include <string.h>
/*****************************************************************************
* Constant and Macro's
*****************************************************************************/
/****************************************************************************
* Global Variables
****************************************************************************/
static USB_SETUP_STRUCT g_setup_pkt;
/*is used to store the value of data which needs to be sent to the USB
host in response to the standard requests.*/
static uint_16 g_std_framework_data;
/*used to store the address received in Set Address in the standard request.*/
static uint_8  g_assigned_address;
/* Framework module callback pointer */
static USB_CLASS_CALLBACK g_framework_callback=NULL;
/* Other Requests Callback pointer */
static USB_REQ_FUNC g_other_req_callback=NULL;
#ifdef DELAYED_PROCESSING
static USB_DEV_EVENT_STRUCT g_f_event;
/* initially no pending request */
static boolean g_control_pending=FALSE;
#endif
boolean const g_validate_request[MAX_STRD_REQ][3] =
{
    TRUE,TRUE,FALSE, /*USB_Strd_Req_Get_Status*/
    /* configured state: valid for existing interfaces/endpoints
            address state   : valid only for interface or endpoint 0
            default state   : not specified
        */
    TRUE,TRUE,FALSE, /* Clear Feature */
    /* configured state: valid only for device in configured state
            address state   : valid only for device (in address state),
                            interface and endpoint 0
            default state   : not specified
        */
    FALSE,FALSE,FALSE, /*reserved for future use*/
    /* configured state: request not supported
            address state   : request not supported
            default state   : request not supported
        */
#ifdef OTG_BUILD         
    TRUE,TRUE,TRUE, /* Set Feature */
    /*  configured state: A B-device that supports OTG features  
            address state   : shall be able to accept the SetFeature command
            default state   : in the Default, Addressed and Configured states
        */
#else
    TRUE,TRUE,FALSE, /* Set Feature */
    /*  configured state: valid only for device in configured state
            address state   : valid only for interface or endpoint 0
            default state   : not specified
        */
#endif                
    
    FALSE,FALSE,FALSE,/*reserved for future use*/
    /*  configured state: request not supported
            address state   : request not supported
            default state   : request not supported
        */
    FALSE,TRUE,TRUE, /*USB_Strd_Req_Set_Address*/
    /*  configured state: not specified
            address state   : changes to default state if specified addr == 0,
                            but uses newly specified address
            default state   : changes to address state if specified addr != 0
        */
    TRUE,TRUE,TRUE, /*USB_Strd_Req_Get_Descriptor*/
    /* configured state: valid request
            address state   : valid request
            default state   : valid request
        */
    FALSE,FALSE,FALSE, /*Set Descriptor*/
    /* configured state: request not supported
            address state   : request not supported
            default state   : request not supported
        */
    TRUE,TRUE,FALSE, /*USB_Strd_Req_Get_Config*/
    /* configured state: bConfiguration Value of current config returned
            address state   : value zero must be returned
            default state   : not specified
        */
    TRUE,TRUE,FALSE, /*USB_Strd_Req_Set_Config*/
    /* configured state: If the specified configuration value is zero,
                            then the device enters the Address state.If the
                            specified configuration value matches the
                            configuration value from a config descriptor,
                            then that config is selected and the device
                            remains in the Configured state. Otherwise, the
                            device responds with a Request Error.
            address state   : If the specified configuration value is zero,
                            then the device remains in the Address state. If
                            the specified configuration value matches the
                            configuration value from a configuration
                            descriptor, then that configuration is selected
                            and the device enters the Configured state.
                            Otherwise,response is Request Error.
            default state   : not specified
        */
    TRUE,FALSE,FALSE, /*USB_Strd_Req_Get_Interface*/
    /* configured state: valid request
            address state   : request error
            default state   : not specified
        */
    TRUE,FALSE,FALSE, /*USB_Strd_Req_Set_Interface*/
    /* configured state: valid request
            address state   : request error
            default state   : not specified
        */
    TRUE,FALSE,FALSE /*USB_Strd_Req_Sync_Frame*/
    /* configured state: valid request
            address state   : request error
            default state   : not specified
        */
};
/*****************************************************************************
* Global Functions Prototypes - None
*****************************************************************************/
/*****************************************************************************
* Local Types - None
*****************************************************************************/
/*****************************************************************************
* Local Functions Prototypes
*****************************************************************************/
static void USB_Control_Service (PTR_USB_DEV_EVENT_STRUCT event );
static void USB_Control_Service_Handler(uint_8 controller_ID,
uint_8 status,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Get_Status(uint_8      controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Feature(uint_8     controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Set_Address(uint_8     controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Assign_Address(uint_8 controller_ID);
static uint_8 USB_Strd_Req_Get_Config(uint_8      controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Set_Config(uint_8      controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Get_Interface(uint_8   controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Set_Interface(uint_8   controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Sync_Frame(uint_8      controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
static uint_8 USB_Strd_Req_Get_Descriptor(uint_8      controller_ID,
USB_SETUP_STRUCT * setup_packet,
uint_8_ptr *data,
USB_PACKET_SIZE *size);
#ifdef DELAYED_PROCESSING
static void USB_Control_Service_Callback(PTR_USB_DEV_EVENT_STRUCT event );
#endif
/*****************************************************************************
* Local Functions Prototypes
*****************************************************************************/
/*****************************************************************************
* Local Variables
*****************************************************************************/
#ifndef _MC9S08JS16_H
static uint_8 ext_req_to_host[32];/* used for extended OUT transactions on
                                    CONTROL ENDPOINT*/
#else
static uint_8 ext_req_to_host[16];/* used for extended OUT transactions on
                                    CONTROL ENDPOINT*/
#endif
/*****************************************************************************
* Global Variables
*****************************************************************************/
USB_REQ_FUNC 
#if ((!defined(_MC9S08MM128_H)) && (!defined(_MC9S08JE128_H)))
const 
#endif
g_standard_request[MAX_STRD_REQ] =
{
    USB_Strd_Req_Get_Status,
    USB_Strd_Req_Feature,
    NULL,
    USB_Strd_Req_Feature,
    NULL,
    USB_Strd_Req_Set_Address,
    USB_Strd_Req_Get_Descriptor,
    NULL,
    USB_Strd_Req_Get_Config,
    USB_Strd_Req_Set_Config,
    USB_Strd_Req_Get_Interface,
    USB_Strd_Req_Set_Interface,
    USB_Strd_Req_Sync_Frame
};
#if (defined(_MC9S08MM128_H) || defined(_MC9S08JE128_H))
#pragma CODE_SEG DEFAULT
#endif
/**************************************************************************//*!
*
* @name  USB_Framework_Init
*
* @brief The funtion initializes the Class Module
*
* @param controller_ID     : Controller ID
* @param class_callback    : Class callback pointer
* @param other_req_callback: Other Request Callback
*
* @return status
*         USB_OK           : When Successfull
*         Others           : Errors
*
******************************************************************************
* This fuction registers the service on the control endpoint
*****************************************************************************/
uint_8 USB_Framework_Init 
    (
        uint_8              controller_ID,       /* [IN] Controller ID */
        USB_CLASS_CALLBACK  class_callback,      /* Class Callback */
        USB_REQ_FUNC        other_req_callback   /* Other Request Callback */
    )
{
    /* Body */
    uint_8 error=USB_OK;
    /* save input parameters */
    g_framework_callback = class_callback;
    g_other_req_callback = other_req_callback;
    /* Register CONTROL service */
    error = USB_Device_Register_Service(controller_ID, USB_SERVICE_EP0,
#ifdef DELAYED_PROCESSING
    USB_Control_Service_Callback
#else
    USB_Control_Service
#endif
    );
    return error;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Framework_DeInit
*
* @brief The funtion De-initializes the Class Module
*
* @param controller_ID     : Controller ID
*
* @return status
*         USB_OK           : When Successfull
*         Others           : Errors
*
******************************************************************************
* This fuction unregisters control service
*****************************************************************************/
uint_8 USB_Framework_DeInit
    (
        uint_8 controller_ID       /* [IN] Controller ID */
    ) 
{
    /* Body */
    uint_8 error;
    /* Free framwork_callback */
    g_framework_callback = NULL;
    
    /* Free other_req_callback */
    g_other_req_callback = NULL;
    
    /* Unregister CONTROL service */
    error = USB_Device_Unregister_Service(controller_ID, USB_SERVICE_EP0);
    
    /* Return error */
    return error;
} /* EndBody */
#ifdef DELAYED_PROCESSING
/**************************************************************************//*!
*
* @name  USB_Framework_Periodic_Task
*
* @brief The funtion is called to respond to any control request
*
* @param None
*
* @return None
*
******************************************************************************
* This function checks for any pending requests and handles them if any
*****************************************************************************/
void USB_Framework_Periodic_Task(void)
{
    /* Body */
    /* if control request pending to be completed */
    if(g_control_pending==TRUE)
    {   
        /* handle pending control request */
        USB_Control_Service(&g_f_event);
        g_control_pending = FALSE;
    } /* EndIf */
} /* EndBody */
#endif
/**************************************************************************//*!
*
* @name  USB_Framework_Reset
*
* @brief The funtion resets the framework
*
* @param controller_ID     : Controller ID
*
* @return status
*         USB_OK           : When Successfull
*
******************************************************************************
* This function is used to reset the framework module
*****************************************************************************/
uint_8 USB_Framework_Reset 
    (
        uint_8 controller_ID /* [IN] Controller ID */
    )
{
    /* Body */
    UNUSED (controller_ID)
    return USB_OK;
} /* EndBody */
#ifdef DELAYED_PROCESSING
/**************************************************************************//*!
*
* @name  USB_Control_Service_Callback
*
* @brief The funtion can be used as a callback function to the service.
*
* @param event : Pointer to USB Event Structure
*
* @return None
*
******************************************************************************
* This function saves the event parameters and queues a pending request
*****************************************************************************/
void USB_Control_Service_Callback 
    (
        PTR_USB_DEV_EVENT_STRUCT event /* [IN] Pointer to USB Event Structure */
    )
{
    /* Body */
    /* save the event parameters */
    g_f_event.buffer_ptr = event->buffer_ptr;
    g_f_event.controller_ID = event->controller_ID;
    g_f_event.ep_num = event->ep_num;
    g_f_event.setup  = event->setup;
    g_f_event.len = event->len;
    g_f_event.errors = event->errors;
    /* set the pending request flag */
    g_control_pending = TRUE;
} /* EndBody */
#endif
/**************************************************************************//*!
*
* @name  USB_Control_Service
*
* @brief Called upon a completed endpoint 0 transfer
*
* @param event : Pointer to USB Event Structure
*
* @return None
*
******************************************************************************
* This function handles the data sent or received on the control endpoint
*****************************************************************************/
static void USB_Control_Service 
    (
        PTR_USB_DEV_EVENT_STRUCT event /* [IN] Pointer to USB Event Structure */
    )
{
    /* Body */
    uint_16    device_state = 0;
    uint_8     status = USBERR_INVALID_REQ_TYPE;
    uint_8_ptr data = NULL;
    USB_PACKET_SIZE size;
    /* get the device state  */
    (void)USB_Device_Get_Status(event->controller_ID, USB_STATUS_DEVICE_STATE, 
    &device_state);
    if (event->setup == TRUE)
    {
        /* get the setup packet */
        (void)memcpy(&g_setup_pkt, event->buffer_ptr, USB_SETUP_PKT_SIZE);
        
        /* take care of endianess  of the 16 bt fields correctly */
        g_setup_pkt.index = BYTE_SWAP16(g_setup_pkt.index);
        g_setup_pkt.value = BYTE_SWAP16(g_setup_pkt.value);
        g_setup_pkt.length = BYTE_SWAP16(g_setup_pkt.length);
        /* if the request is standard request */
        if ((g_setup_pkt.request_type & USB_REQUEST_CLASS_MASK) ==
                USB_REQUEST_CLASS_STRD)
        {
            /* if callback is not NULL */
            if (g_standard_request[g_setup_pkt.request] != NULL)
            {
                /* if the request is valid in this device state */
                if((device_state < USB_STATE_POWERED) &&
                        (g_validate_request[g_setup_pkt.request][device_state]
                            ==TRUE))
                {
                    /* Standard Request function pointers */
                    status = g_standard_request[g_setup_pkt.request]
                    (event->controller_ID, &g_setup_pkt, &data, 
                    (USB_PACKET_SIZE *)&size);
                } /* EndIf */
            } /* EndIf */
        }
        else /* for Class/Vendor requests */
        {
            /*get the length from the setup_request*/
            size = (USB_PACKET_SIZE)g_setup_pkt.length;
            if( (size != 0) &&
                    ((g_setup_pkt.request_type & USB_DATA_DIREC_MASK) ==
                        USB_DATA_TO_DEVICE) )
            {
                (void)memcpy(ext_req_to_host, &g_setup_pkt, USB_SETUP_PKT_SIZE);
                /* expecting host to send data (OUT TRANSACTION)*/
                (void)USB_Device_Recv_Data(event->controller_ID,
                CONTROL_ENDPOINT, ext_req_to_host+USB_SETUP_PKT_SIZE,
                (USB_PACKET_SIZE)(size));
                return;
            } /* EndIf */
            else if(g_other_req_callback != NULL)/*call class/vendor request*/
            {
                status = g_other_req_callback(event->controller_ID,
                &g_setup_pkt, &data, (USB_PACKET_SIZE*)&size);
            } /* EndIf */
        } /* EndIf */
        USB_Control_Service_Handler(event->controller_ID,
        status, &g_setup_pkt, &data, (USB_PACKET_SIZE*)&size);
    }
    /* if its not a setup request */
    else if(device_state == USB_STATE_PENDING_ADDRESS)
    {
        /* Device state is PENDING_ADDRESS */
        /* Assign the new adddress to the Device */
        (void)USB_Strd_Req_Assign_Address(event->controller_ID);
        return;
    }
    else if( ((g_setup_pkt.request_type &
                    USB_DATA_DIREC_MASK) == USB_DATA_TO_DEVICE) &&
            (event->direction == USB_RECV) )
    {
        /* execution enters Control Service because of
        OUT transaction on CONTROL_ENDPOINT*/
        if(g_other_req_callback != NULL)
        {   /* class or vendor request */
            size = (USB_PACKET_SIZE)(event->len + USB_SETUP_PKT_SIZE);
            status = g_other_req_callback(event->controller_ID,
            (USB_SETUP_STRUCT*)(ext_req_to_host), &data, 
            (USB_PACKET_SIZE*)&size);
        } /* EndIf */
        USB_Control_Service_Handler(event->controller_ID,
        status, &g_setup_pkt, &data, 
        (USB_PACKET_SIZE*)&size);
    } /* EndIf */
    return;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Control_Service_Handler
*
* @brief The function is used to send a response to the Host based.
*
* @param controller_ID : Controller ID
* @param status        : Status of request
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return None
*
******************************************************************************
* This function sends a response to the data received on the control endpoint.
* the request is decoded in the control service
*****************************************************************************/
static void USB_Control_Service_Handler 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        uint_8 status,                      /* [IN] Device Status */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    if(status == USBERR_INVALID_REQ_TYPE)
    {
        /* incase of error Stall endpoint */
        (void)USB_Device_Set_Status(controller_ID,
        (uint_8)(USB_STATUS_ENDPOINT | CONTROL_ENDPOINT |
        (USB_SEND << USB_COMPONENT_DIRECTION_SHIFT)),
        USB_STATUS_STALLED);
    }
    else /* Need to send Data to the USB Host */
    {
        /* send the data prepared by the handlers.*/
        if(*size > setup_packet->length)
        {
            *size = (USB_PACKET_SIZE)setup_packet->length;
        }
        /* send the data to the host */
        (void)USB_Class_Send_Data(controller_ID,
        CONTROL_ENDPOINT, *data, *size);
        if((setup_packet->request_type & USB_DATA_DIREC_MASK) ==
                USB_DATA_TO_HOST)
        {   /* Request was to Get Data from device */
            /* setup rcv to get status from host */
            (void)USB_Device_Recv_Data(controller_ID,
            CONTROL_ENDPOINT,NULL,0);
        } /* EndIf */
    } /* EndIf */
    return;
} /* EndBody */
/*************************************************************************//*!
*
* @name  USB_Strd_Req_Get_Status
*
* @brief  This function is called in response to Get Status request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : When Successfull
*               Others : When Error
*
******************************************************************************
* this is a ch9 request and is called by the host to know the status of the
* device, the interface and the endpoint
*****************************************************************************/
static uint_8 USB_Strd_Req_Get_Status 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    uint_8 interface, endpoint;
    uint_8 status;
    if((setup_packet->request_type & USB_REQUEST_SRC_MASK) ==
            USB_REQUEST_SRC_DEVICE)
    {    
        #ifdef OTG_BUILD
        
        if(setup_packet->index == USB_WINDEX_OTG_STATUS_SEL)
        {
            /* request for Device */
            status = USB_Device_Get_Status(controller_ID,
            USB_STATUS_OTG,&g_std_framework_data);
            g_std_framework_data &= GET_STATUS_OTG_MASK;
            g_std_framework_data = BYTE_SWAP16(g_std_framework_data);
            *size = OTG_STATUS_SIZE;
        }
        else
        #endif
        {          
            /* request for Device */
            status = USB_Device_Get_Status(controller_ID,
            USB_STATUS_DEVICE,&g_std_framework_data);
            g_std_framework_data &= GET_STATUS_DEVICE_MASK;
            g_std_framework_data = BYTE_SWAP16(g_std_framework_data);
            *size = DEVICE_STATUS_SIZE;
        } /* EndIf */
    }
    else if ((setup_packet->request_type & USB_REQUEST_SRC_MASK) ==
            USB_REQUEST_SRC_INTERFACE)
    {    
        /* request for Interface */
        interface  = (uint_8) setup_packet->index;
        status =  USB_Desc_Get_Interface(controller_ID,interface,
        (uint_8_ptr)&g_std_framework_data);
        *size = INTERFACE_STATUS_SIZE;
    }
    else if ((setup_packet->request_type & USB_REQUEST_SRC_MASK) ==
            USB_REQUEST_SRC_ENDPOINT)
    {   
        /* request for Endpoint */
        endpoint  = (uint_8)(((uint_8) setup_packet->index) | 
        USB_STATUS_ENDPOINT);
        status =  USB_Device_Get_Status(controller_ID,
        endpoint,
        &g_std_framework_data);
        g_std_framework_data = BYTE_SWAP16(g_std_framework_data);
        *size = ENDP_STATUS_SIZE;
    } /* EndIf */
    *data = (uint_8_ptr)&g_std_framework_data;
    return status;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Feature
*
* @brief  This function is called in response to Clear or Set Feature request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : When Successfull
*               Others : When Error
*
******************************************************************************
* This is a ch9 request, used to set/clear a feature on the device
* (device_remote_wakeup and test_mode) or on the endpoint(ep halt)
*****************************************************************************/
static uint_8 USB_Strd_Req_Feature 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    uint_16 device_status;
    uint_8  set_request;
    uint_8  status = USBERR_INVALID_REQ_TYPE;
    uint_8  epinfo;
    uint_8  event;
    UNUSED (data)
    *size=0;
    /* find whether its a clear feature request or a set feature request */
    set_request = (uint_8)
    ((setup_packet->request & USB_SET_REQUEST_MASK) >> 1 );
    if((setup_packet->request_type & USB_REQUEST_SRC_MASK) ==
            USB_REQUEST_SRC_DEVICE)
    {
        if(set_request == TRUE)
        { 
            uint_16 device_set_feature_bitfield = DEVICE_SET_FEATURE_MASK;
            #ifdef OTG_BUILD
            OTG_DESCRIPTOR_PTR_T otg_desc_ptr;
            USB_PACKET_SIZE size ;
            status = USB_Desc_Get_Descriptor(controller_ID,
            USB_OTG_DESCRIPTOR,(uint_8)UNINITIALISED_VAL,(uint_16)UNINITIALISED_VAL,(uint_8_ptr *)&otg_desc_ptr,&size);
            if(status == USB_OK )
            {
                if(otg_desc_ptr->bmAttributes & USB_OTG_HNP_SUPPORT )
                {
                    device_set_feature_bitfield |= (1<<DEVICE_SET_FEATURE_B_HNP_ENABLE)|(1<<DEVICE_SET_FEATURE_A_HNP_SUPPORT);   
                } /* EndIf */
            } /* EndIf */
            #endif                          
            
            if(((uint_16)(1 << (uint_8)setup_packet->value)) & device_set_feature_bitfield)
            {
                status = USB_OK;
                if(setup_packet->value == DEVICE_FEATURE_REMOTE_WAKEUP)
                {
                    status = USB_Device_Get_Status(controller_ID,
                    USB_STATUS_DEVICE, &device_status);
                    /* add the request to be cleared from device_status */
                    device_status |= (uint_16)(1 << (uint_8)setup_packet->value);                   
                    
                    /* set the status on the device */
                    status = USB_Device_Set_Status(controller_ID,
                    USB_STATUS_DEVICE, device_status);
                }
                
                #ifdef OTG_BUILD
                else if(setup_packet->value == DEVICE_SET_FEATURE_B_HNP_ENABLE)
                {
                    _usb_otg_hnp_enable(controller_ID, set_request);
                } /* EndIf */
                #endif
            }
            else
            {
                status = USBERR_INVALID_REQ_TYPE;    
            } /* EndIf */
        }
        else//(set_request == FALSE) it is a clear feature request
        {
            if(((uint_16)(1 << (uint_8)setup_packet->value)) & DEVICE_CLEAR_FEATURE_MASK) 
            {
                status = USB_OK;
                if(setup_packet->value == DEVICE_FEATURE_REMOTE_WAKEUP)
                {
                    status = USB_Device_Get_Status(controller_ID,
                    USB_STATUS_DEVICE, &device_status);
                    /* remove the request to be cleared from device_status */
                    device_status &= (uint_16)~(1 << (uint_8)setup_packet->value);
                    status = USB_Device_Set_Status(controller_ID,
                    USB_STATUS_DEVICE, device_status);
                } /* EndIf */
            }
            else
            {
                status = USBERR_INVALID_REQ_TYPE;   
            } /* EndIf */
        } /* EndIf */
    }
    else if ((setup_packet->request_type & USB_REQUEST_SRC_MASK) ==
            USB_REQUEST_SRC_ENDPOINT)
    {
        /* request for Endpoint */
        epinfo = (uint_8)(setup_packet->index & 0x00FF);
        status = USB_Device_Set_Status(controller_ID,
        (uint_8)(epinfo|USB_STATUS_ENDPOINT), set_request);
        if((set_request == 0)&&(epinfo == 0x03))
        {
            #if defined(__CWCC__)
            asm(nop);
            #elif defined(__IAR_SYSTEMS_ICC__)
            __no_operation();
            #endif
        } /* EndIf */
        event = (uint_8)(set_request ? 
        USB_APP_EP_STALLED : USB_APP_EP_UNSTALLED);
        /* inform the upper layers of stall/unstall */
        g_framework_callback(controller_ID,event,(void*)&epinfo);
    } /* EndIf */
    return status;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Set_Address
*
* @brief  This function is called in response to Set Address request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : When Successfull
*               Others : When Error
*
******************************************************************************
* This is a ch9 request, saves the new address in a global variable. this
* address is assigned to the device after this transaction completes
*****************************************************************************/
static uint_8 USB_Strd_Req_Set_Address 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    UNUSED (data)
    *size=0;
    /* update device stae */
    (void)USB_Device_Set_Status(controller_ID,
    USB_STATUS_DEVICE_STATE, USB_STATE_PENDING_ADDRESS);
    /*store the address from setup_packet into assigned_address*/
    g_assigned_address = (uint_8)setup_packet->value;
    return USB_OK;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Assign_Address
*
* @brief  This function assigns the address to the Device
*
* @param controller_ID : Controller ID
*
* @return status
*                       USB_OK: Always
*
******************************************************************************
* This function assigns the new address and is called (from the control
* service) after the set address transaction completes
*****************************************************************************/
static uint_8 USB_Strd_Req_Assign_Address 
    (
        uint_8    controller_ID         /* [IN] Controller ID */
    )
{
    /* Body */
    /* Set Device Address */
    (void)USB_Device_Set_Address(controller_ID, g_assigned_address);
    /* Set Device state */
    (void)USB_Device_Set_Status(controller_ID,
    USB_STATUS_DEVICE_STATE, USB_STATE_ADDRESS);
    /* Set Device state */
    (void)USB_Device_Set_Status(controller_ID, USB_STATUS_ADDRESS,
    g_assigned_address);
    return USB_OK;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Get_Config
*
* @brief  This function is called in response to Get Config request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : Always
*
******************************************************************************
* This is a ch9 request and is used to know the currently used configuration
*****************************************************************************/
static uint_8 USB_Strd_Req_Get_Config 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    UNUSED (setup_packet)
    *size = CONFIG_SIZE;
    (void)USB_Device_Get_Status(controller_ID,
    USB_STATUS_CURRENT_CONFIG, &g_std_framework_data);
    g_std_framework_data = BYTE_SWAP16(g_std_framework_data);
    *data = (uint_8_ptr)(&g_std_framework_data);
    return USB_OK;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Set_Config
*
* @brief  This function is called in response to Set Config request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : When Successfull
*               Others : When Error
*
******************************************************************************
* This is a ch9 request and is used by the host to set the new configuration
*****************************************************************************/
static uint_8 USB_Strd_Req_Set_Config 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    uint_8 status = USBERR_INVALID_REQ_TYPE;
    uint_16 config_val;
    UNUSED (data)
    *size = 0;
    status = USB_STATUS_ERROR;
    config_val = setup_packet->value;
    if (USB_Desc_Valid_Configation(controller_ID, config_val))
    /*if valid configuration (fn returns boolean value)*/
    {
        uint_16 device_state = USB_STATE_CONFIG;
        
        /* if config_val is 0 */
        if (!config_val) 
        {
            device_state = USB_STATE_ADDRESS ;
        } /* EndIf */
        status = USB_Device_Set_Status(controller_ID, USB_STATUS_DEVICE_STATE, 
        device_state);
        status = USB_Device_Set_Status(controller_ID, 
        USB_STATUS_CURRENT_CONFIG, config_val);
        /*
        Callback to the app. to let the application know about the new
        Configuration
        */
        g_framework_callback(controller_ID,USB_APP_CONFIG_CHANGED,
        (void *)&config_val);
        g_framework_callback(controller_ID,USB_APP_ENUM_COMPLETE, NULL);
    } /* EndIf */
    return status;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Get_Interface
*
* @brief  This function is called in response to Get Interface request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : When Successfull
*               Others : When Error
*
******************************************************************************
* This is a ch9 request and is used to know the current interface
*****************************************************************************/
static uint_8 USB_Strd_Req_Get_Interface 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    uint_8 status;
    *size = INTERFACE_STATUS_SIZE;
    status = USB_Desc_Get_Interface(controller_ID,(uint_8)setup_packet->index,
    (uint_8_ptr)&g_std_framework_data);
    *data = (uint_8_ptr)&g_std_framework_data;
    return status;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Set_Interface
*
* @brief  This function is called in response to Set Interface request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : Always
*
******************************************************************************
* This is a ch9 request and is used by the  host to set the interface
*****************************************************************************/
static uint_8 USB_Strd_Req_Set_Interface 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    uint_8 status;
    UNUSED (data)
    *size=0;
    /* Request type not for interface */
    if ((setup_packet->request_type & 0x03) != 0x01)
    {
        return USB_STATUS_ERROR;
    } /* EndIf */
    /* Get Interface and alternate interface from setup_packet */
    status = USB_Desc_Set_Interface(controller_ID,(uint_8)setup_packet->index, 
    (uint_8)setup_packet->value);
    return USB_OK;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Strd_Req_Sync_Frame
*
* @brief  This function is called in response to Sync Frame request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : When Successfull
*               Others : When Error
*
******************************************************************************
* This req is used to set and then report an ep's synchronization frame
*****************************************************************************/
static uint_8 USB_Strd_Req_Sync_Frame 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    uint_8 status;
    UNUSED (setup_packet)
    *size=FRAME_SIZE;
    /* Get the frame number */
    status = USB_Device_Get_Status(controller_ID, USB_STATUS_SOF_COUNT,
    &g_std_framework_data);
    *data = (uint_8_ptr)&g_std_framework_data;
    return status;
} /* EndBody */
/**************************************************************************//*!
*
* @name  USB_Std_Req_Get_Descriptor
*
* @brief  This function is called in response to Get Descriptor request
*
* @param controller_ID : Controller ID
* @param setup_packet  : Setup packet received
* @param data          : Data to be send back
* @param size          : Size to be returned
*
* @return status:
*               USB_OK : When Successfull
*               Others : When Error
*
******************************************************************************
* This is a ch9 request and is used to send the descriptor requested by the
* host
*****************************************************************************/
static uint_8 USB_Strd_Req_Get_Descriptor 
    (
        uint_8    controller_ID,            /* [IN] Controller ID */
        USB_SETUP_STRUCT * setup_packet,    /* [IN] Setup packet received */
        uint_8_ptr *data,                   /* [OUT] Data to be send back */
        USB_PACKET_SIZE *size               /* [OUT] Size to be returned */
    )
{
    /* Body */
    uint_8 type = USB_uint_16_high(setup_packet->value);
    uint_16 index = (uint_8)UNINITIALISED_VAL;
    uint_8 str_num = (uint_8)UNINITIALISED_VAL;
    uint_8 status;
    if (type == STRING_DESCRIPTOR_TYPE)
    {
        /* for string descriptor set the language and string number */
        index = setup_packet->index;
        /*g_setup_pkt.lValue*/
        str_num = USB_uint_16_low(setup_packet->value);
    } /* EndIf */
    /* Call descriptor class to get descriptor */
    status = USB_Desc_Get_Descriptor(controller_ID,
    type,str_num,index,data,size);
    return status;
} /* EndBody */

/* EOF */
