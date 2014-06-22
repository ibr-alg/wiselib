/* Test get_fat() with FAT12 and FAT16 */

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

#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))

#define LD_CLUST(dir)	(((DWORD)LD_WORD(dir+DIR_FstClusHI)<<16) | LD_WORD(dir+DIR_FstClusLO))

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

/* Directory object structure (DIR) */

        typedef struct {
//            FATFS*	fs;				/* Pointer to the owner file system object (**do not change order**) */
//            WORD	id;				/* Owner file system mount ID (**do not change order**) */
            WORD	index;			/* Current read/write index number */
            DWORD	sclust;			/* Table start cluster (0:Root dir) */
            DWORD	clust;			/* Current cluster */
            DWORD	sect;			/* Current sector */
            BYTE*	dir;			/* Pointer to the current SFN entry in the win[] */
            BYTE*	fn;				/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
        } DIR;

/* File object structure (FIL) */

        typedef struct {
//            FATFS*	fs;				/* Pointer to the related file system object (**do not change order**) */
//            WORD	id;				/* Owner file system mount ID (**do not change order**) */
            BYTE	flag;			/* Status flags */
            BYTE	err;			/* Abort flag (error code) */
            DWORD	fptr;			/* File read/write pointer (Zeroed on file open) */
            DWORD	fsize;			/* File size */
            DWORD	sclust;			/* File start cluster (0:no cluster chain, always 0 when fsize is 0) */
            DWORD	clust;			/* Current cluster of fpter (not valid when fprt is 0) */
            DWORD	dsect;			/* Sector number appearing in buf[] (0:invalid) */
        } FIL;

/* File status structure (FILINFO) */

        typedef struct {
            DWORD	fsize;			/* File size */
            WORD	fdate;			/* Last modified date */
            WORD	ftime;			/* Last modified time */
            BYTE	fattrib;		/* Attribute */
            BYTE	fname[13];		/* Short file name (8.3 format) */
        } FILINFO;

        int init(Debug& debug, BlockMemory& block_memory) {
        // initialize
            bm_ = &block_memory;
            debug_ = &debug;
            debug_->debug("Initializing FAT32 in wiselib");

            return wf_mount();
        }

        // your methods here

/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

        FRESULT wf_open (
//            FIL* fp,			/* Pointer to the blank file object */
            const char* path	/* Pointer to the file name */
//            BYTE mode			/* Access mode and file open mode flags */
        )
        {
            FRESULT res;
            DIR dj;
            BYTE sfn[12];
            BYTE dir[32];
//            FATFS *fs = this;

            if (!fs)						/* Check file system */
                return FR_NOT_ENABLED;

//            if (!fp) return FR_INVALID_OBJECT;
//                fp->fs = 0;			/* Clear file object */
//            mode &= FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW;
//            res = find_volume(&dj.fs, &path, (BYTE)(mode & ~FA_READ));

            fs->flag = 0;
            dj.fn = sfn;
            res = follow_path(&dj, dir, path);	/* Follow the file path */
            //taken from pff.c, compare with ff.c

            if (res != FR_OK) return res;		/* Follow failed */
            if (!dir[0] || (dir[DIR_Attr] & AM_DIR)) {	/* It is a directory */
                debug_->debug("In if %x - %d", dir[0], res);
                return FR_NO_FILE;
            }
            debug_->debug("In open %s %d", dj.fn, res);
            fs->org_clust = LD_CLUST(dir);			/* File start cluster */
            fs->fsize = LD_DWORD(dir+DIR_FileSize);	/* File size */
            fs->fptr = 0;						/* File pointer */
            fs->flag = FA_OPENED;

            return res;
        }

