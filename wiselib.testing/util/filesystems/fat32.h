#include <stdio.h>
#include "pff.h"
#include "integer.h"
#include "diskio.h"

/* FatFs refers the members in the FAT structures with byte offset instead
/ of structure member because there are incompatibility of the packing option
/ between various compilers. */

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec		11
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82

#define MBR_Table			446

#define	DIR_Name			0
#define	DIR_Attr			11
#define	DIR_NTres			12
#define	DIR_CrtTime			14
#define	DIR_CrtDate			16
#define	DIR_FstClusHI		20
#define	DIR_WrtTime			22
#define	DIR_WrtDate			24
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28


namespace wiselib {

template<
    typename OsModel_P,
	typename BlockMemory_P = typename OsModel_P::BlockMemory,
	typename Debug_P = typename OsModel_P::Debug
>
class Fat32 {
    public:
        typedef OsModel_P OsModel;
        typedef BlockMemory_P BlockMemory;
        typedef Debug_P Debug;

        enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_IO = OsModel::ERR_IO,
				ERR_NOMEM = OsModel::ERR_NOMEM,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
        };

        int init(Debug& debug, BlockMemory& block_memory) {
        // initialize
            bm_ = &block_memory;
            debug_ = &debug;
            debug_->debug("Initializing FAT32 in wiselib");
            wf_mount();
            return 0;
        }

        // your methods here

    private:
        typename Debug::self_pointer_t debug_;
        typename BlockMemory::self_pointer_t bm_;
        /*static */Fat32<OsModel, BlockMemory, Debug> *fs;

/* FAT Attributes   */
        BYTE	fs_type;	/* FAT sub type */
        BYTE	flag;		/* File status flags */
        BYTE	csize;		/* Number of sectors per cluster */
        BYTE	pad1;
        WORD	n_rootdir;	/* Number of root directory entries (0 on FAT32) */
        CLUST	n_fatent;	/* Number of FAT entries (= number of clusters + 2) */
        DWORD	fatbase;	/* FAT start sector */
        DWORD	dirbase;	/* Root directory start sector (Cluster# on FAT32) */
        DWORD	database;	/* Data start sector */
        DWORD	fptr;		/* File R/W pointer */
        DWORD	fsize;		/* File size */
        CLUST	org_clust;	/* File start cluster */
        CLUST	curr_clust;	/* File current cluster */
        DWORD	dsect;		/* File current data sector */

/*-----------------------------------------------------------------------*/
/* Check a sector if it is an FAT boot record                            */
/*-----------------------------------------------------------------------*/

        BYTE check_fs (	/* 0:The FAT boot record, 1:Valid boot record but not an FAT, 2:Not a boot record, 3:Error */
            BYTE *buf,	/* Working buffer */
            DWORD sect	/* Sector# (lba) to check if it is an FAT boot record or not */
        )
        {
            if(bm_->read(buf, sect)==SUCCESS) {

                // Assumed only for Little-endian systems, if(buf[510]!=0xAA || buf[511]!=0x55) for Big-endian
                if(buf[510]!=0x55 || buf[511]!=0xAA) {
                    debug_->debug("Invalid Boot signature detected.");
                    return 2;
                }
                if(buf[54]==0x46 && buf[55]==0x41) {
                    debug_->debug("FAT 12/16 found");
                    return 0;
                }
                if(buf[82]==0x46 && buf[83]==0x41) {
                    debug_->debug("FAT 32 found");
                    return 0;
                }
            }
            else {
                debug_->debug("Error reading Block device");
                return 3;
            }
            return 1;
        }

/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Locical Drive                                         */
/*-----------------------------------------------------------------------*/

