/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb.h"
#include "mtp_file_system_adapter.h"
#include "hal/tinyvfs.h"
#include "sys/statvfs.h"
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DATE_YEAR_SHIFT  (9U)
#define DATE_YEAR_MASK   (0xFE00U)
#define DATE_MONTH_SHIFT (5U)
#define DATE_MONTH_MASK  (0x01E0U)
#define DATE_DAY_SHIFT   (0U)
#define DATE_DAY_MASK    (0x001FU)

#define TIME_HOUR_SHIFT   (11U)
#define TIME_HOUR_MASK    (0xF800U)
#define TIME_MINUTE_SHIFT (5U)
#define TIME_MINUTE_MASK  (0x07E0U)
#define TIME_SECOND_SHIFT (0U)
#define TIME_SECOND_MASK  (0x001FU)

typedef struct {
    DIR *dir;
    uint8_t flags;
} usb_device_mtp_dir_instance_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static usb_device_mtp_dir_instance_t s_DirInstance[USB_DEVICE_MTP_DIR_INSTANCE];

/*******************************************************************************
 * Code
 ******************************************************************************/

static usb_status_t USB_DeviceMtpAllocateDirHandle(DIR **dir) {
    uint32_t i;

    for (i = 0; i < USB_DEVICE_MTP_DIR_INSTANCE; i++) {
        if (s_DirInstance[i].flags == 0U) {
            s_DirInstance[i].flags = 1U;
            *dir = &s_DirInstance[i].dir;
            return kStatus_USB_Success;
        }
    }

    return kStatus_USB_Busy;
}

static usb_status_t USB_DeviceMtpFreeDirHandle(DIR *dir) {
    uint32_t i;

    for (i = 0; i < USB_DEVICE_MTP_DIR_INSTANCE; i++) {
        if ((s_DirInstance[i].flags != 0U) && (&s_DirInstance[i].dir == dir)) {
            s_DirInstance[i].flags = 0U;
            return kStatus_USB_Success;
        }
    }

    return kStatus_USB_Busy;
}

static uint32_t USB_DeviceUnicodeStringLength(const uint16_t *srcBuf)
{
    const uint8_t *src = (const uint8_t *)srcBuf;
    uint32_t count     = 0;

    if (srcBuf == NULL)
    {
        return 0;
    }

    while ((src[count] != 0U) || (src[count + 1U] != 0U) || ((count % 2U) != 0U))
    {
        count++;
    }

    return count;
}

static char ascii_buffer[256] = {0};
const char* to_ascii(const uint16_t *fileName){
    memset(ascii_buffer,0,sizeof ascii_buffer);

    uint8_t* p = (uint8_t*)fileName;
    uint8_t idx = 0;
    for(uint32_t i =0; i< USB_DeviceUnicodeStringLength(fileName); i++){
        if(i % 2 == 0){
            ascii_buffer[idx] = *p;
            idx++;
        }
        p++;
    }
    return ascii_buffer;
}

