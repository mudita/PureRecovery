/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb.h"
#include "mtp_file_system_adapter.h"
#include "mtp_object_handle.h"
#include <stdio.h>
#include <unistd.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static FILE * s_File;
static const char* file_path = "/user/handle.bin";

/*******************************************************************************
 * Code
 ******************************************************************************/


usb_status_t USB_DeviceMtpObjHandleInit(void)
{
    fclose(s_File);
    s_File = fopen(file_path,"wb+");

    if (s_File == NULL)
    {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpObjHandleDeinit(void)
{
    fclose(s_File);

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpObjHandleRead(uint32_t objHandle, usb_mtp_obj_handle_t *objHandleStruct)
{
    if (fseek(s_File, (objHandle - 1U) * sizeof(usb_mtp_obj_handle_t),SEEK_SET) == 0)
    {
        const size_t size = fread(objHandleStruct, 1,sizeof(usb_mtp_obj_handle_t), s_File);
        if(size != sizeof(usb_mtp_obj_handle_t)){
            return kStatus_USB_Error;
        }
    }
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpObjHandleWrite(uint32_t objHandle, usb_mtp_obj_handle_t *objHandleStruct)
{
    if (fseek(s_File, (objHandle - 1U) * sizeof(usb_mtp_obj_handle_t),SEEK_SET) == 0)
    {
        const size_t size = fwrite(objHandleStruct, 1,sizeof(usb_mtp_obj_handle_t), s_File);
        if(size != sizeof(usb_mtp_obj_handle_t)){
            return kStatus_USB_Error;
        }
    }
    return kStatus_USB_Success;
}