/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

        FRESULT wf_read (
            void* buff,		/* Pointer to the read buffer (NULL:Forward data to the stream)*/
            WORD btr,		/* Number of bytes to read */
            WORD* br		/* Pointer to number of bytes read */
        )
        {
            DRESULT dr;
            CLUST clst;
            DWORD sect, remain;
            WORD rcnt;
            BYTE cs, *rbuff = (BYTE*)buff;
//            FATFS *fs = FatFs;

            *br = 0;
            if (!fs) return FR_NOT_ENABLED;		/* Check file system */

            if (!(fs->flag & FA_OPENED))		/* Check if opened */
                return FR_NOT_OPENED;
            remain = fs->fsize - fs->fptr;
            if (btr > remain) btr = (WORD)remain;			/* Truncate btr by remaining bytes */
//            debug_->debug("In read %d and %d", fs->fsize, btr);
            while (btr)	{									/* Repeat until all data transferred */
                if ((fs->fptr % 512) == 0) {				/* On the sector boundary? */
                    cs = (BYTE)(fs->fptr / 512 & (fs->csize - 1));	/* Sector offset in the cluster */
                    if (!cs) {								/* On the cluster boundary? */
                        clst = (fs->fptr == 0) ?			/* On the top of the file? */
                            fs->org_clust : get_fat(fs->curr_clust);
//                        debug_->debug("Entering WHILE %d",clst);
                        if (clst <= 1) goto fr_abort;
                        fs->curr_clust = clst;				/* Update current cluster */
                    }
                    sect = clust2sect(fs->curr_clust);		/* Get current sector */
                    if (!sect) goto fr_abort;
                    fs->dsect = sect + cs;
//                    debug_->debug("In read while %d and %d", fs->dsect, cs);
                }
                rcnt = (WORD)(512 - (fs->fptr % 512));		/* Get partial sector data from sector buffer */
                if (rcnt > btr) rcnt = btr;
                BYTE buffer[512];
                bm_->read(buffer, fs->dsect);
                for(int x=0; x<rcnt; x++) {
                    rbuff[x] = buffer[x+(WORD)(fs->fptr % 512)];
//                    debug_->debug("In read for %d and %c", rbuff[x], rbuff[x]);
                }
//                rbuff = (BYTE*)buff + (WORD)(fs->fptr % 512);
//                dr = disk_readp(!buff ? 0 : rbuff, fs->dsect, (WORD)(fs->fptr % 512), rcnt);
//                if (dr) goto fr_abort;
                fs->fptr += rcnt; rbuff += rcnt;			/* Update pointers and counters */
                btr -= rcnt; *br += rcnt;
                debug_->debug("In read end of while %d and %d", fs->fptr, btr);
            }
            return FR_OK;

        fr_abort:
            fs->flag = 0;
            return FR_DISK_ERR;
        }

/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

        FRESULT wf_write (
            /*const*/ void* buff,	/* Pointer to the data to be written */
            WORD btw,			/* Number of bytes to write (0:Finalize the current write operation) */
            WORD* bw			/* Pointer to number of bytes written */
        )
        {
            CLUST clst;
            DWORD sect, remain;
            /*const*/ BYTE *p = (BYTE*)buff;
            BYTE cs;
            WORD wcnt;
            BYTE readBuff[512];
//            FATFS *fs = FatFs;

            *bw = 0;
            if (!fs) return FR_NOT_ENABLED;		/* Check file system */
            if (!(fs->flag & FA_OPENED))		/* Check if opened */
                return FR_NOT_OPENED;

            if (!btw) {		/* Finalize request */
                if ((fs->flag & FA__WIP) /*&& disk_writep(0, 0)*/) goto fw_abort;
                fs->flag &= ~FA__WIP;
                return FR_OK;
            } else {		/* Write data request */
                if (!(fs->flag & FA__WIP))		/* Round-down fptr to the sector boundary */
                    fs->fptr &= 0xFFFFFE00;
            }
            remain = fs->fsize - fs->fptr;
            if (btw > remain) btw = (WORD)remain;			/* Truncate btw by remaining bytes */

            while (btw)	{									/* Repeat until all data transferred */
                if (((WORD)fs->fptr % 512) == 0) {			/* On the sector boundary? */
                    cs = (BYTE)(fs->fptr / 512 & (fs->csize - 1));	/* Sector offset in the cluster */
                    if (!cs) {								/* On the cluster boundary? */
                        clst = (fs->fptr == 0) ?			/* On the top of the file? */
                            fs->org_clust : get_fat(fs->curr_clust);
                        if (clst <= 1) goto fw_abort;
                        fs->curr_clust = clst;				/* Update current cluster */
                    }
                    sect = clust2sect(fs->curr_clust);		/* Get current sector */
                    if (!sect) goto fw_abort;
                    fs->dsect = sect + cs;
//                    if (disk_writep(0, fs->dsect)) goto fw_abort;	/* Initiate a sector write operation */
                    fs->flag |= FA__WIP;
                }
                wcnt = 512 - ((WORD)fs->fptr % 512);		/* Number of bytes to write to the sector */
                if (wcnt > btw) wcnt = btw;
//                if (disk_writep(p, wcnt)) goto fw_abort;	/* Send data to the sector */
                bm_->read(readBuff, fs->dsect);
                int i;
                for(i = 0; i<wcnt;i++) {
                    readBuff[i] = p[i];
//                    debug_->debug("In wf_write %c", readBuff[i]);
                }

                bm_->write(readBuff, fs->dsect);
                fs->fptr += wcnt; p += wcnt;				/* Update pointers and counters */
                btw -= wcnt; *bw += wcnt;
                if (((WORD)fs->fptr % 512) == 0) {
//                    if (disk_writep(0, 0)) goto fw_abort;	/* Finalize the currtent secter write operation */
                    fs->flag &= ~FA__WIP;
                }
                debug_->debug("In wf_write %d", btw);
            }

            return FR_OK;

        fw_abort:
            fs->flag = 0;
            return FR_DISK_ERR;
        }