usb_status_t USB_DeviceMtpOpen(usb_device_mtp_file_handle_t *file, const uint16_t *fileName, uint32_t flags) {
    char mode[3] = {0};
    if ((flags & (USB_DEVICE_MTP_READ | USB_DEVICE_MTP_WRITE)) != 0U) {
        mode[0] = 'w';
        mode[1] = '+';
    }else if((flags & USB_DEVICE_MTP_WRITE) != 0U){
        mode[0] = 'w';
        mode[1] = '\0';
    }else if((flags & USB_DEVICE_MTP_READ) != 0U){
        mode[0] = 'r';
        mode[1] = '\0';
    }

    *file = (usb_device_mtp_file_handle_t *) fopen(to_ascii(fileName), mode);
    if (*file == NULL) {
        return kStatus_USB_Error;
    }
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpClose(usb_device_mtp_file_handle_t file) {

    if (file == NULL) {
        return kStatus_USB_InvalidHandle;
    }

    if (fclose(file) != 0) {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpLseek(usb_device_mtp_file_handle_t file, uint32_t offset) {
    FILE *fil;

    if (file == NULL) {
        return kStatus_USB_InvalidHandle;
    }

    fil = (FILE *) file;

    if (fseek(fil, offset, SEEK_SET) != 0) {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpRead(usb_device_mtp_file_handle_t file, void *buffer, uint32_t size, uint32_t *actualsize) {
    FILE *fil;

    if (file == NULL) {
        return kStatus_USB_InvalidHandle;
    }

    fil = (FILE *) file;
    *actualsize = fread(buffer, 1, size, fil);
    if (*actualsize != size) {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpWrite(usb_device_mtp_file_handle_t file, void *buffer, uint32_t size, uint32_t *actualsize) {
    FILE *fil;

    if (file == NULL) {
        return kStatus_USB_InvalidHandle;
    }

    fil = (FILE *) file;
    *actualsize = fwrite(buffer, 1, size, fil);
    if (*actualsize != size) {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpFstat(const uint16_t *fileName, usb_device_mtp_file_info_t *fileInfo) {
    if (fileInfo == NULL) {
        return kStatus_USB_InvalidParameter;
    }

    struct stat ret = {};
    if (stat(to_ascii(fileName), &ret) != 0) {
        return kStatus_USB_Error; /* return on error */
    }

    fileInfo->size = ret.st_size;

    fileInfo->attrib = 0U;
    if (ret.st_mode & S_IFDIR) {
        fileInfo->attrib |= USB_DEVICE_MTP_DIR;
    }

    /* copy file name, unicode encoding. */
    uint32_t count;
    uint8_t *src;
    uint8_t *dest;

    src = (uint8_t *) &fileName;
    dest = (uint8_t *) &fileInfo->name[0];
    count = 0;
    while ((src[count] != 0U) || (src[count + 1U] != 0U) || ((count % 2U) != 0U)) {
        dest[count] = src[count];
        count++;
    }
    dest[count] = 0U;
    dest[count + 1U] = 0U; /* terminate with 0x00, 0x00 */

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpUtime(const uint16_t *fileName, usb_device_mtp_file_time_stamp_t *timeStamp) {
//    FILINFO fno;
//
//    fno.fdate = (WORD)((((WORD)timeStamp->year - 1980U) << 9U) | ((WORD)timeStamp->month << 5U) | (WORD)timeStamp->day);
//    fno.ftime =
//        (WORD)(((WORD)timeStamp->hour << 11U) | ((WORD)timeStamp->minute * 32U) | ((WORD)timeStamp->second / 2U));
//
//    if (f_utime((const TCHAR *)fileName, &fno) != FR_OK)
//    {
//        return kStatus_USB_Error;
//    }
    timeStamp->year = 2023;
    timeStamp->month = 10;
    timeStamp->day = 20;
    timeStamp->hour = 5;
    timeStamp->minute = 5;
    timeStamp->second = 5;

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpOpenDir(usb_device_mtp_dir_handle_t *dir, const uint16_t *dirName) {
//    DIR *dir1;
//
//    if (USB_DeviceMtpAllocateDirHandle(&dir1) != kStatus_USB_Success)
//    {
//        return kStatus_USB_Busy;
//    }
//
//    dir1 = opendir(dirName);
//
//    if (dir1 == NULL)
//    {
//        USB_DeviceMtpFreeDirHandle(dir1);
//        return kStatus_USB_Error;
//    }
//
//    return kStatus_USB_Success;
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceMtpCloseDir(usb_device_mtp_dir_handle_t dir) {
//    DIR *dir1;
//
//    if (dir == NULL)
//    {
//        return kStatus_USB_InvalidHandle;
//    }
//
//    dir1 = (DIR *)dir;
//
//    if (f_closedir(dir1) != FR_OK)
//    {
//        USB_DeviceMtpFreeDirHandle(dir1);
//        return kStatus_USB_Error;
//    }
//
//    USB_DeviceMtpFreeDirHandle(dir1);
//
//    return kStatus_USB_Success;
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceMtpReadDir(usb_device_mtp_dir_handle_t dir, usb_device_mtp_file_info_t *fileInfo) {
//    DIR *dir1;
//    FILINFO fno;
//    uint32_t count;
//    uint8_t *src;
//    uint8_t *dest;
//
//    if (dir == NULL)
//    {
//        return kStatus_USB_InvalidHandle;
//    }
//
//    if (fileInfo == NULL)
//    {
//        return kStatus_USB_InvalidParameter;
//    }
//
//    dir1 = (DIR *)dir;
//
//    for (;;)
//    {
//        if (f_readdir(dir1, &fno) != FR_OK)
//        {
//            return kStatus_USB_Error; /* return on error */
//        }
//
//        if (fno.fname[0] == 0U)
//        {
//            return kStatus_USB_InvalidRequest; /* return on end of dir */
//        }
//
//        if ((fno.fattrib & AM_SYS) == 0U)
//        {
//            break; /* do not expose system directories or files */
//        }
//    }
//
//    fileInfo->size = fno.fsize;
//    fileInfo->dateUnion.dateBitField.year =
//        ((fno.fdate & DATE_YEAR_MASK) >> DATE_YEAR_SHIFT) + 1980U; /* Year origin from 1980 */
//    fileInfo->dateUnion.dateBitField.month  = (fno.fdate & DATE_MONTH_MASK) >> DATE_MONTH_SHIFT;
//    fileInfo->dateUnion.dateBitField.day    = (fno.fdate & DATE_DAY_MASK) >> DATE_DAY_SHIFT;
//    fileInfo->timeUnion.timeBitField.hour   = (fno.ftime & TIME_HOUR_MASK) >> TIME_HOUR_SHIFT;
//    fileInfo->timeUnion.timeBitField.minute = (fno.ftime & TIME_MINUTE_MASK) >> TIME_MINUTE_SHIFT;
//    fileInfo->timeUnion.timeBitField.second = ((fno.ftime & TIME_SECOND_MASK) >> TIME_SECOND_SHIFT)
//                                              << 1U; /* Second / 2 (0...29) */
//
//    fileInfo->attrib = 0U;
//    if (fno.fattrib & AM_DIR)
//    {
//        fileInfo->attrib |= USB_DEVICE_MTP_DIR;
//    }
//    if (fno.fattrib & AM_RDO)
//    {
//        fileInfo->attrib |= USB_DEVICE_MTP_READ_ONLY;
//    }
//
//    /* copy file name, unicode encoding. */
//    src   = (uint8_t *)&fno.fname[0];
//    dest  = (uint8_t *)&fileInfo->name[0];
//    count = 0;
//    while ((src[count] != 0U) || (src[count + 1U] != 0U) || ((count % 2U) != 0U))
//    {
//        dest[count] = src[count];
//        count++;
//    }
//    dest[count]      = 0U;
//    dest[count + 1U] = 0U; /* terminate with 0x00, 0x00 */
//
//    return kStatus_USB_Success;
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceMtpMakeDir(const uint16_t *fileName) {
//    if (f_mkdir((const TCHAR *)fileName) != FR_OK)
//    {
//        return kStatus_USB_Error;
//    }
//
//    return kStatus_USB_Success;
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceMtpUnlink(const uint16_t *fileName) {
    return unlink(to_ascii(fileName)) != 0 ? kStatus_USB_Error : kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpRename(const uint16_t *oldName, const uint16_t *newName) {
    return rename((const char *) oldName, (const char *) newName) != 0 ? kStatus_USB_Error : kStatus_USB_Success;
    //TODO
}

usb_status_t USB_DeviceMtpGetDiskTotalBytes(const uint16_t *path, uint64_t *totalBytes) {
    struct statvfs stat = {};
    const int ret = vfs_statvfs(to_ascii(path), &stat);

    if (ret != 0) {
        return kStatus_USB_InvalidParameter;
    }
    *totalBytes = stat.f_bsize * stat.f_blocks;

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpGetDiskFreeBytes(const uint16_t *path, uint64_t *freeBytes) {
    struct statvfs stat = {};
    const int ret = vfs_statvfs(to_ascii(path), &stat);

    if (ret != 0) {
        return kStatus_USB_InvalidParameter;
    }
    *freeBytes = stat.f_bsize * stat.f_bfree;

    return kStatus_USB_Success;
}