        FRESULT wf_mount (
//            BlockMemory& block_memory = &bm_               /* Comment about block memory */
        ) {
            BYTE fmt, buf[512];
            DWORD bsect, fsize, tsect, mclst;

            fs = 0;
            if (!this) return FR_OK;				/* Unregister fs object */

/* Assume Disk is initialized */
//            if (disk_initialize() & STA_NOINIT)	/* Check if the drive is ready or not */
//                return FR_NOT_READY;

            /* Search FAT partition on the drive */
            bsect = 0;
            fmt = check_fs(buf, bsect);			/* Check sector 0 as an SFD format */
            if (fmt == 1) {						/* Not an FAT boot record, it may be FDISK format */
                /* Check a partition listed in top of the partition table */
//                if (disk_readp(buf, bsect, MBR_Table, 16)) {	/* 1st partition entry */
//                    fmt = 3;
//                } else {
                debug_->debug("Checking fmt again");
                    if (buf[446+4]) {					/* Is the partition existing? */
                        bsect = LD_DWORD(&buf[446+8]);	/* Partition offset in LBA */
                        fmt = check_fs(buf, bsect);	/* Check the partition */
                        debug_->debug("Checking FS again, %d and %d (fmt)",bsect, fmt);
                    }
//                }
            }
            if (fmt == 3) return FR_DISK_ERR;
            if (fmt) return FR_NO_FILESYSTEM;	/* No valid FAT patition is found */

//            /* Initialize the file system object */

            fsize = LD_WORD(buf+BPB_FATSz16);				/* Number of sectors per FAT */
            if (!fsize) fsize = LD_DWORD(buf+BPB_FATSz32);
            fsize *= buf[BPB_NumFATs];						/* Number of sectors in FAT area */
            this->fatbase = bsect + LD_WORD(buf+BPB_RsvdSecCnt); /* FAT start sector (lba) */
            this->csize = buf[BPB_SecPerClus];					/* Number of sectors per cluster */
            this->n_rootdir = LD_WORD(buf+BPB_RootEntCnt);		/* Nmuber of root directory entries, 0 for FAT32 */
            tsect = LD_WORD(buf+BPB_TotSec16);				/* Number of sectors on the file system */
            if (!tsect) {
                tsect = LD_DWORD(buf+BPB_TotSec32);
            }
//            debug_->debug("Debugging here: fatbase %d, csize %d, rootdir %d, tsect %lx ",(int)fs->fatbase, (int)fs->csize, (int)fs->n_rootdir, (long unsigned)tsect);
            mclst = (tsect						/* Last cluster# + 1, mclst holds total number of clusters */
                - LD_WORD(buf+BPB_RsvdSecCnt) - fsize - this->n_rootdir / 16
                ) / this->csize + 2;
            this->n_fatent = (CLUST)mclst;
            // tsect - 32 - 3592 = 1944024 / 8 = 243003 + 2 = 243005
//            debug_->debug("Here %lx", (long unsigned)this->n_fatent);
            fmt = FS_FAT16;							/* Determine the FAT sub type */
            if (mclst < 0xFF7) 						/* Number of clusters < 0xFF5 */
        #if _FS_FAT12
                fmt = FS_FAT12;
        #else
                return FR_NO_FILESYSTEM;
        #endif
            if (mclst >= 0xFFF7)					/* Number of clusters >= 0xFFF5 */
        #if _FS_FAT32
                fmt = FS_FAT32;
        #else
                return FR_NO_FILESYSTEM;
        #endif

            this->fs_type = fmt;		/* FAT sub-type */

            if (_FS_FAT32 && fmt == FS_FAT32)
                this->dirbase = LD_DWORD(buf+(BPB_RootClus));	/* Root directory start cluster */
            else
                this->dirbase = this->fatbase + fsize;				/* Root directory start sector (lba) */
            this->database = this->fatbase + fsize + this->n_rootdir / 16;	/* Data start sector (lba) */

//            debug_->debug("FAT DB is %d, DirB is %d",(int)this->database, (int)this->dirbase);
            this->flag = 0;
            fs = this;

            return FR_OK;
        }

};

}   // namespace wiselib