/*-----------------------------------------------------------------------*/
/* Seek File R/W Pointer                                                 */
/*-----------------------------------------------------------------------*/

        FRESULT wf_lseek (
            DWORD ofs		/* File pointer from top of file */
        )
        {
            CLUST clst;
            DWORD bcs, sect, ifptr;
//            FATFS *fs = FatFs;


            if (!fs) return FR_NOT_ENABLED;		/* Check file system */
            if (!(fs->flag & FA_OPENED))		/* Check if opened */
                    return FR_NOT_OPENED;

            if (ofs > fs->fsize) ofs = fs->fsize;	/* Clip offset with the file size */
            ifptr = fs->fptr;
            fs->fptr = 0;
            if (ofs > 0) {
                bcs = (DWORD)fs->csize * 512;	/* Cluster size (byte) */
                if (ifptr > 0 &&
                    (ofs - 1) / bcs >= (ifptr - 1) / bcs) {	/* When seek to same or following cluster, */
                    fs->fptr = (ifptr - 1) & ~(bcs - 1);	/* start from the current cluster */
                    ofs -= fs->fptr;
                    clst = fs->curr_clust;
                } else {							/* When seek to back cluster, */
                    clst = fs->org_clust;			/* start from the first cluster */
                    fs->curr_clust = clst;
                }
                while (ofs > bcs) {				/* Cluster following loop */
                    clst = get_fat(clst);		/* Follow cluster chain */
                    if (clst <= 1 || clst >= fs->n_fatent) goto fe_abort;
                    fs->curr_clust = clst;
                    fs->fptr += bcs;
                    ofs -= bcs;
                }
                fs->fptr += ofs;
                sect = clust2sect(clst);		/* Current sector */
                if (!sect) goto fe_abort;
                fs->dsect = sect + (fs->fptr / 512 & (fs->csize - 1));
            }

            return FR_OK;

        fe_abort:
            fs->flag = 0;
            return FR_DISK_ERR;
        }

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

/* Fill memory */

        void mem_set (void* dst, int val, int cnt) {
            char *d = (char*)dst;
            while (cnt--) *d++ = (char)val;
        }

/* Compare memory to memory */

        int mem_cmp (const void* dst, const void* src, int cnt) {
            const char *d = (const char *)dst, *s = (const char *)src;
            int r = 0;
            while (cnt-- && (r = *d++ - *s++) == 0) ;
            return r;
        }

