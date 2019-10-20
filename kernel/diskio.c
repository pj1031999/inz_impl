/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "fat/diskio.h"
#include "rpi/sd.h"
#include "klibc.h"

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(void) {
  DSTATUS res = RES_OK;

  // Put your code here
  res = sd_init();
 
  return res;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp(
    BYTE * buff, /* Pointer to the destination object */
    DWORD sector, /* Sector number (LBA) */
    UINT offset,  /* Offset in the sector */
    UINT count    /* Byte count (bit15:destination) */
    ) {

  // Put your code here
  static BYTE buf[512];
  int res = sd_readblock(sector, buf, 1);

  memcpy(buff, buf + offset, count);
  
  return res != 0? RES_OK: RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT
disk_writep(const BYTE *buff, /* Pointer to the data to be written,
                                 NULL:Initiate/Finalize write operation */
            DWORD sc /* Sector number (LBA) or Number of bytes to send */
            ) {
  DRESULT res = RES_OK;
  res = sd_writeblock(sc, buff, 1);

  if (!buff) {
    if (sc) {

      // Initiate write process

    } else {

      // Finalize write process
    }
  } else {

    // Send data to the disk
  }

  return res;
}
