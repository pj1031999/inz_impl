/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "fat/ff.h"     /* Obtains integer types */
#include "fat/diskio.h" /* Declarations of disk functions */
#include "rpi/sd.h"
#include "klibc.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM 0 /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC 1 /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB 2 /* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS
disk_status(BYTE pdrv __unused /* Physical drive nmuber to identify the drive */
) {
  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
  BYTE pdrv __unused /* Physical drive nmuber to identify the drive */
) {
  return sd_init();
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT
disk_read(BYTE pdrv __unused, /* Physical drive nmuber to identify the drive */
          BYTE *buff,         /* Data buffer to store read data */
          LBA_t sector,       /* Start sector in LBA */
          UINT count          /* Number of sectors to read */
) {
  int res = sd_readblock(sector, buff, count);
  return res != 0 ? RES_OK : RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT
disk_write(BYTE pdrv __unused, /* Physical drive nmuber to identify the drive */
           const BYTE *buff __unused, /* Data to be written */
           LBA_t sector __unused,     /* Start sector in LBA */
           UINT count __unused        /* Number of sectors to write */
) {
  return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv __unused, /* Physical drive nmuber (0..) */
                   BYTE cmd __unused,  /* Control code */
                   void *buff __unused /* Buffer to send/receive control data */
) {
  return RES_PARERR;
}