/*-----------------------------------------------------------------------*/
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/

        CLUST get_fat (	/* 1:IO error, Else:Cluster status */
            CLUST clst	/* Cluster# to get the link information */
        )
        {
            WORD wc, bc, ofs;
            BYTE buf[512];
//            FATFS *fs;      //  = FatFs


            if (clst < 2 || clst >= fs->n_fatent)	/* Range check */
                return 1;
//            debug_->debug("fs->type %d, %d", fs->fs_type, _FS_FAT32);
            switch (fs->fs_type) {
        #if _FS_FAT12
            case FS_FAT12 :
                bc = (WORD)clst;
                bc += bc / 2;
                ofs = bc % 512; bc /= 512;
                if (ofs != 511) {
//                    if (disk_readp(buf, fs->fatbase + bc, ofs, 2)) break;
                    bm_->read(buf, fs->fatbase + bc);
                    wc = LD_WORD(buf+ofs);
                } else {
                    bm_->read(buf, fs->fatbase + bc);
                    BYTE tmp = buf[511];
                    bm_->read(buf, fs->fatbase + bc + 1);
                    buf[1] = tmp;
                    wc = LD_WORD(buf);
//                    if (disk_readp(buf, fs->fatbase + bc, 511, 1)) break;
//                    if (disk_readp(buf+1, fs->fatbase + bc + 1, 0, 1)) break;
                }

                return (clst & 1) ? (wc >> 4) : (wc & 0xFFF);
        #endif
            case FS_FAT16 :
//                if (disk_readp(buf, fs->fatbase + clst / 256, (WORD)(((WORD)clst % 256) * 2), 2)) break;
                bm_->read(buf, fs->fatbase + clst / 256);
                return LD_WORD(buf+(WORD)(((WORD)clst % 256) * 2));
        #if _FS_FAT32
            case FS_FAT32 :
                bm_->read(buf, fs->fatbase + clst / 128);
                debug_->debug("In get_fat, clst %d, buf %d, %d, %d, %d", fs->fatbase + clst / 128, 0,0,0,0 );
//                if (disk_readp(buf, fs->fatbase + clst / 128, (WORD)(((WORD)clst % 128) * 4), 4)) break;
                return LD_DWORD(buf+(WORD)(((WORD)clst % 128) * 4)) & 0x0FFFFFFF;
        #endif
            }

            return 1;	/* An error occured at the disk I/O layer */
        }

/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/

        DWORD clust2sect (	/* !=0: Sector number, 0: Failed - invalid cluster# */
            CLUST clst		/* Cluster# to be converted */
        )
        {
//            FATFS *fs;      //  = FatFs


            clst -= 2;
            debug_->debug("In clust2sect cls %d, nfatent %d", clst, fs->n_fatent);
            if (clst >= (fs->n_fatent - 2)) return 0;		/* Invalid cluster# */
            return (DWORD)clst * fs->csize + fs->database;
        }

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
/* Directory handling - Rewind directory index                           */
/*-----------------------------------------------------------------------*/

        FRESULT dir_rewind (
            DIR *dj			/* Pointer to directory object */
        )
        {
            CLUST clst;
//            FATFS *fs;      //  = FatFs


            dj->index = 0;
            clst = dj->sclust;
            if (clst == 1 || clst >= fs->n_fatent)	/* Check start cluster range */
                return FR_DISK_ERR;
            if (_FS_FAT32 && !clst && fs->fs_type == FS_FAT32)  {	/* Replace cluster# 0 with root cluster# if in FAT32 */
                clst = (CLUST)fs->dirbase;
//                debug_->debug("In dir_rewind in IF %d", clst);
            }
            dj->clust = clst;						/* Current cluster */
            dj->sect = clst ? clust2sect(clst) : fs->dirbase;	/* Current sector */
//            debug_->debug("In dir_rewind, sc = %d, c = %d, sec = %d", dj->sclust, dj->clust, dj->sect);
            return FR_OK;	/* Seek succeeded */
        }

/*-----------------------------------------------------------------------*/
/* Pick a segment and create the object name in directory form           */
/*-----------------------------------------------------------------------*/

        FRESULT create_name (
            DIR *dj,			/* Pointer to the directory object */
            const char **path	/* Pointer to pointer to the segment in the path string */
        )
        {
            BYTE c, /*d, */ni, si, i, *sfn;
            const char *p;

            /* Create file name in directory form */
            sfn = dj->fn;

            mem_set(sfn, ' ', 11);
//            sfn[11] = '\0';
//            debug_->debug("In create_name path is \"%s\" %d", (dj->fn), c);
            si = i = 0; ni = 8;
            p = *path;
            for (;;) {
                c = p[si++];
                if (c <= ' ' || c == '/') break;	/* Break on end of segment */
                if (c == '.' || i >= ni) {
                    if (ni != 8 || c != '.') break;
                    i = 8; ni = 11;
                    continue;
                }
//                if (IsDBCS1(c) && i < ni - 1) {	/* DBC 1st byte? */
//                    d = p[si++];				/* Get 2nd byte */
//                    sfn[i++] = c;
//                    sfn[i++] = d;
//                } else {						/* Single byte code */
                    if (IsLower(c)) c -= 0x20;	/* toupper */
                    sfn[i++] = c;
//                }
            }
            *path = &p[si];						/* Rerurn pointer to the next segment */
            sfn[11] = (c <= ' ') ? 1 : 0;		/* Set last segment flag if end of path */
            debug_->debug("In create_name path is \"%s\" %d", dj->fn, c);
            return FR_OK;
        }

/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory index next                        */
/*-----------------------------------------------------------------------*/

        FRESULT dir_next (	/* FR_OK:Succeeded, FR_NO_FILE:End of table */
            DIR *dj			/* Pointer to directory object */
        )
        {
            CLUST clst;
            WORD i;
//            FATFS *fs;       // = FatFs (global)


            i = dj->index + 1;
            if (!i || !dj->sect)	/* Report EOT when index has reached 65535 */
                return FR_NO_FILE;

            if (!(i % 16)) {		/* Sector changed? */
                dj->sect++;			/* Next sector */

                if (dj->clust == 0) {	/* Static table */
                    if (i >= fs->n_rootdir)	/* Report EOT when end of table */
                        return FR_NO_FILE;
                }
                else {					/* Dynamic table */
                    if (((i / 16) & (fs->csize-1)) == 0) {	/* Cluster changed? */
                        clst = get_fat(dj->clust);		/* Get next cluster */
                        if (clst <= 1) return FR_DISK_ERR;
                        if (clst >= fs->n_fatent)		/* When it reached end of dynamic table */
                            return FR_NO_FILE;			/* Report EOT */
                        dj->clust = clst;				/* Initialize data for new cluster */
                        dj->sect = clust2sect(clst);
                    }
                }
            }

            dj->index = i;

            return FR_OK;
        }

/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/

        FRESULT dir_find (
            DIR *dj,		/* Pointer to the directory object linked to the file name */
            BYTE *dir		/* 32-byte working buffer */
        )
        {
            FRESULT res;
            BYTE c;
            BYTE buf[512];
            int counter = 0;
            res = dir_rewind(dj);			/* Rewind directory object */
            if (res != FR_OK) return res;
//            debug_->debug("In dir_find res is %d", dj->sect);
            do {
                if(bm_->read(buf, dj->sect)==SUCCESS) {
                    for(counter = 0; counter < 32; counter++) {
                        dir[counter] = buf[(WORD)((dj->index % 16) * 32)+counter];
//                        debug_->debug("In dir_find while \"%d\" %d", dir[counter], res);
                    }
//                    dir = buf+(WORD)((dj->index % 16) * 32);
//                    res = disk_readp(dir, dj->sect, (WORD)((dj->index % 16) * 32), 32)	/* Read an entry */
//                        ? FR_DISK_ERR : FR_OK;
                }
                else
                    break;
                c = dir[DIR_Name];	/* First character */
                if (c == 0) { res = FR_NO_FILE; break; }	/* Reached to end of table */
                if (!(dir[DIR_Attr] & AM_VOL) && !mem_cmp(dir, dj->fn, 11)) { /* Is it a valid entry? */
                    debug_->debug("In dir_find BREAKING \"%x\" and res is %d", dir[0], res);
                    break;
                }
                res = dir_next(dj);					/* Next entry */
            } while (res == FR_OK);
            debug_->debug("In dir_find \"%d\" and res is %d", dir[0], res);


            return res;
        }

/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

        FRESULT follow_path (	/* FR_OK(0): successful, !=0: error code */
            DIR *dj,			/* Directory object to return last directory and found object */
            BYTE *dir,			/* 32-byte working buffer */
            const char *path	/* Full-path string to find a file or directory */
        )
        {
            FRESULT res;

            while (*path == ' ') path++;		/* Skip leading spaces */
            if (*path == '/') path++;			/* Strip heading separator */
            dj->sclust = 0;						/* Set start directory (always root dir) */

            if ((BYTE)*path <= ' ') {			/* Null path means the root directory */
                res = dir_rewind(dj);
                // Taken from pff.c, compare dir_sdi() from ff.c
                dir[0] = 0;
            } else {							/* Follow path */
                for (;;) {
                    res = create_name(dj, &path);	/* Get a segment */
                    if (res != FR_OK) break;
                    res = dir_find(dj, dir);		/* Find it */
                    debug_->debug("In follow_path for is %d (0)", res);
                    if (res != FR_OK) {				/* Could not find the object */
                        if (res == FR_NO_FILE && !*(dj->fn+11))
                            res = FR_NO_PATH;
                        break;
                    }

                    if (*(dj->fn+11)) break;		/* Last segment match. Function completed. */

                    if (!(dir[DIR_Attr] & AM_DIR)) { /* Cannot follow because it is a file */
                        res = FR_NO_PATH; break;
                    }
                    dj->sclust = LD_CLUST(dir);
                }
            }
            debug_->debug("In follow_path is \"%lx\" %d", (DWORD)(dj->sclust), res);

            return res;
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

            /* Initialize the file system object */
            fsize = LD_WORD(buf+BPB_FATSz16);				/* Number of sectors per FAT */
            if (!fsize) fsize = LD_DWORD(buf+BPB_FATSz32);
            fsize *= buf[BPB_NumFATs];						/* Number of sectors in FAT area */
            this->fatbase = bsect + LD_WORD(buf+BPB_RsvdSecCnt); /* FAT start sector (lba) */
            this->csize = buf[BPB_SecPerClus];					/* Number of sectors per cluster */
            this->n_rootdir = LD_WORD(buf+BPB_RootEntCnt);		/* Nmuber of root directory entries, FAT32(0) */
            tsect = LD_WORD(buf+BPB_TotSec16);				/* Number of sectors on the file system */
            if (!tsect) {
                tsect = LD_DWORD(buf+BPB_TotSec32);
            }
//            debug_->debug("Debugging here: fatbase %d, csize %d, rootdir %d, tsect %ld ",(int)this->fatbase, (int)this->csize, (int)this->n_rootdir, (long)tsect);
            mclst = (tsect						/* Last cluster# + 1, mclst holds total number of clusters */
                - LD_WORD(buf+BPB_RsvdSecCnt) - fsize - this->n_rootdir / 16
                ) / this->csize + 2;
            this->n_fatent = (CLUST)mclst;
            // tsect - 32 - 3592 = 1944024 / 8 = 243003 + 2 = 243005
//            debug_->debug("Here %lx", (long unsigned)this->n_fatent);
//            fmt = FS_FAT16;							/* Determine the FAT sub type */
            fmt = FS_FAT32;							/* Forcing FAT32 */

//            if (mclst < 0xFF7) 						/* Number of clusters < 0xFF5 */
//        #if _FS_FAT12
//                fmt = FS_FAT12;
//        #else
//                return FR_NO_FILESYSTEM;
//        #endif
//            if (mclst >= 0xFFF7)					/* Number of clusters >= 0xFFF5 */
//        #if _FS_FAT32
//                fmt = FS_FAT32;
//        #else
//                return FR_NO_FILESYSTEM;
//        #endif

            this->fs_type = fmt;		/* FAT sub-type */

            if (_FS_FAT32 && fmt == FS_FAT32)
                this->dirbase = LD_DWORD(buf+(BPB_RootClus));	/* Root directory start cluster */
            else
                this->dirbase = this->fatbase + fsize;				/* Root directory start sector (lba) */
            this->database = this->fatbase + fsize + this->n_rootdir / 16;	/* Data start sector (lba) */

//            debug_->debug("FAT DB is %d, DirB is %d",(int)this->database, (int)this->dirbase);
            this->flag = 0;
            fs = this;
//            debug_->debug("In mount fstype %d",fs->fs_type);
            return FR_OK;
        }

};

}   // namespace wiselib
