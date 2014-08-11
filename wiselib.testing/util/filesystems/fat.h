/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

/*
 * File:   fat.h
 * Author: Dhruv
 *
 */

#include <external_interface/external_interface.h>
#include <util/string_util.h>

#if _FS_FAT32
#define	CLUST	::uint32_t
#else
#define	CLUST	::uint16_t
#endif

#define _FS_FAT12	1	/* 1:Enable FAT12 support */
#define _FS_FAT32	1	/* 1:Enable FAT32 support */

namespace wiselib {

template<
    typename OsModel_P,
	typename BlockMemory_P = typename OsModel_P::BlockMemory,
	typename Debug_P = typename OsModel_P::Debug
>
class Fat {
    public:
        typedef OsModel_P OsModel;
        typedef BlockMemory_P BlockMemory;
        typedef Debug_P Debug;
        typedef typename OsModel::block_data_t block_data_t;

        enum {
            SUCCESS = OsModel::SUCCESS,
            ERR_IO = OsModel::ERR_IO,
            ERR_NOMEM = OsModel::ERR_NOMEM,
            ERR_UNSPEC = OsModel::ERR_UNSPEC
        };

/**
 * Results of Disk Functions
 */
        typedef enum {
            RES_OK = 0,		/* 0: Function succeeded */
            RES_ERROR,		/* 1: Disk error */
            RES_NOTRDY,		/* 2: Not ready */
            RES_PARERR		/* 3: Invalid parameter */
        } DRESULT;

/**
 * File function return code (FRESULT)
 */
        typedef enum {
            FR_OK = 0,			/* 0 */
            FR_DISK_ERR,		/* 1 */
            FR_NOT_READY,		/* 2 */
            FR_NO_FILE,			/* 3 */
            FR_NO_PATH,			/* 4 */
            FR_NOT_OPENED,		/* 5 */
            FR_NOT_ENABLED,		/* 6 */
            FR_NO_FILESYSTEM	/* 7 */
        } FRESULT;

/**
 * Directory object structure (DIR)
 */

        struct DIR{
            ::uint16_t	index;			/* Current read/write index number */
            ::uint32_t	sclust;			/* Table start cluster (0:Root dir) */
            ::uint32_t	clust;			/* Current cluster */
            ::uint32_t	sect;			/* Current sector */
            block_data_t*	fn;				/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
        };

/**
 * Initialize the File System Object (Mount)
 */

        int init (
            Debug& debug,
            BlockMemory& block_memory
        )
        {
            bm_ = &block_memory;
            debug_ = &debug;

            return mount();
        }

/**
 * Open or Create a File
 */

        FRESULT open (
            const char* path	/* Pointer to the file name */
        )
        {
            FRESULT res;
            DIR dj;
            block_data_t sfn[12];
            block_data_t *dir;
            block_data_t buf[bm_->BLOCK_SIZE];

            if (!fs) {						/* Check file system */
                return FR_NOT_ENABLED;
            }

            fs->flag = 0;
            dj.fn = sfn;
            res = follow_path(&dj,buf, dir, path);	/* Follow the file path */
            dir = buf+(((::uint16_t)dj.index % (bm_->BLOCK_SIZE/SZ_DIR)) * SZ_DIR);

            if (res != FR_OK)   {                   	    /* Follow failed */
                if (res == FR_NO_FILE) {                    /* No file, create new */
                    res = dir_register(&dj, buf, dir);
                    // This writes entry to current CLuster. Handle the case where Cluster is full, new cluster needed for directory entry. [later]
                    bm_->write(buf, dj.sect);
                }
            }
            else {                                          /* Found some object */
                if (!dir[0] || (dir[DIR_Attr] & AM_DIR)) {	/* It is a directory */
                    return FR_NO_FILE;
                }
            }
            fs->dir_sect = dj.sect;
            fs->dir_index = dj.index;

            fs->org_clust = LD_CLUST(dir);			/* File start cluster */
            fs->fsize = LD_uint32_t(dir+DIR_FileSize);	/* File size */
            fs->fptr = 0;						/* File pointer */
            fs->flag = FA_OPENED;

            return res;
        }

/**
 * Read a File
 */

        FRESULT read (
            void* buff,		/* Pointer to the read buffer (NULL:Forward data to the stream)*/
            ::uint16_t btr,		/* Number of bytes to read */
            ::uint16_t* br		/* Pointer to number of bytes read */
        )
        {
            DRESULT dr;
            CLUST clst;
            ::uint32_t sect, remain;
            ::uint16_t rcnt;
            block_data_t cs, *rbuff = (block_data_t*)buff;

            *br = 0;
            if (!fs)    {                           /* Check file system */
                return FR_NOT_ENABLED;
            }

            if (!(fs->flag & FA_OPENED))    {       /* Check if opened */
                return FR_NOT_OPENED;
            }

            remain = fs->fsize - fs->fptr;
            if (btr > remain)   {
                btr = (::uint16_t)remain;			/* Truncate btr by remaining bytes */
            }

            while (btr)	{									/* Repeat until all data transferred */
                if ((fs->fptr % bm_->BLOCK_SIZE) == 0) {				/* On the sector boundary? */
                    cs = (block_data_t)(fs->fptr / bm_->BLOCK_SIZE & (fs->csize - 1));	/* Sector offset in the cluster */
                    if (!cs) {								/* On the cluster boundary? */
                        clst = (fs->fptr == 0) ?			/* On the top of the file? */
                            fs->org_clust : get_fat(fs->curr_clust);
                        if (clst <= 1)  {
                            goto fr_abort;
                        }
                        fs->curr_clust = clst;				/* Update current cluster */
                    }
                    sect = clust2sect(fs->curr_clust);		/* Get current sector */
                    if (!sect)  {
                        goto fr_abort;
                    }
                    fs->dsect = sect + cs;
                }
                rcnt = (::uint16_t)(bm_->BLOCK_SIZE - (fs->fptr % bm_->BLOCK_SIZE));		/* Get partial sector data from sector buffer */
                if (rcnt > btr) {
                    rcnt = btr;
                }
                block_data_t buffer[bm_->BLOCK_SIZE];
                bm_->read(buffer, fs->dsect);
                for(int x=0; x<rcnt; x++) {
                    rbuff[x] = buffer[x+(::uint16_t)(fs->fptr % bm_->BLOCK_SIZE)];
                }
                fs->fptr += rcnt; rbuff += rcnt;			/* Update pointers and counters */
                btr -= rcnt; *br += rcnt;
            }
            return FR_OK;

        fr_abort:
            fs->flag = 0;
            return FR_DISK_ERR;
        }

/**
 * Write File
 */

        FRESULT write (
            void* buff,	        /* Pointer to the data to be written */
            ::uint16_t btw,			/* Number of bytes to write (0:Finalize the current write operation) */
            ::uint16_t* bw			/* Pointer to number of bytes written */
        )
        {
            CLUST clst;
            ::uint32_t sect;
            block_data_t *p = (block_data_t*)buff;
            block_data_t cs;
            ::uint16_t wcnt,cc;
            block_data_t readBuff[bm_->BLOCK_SIZE];

            *bw = 0;
            if (!fs)    {
                return FR_NOT_ENABLED;		/* Check file system */
            }
            if (!(fs->flag & FA_OPENED))    {
                return FR_NOT_OPENED;		/* Check if opened */
            }
            wcnt = (bm_->BLOCK_SIZE * fs->csize) - (fs->fptr % (bm_->BLOCK_SIZE * fs->csize));

            if((wcnt>0 && btw > 0) && fs->fsize != 0)   {      /* Write to existing cluster */
                wcnt = bm_->BLOCK_SIZE - ((::uint16_t)fs->fptr % bm_->BLOCK_SIZE);		/* Number of bytes to write to the current sector */
                if (wcnt > btw) {
                    wcnt = btw;
                }
                bm_->read(readBuff, fs->dsect);
                memcpy(readBuff+(fs->fptr % bm_->BLOCK_SIZE), p, wcnt);
                bm_->write(readBuff, fs->dsect);
                fs->fptr += wcnt; p += wcnt;				/* Update pointers and counters */
                btw -= wcnt; *bw += wcnt;
            }
            while (btw)	{									/* Repeat until all data transferred */
                if (((::uint16_t)fs->fptr % bm_->BLOCK_SIZE) == 0) {			/* On the sector boundary? */
                    cs = (block_data_t)(fs->fptr / bm_->BLOCK_SIZE & (fs->csize - 1));	/* Sector offset in the cluster */
                    if (!cs) {								/* On the cluster boundary? */
                            clst = fs->org_clust;           /* Follow from the origin */
                            if(clst == 0)   {               /* When no cluster is allocated, */
                                clst = create_chain(0);     /* Create a new cluster chain */
                            }
                            else {                          /* Middle or end of the file */
                //Assuming lseek() brought curr_clust to appropriate clst, DO VERIFY [later]
                                clst = create_chain(fs->curr_clust);    /* Follow or stretch cluster
 chain on the FAT */
                            }
                        if (clst == 0)  {
                            break;		/* Could not allocate a new cluster (disk full) */
                        }
                        if (clst == 1)  {
                            goto fw_abort;
                        }
                        if (clst == 0xFFFFFFFF) {
                            goto fw_abort;
                        }
                        fs->curr_clust = clst;				/* Update current cluster */
                        if(fs->org_clust==0)    {
                            fs->org_clust = clst;	        /* Set start cluster if the first write */
                        }
                    }
                    sect = clust2sect(fs->curr_clust);		/* Get current sector */
                    if (!sect)  {
                        goto fw_abort;
                    }
                    sect = sect + cs;

                    if(btw<bm_->BLOCK_SIZE) {          /* Write ending partial sector */
                        wcnt = btw;
                        bm_->read(readBuff, sect);
                        memcpy(readBuff, p, btw);
                        bm_->write(readBuff, sect);
                        fs->fptr += wcnt; p += wcnt;	/* Update pointers and counters */
                        btw -= wcnt; *bw += wcnt;
                        fs->dsect = sect;
                        continue;
                    }
                    cc = btw/bm_->BLOCK_SIZE;
                    if(cc)  {                   /* Write maximum contiguous sectors directly */
                        if (cs + cc > fs->csize)	/* Clip at cluster boundary */
                            cc = fs->csize - cs;
                        bm_->write(p, sect, cc);
                        wcnt = bm_->BLOCK_SIZE * cc;		/* Number of bytes transferred */
                        fs->fptr += wcnt; p += wcnt;				/* Update pointers and counters */
                        btw -= wcnt; *bw += wcnt;
                        fs->dsect = sect;
                        continue;
                    }
                    fs->flag |= FA__WIP;
                }
            }
            if (fs->fptr > fs->fsize)   {
                fs->fsize = fs->fptr;	/* Update file size if needed */
                file_sync();
            }
            fs->flag &= ~FA__WIP;

            return FR_OK;

        fw_abort:
            fs->flag = 0;
            return FR_DISK_ERR;
        }

/**
 * Delete a File or Directory
 */

        FRESULT erase_obj (
            const char* path		/* Pointer to the file or directory path */
        )
        {
            FRESULT res;
            DIR dj, sdj;
            block_data_t *dir;
            ::uint32_t dclst;
            block_data_t sfn[12];
            block_data_t buf[bm_->BLOCK_SIZE];

            dj.fn = sfn;
            res = follow_path(&dj, buf, dir, path);		/* Follow the file path */
            if (res == FR_OK) {					/* The object is accessible */
                dir = buf + ((sdj.index % (bm_->BLOCK_SIZE / SZ_DIR)) * SZ_DIR);
                if (!dir) {
                    res = FR_NO_FILE;		/* Cannot remove the start directory */
                } else {
                    if (dir[DIR_Attr] & AM_RDO) {
                        res = FR_NOT_ENABLED;		/* Cannot remove R/O object */
                    }
                }
                dclst = LD_CLUST(dir);
                if (res == FR_OK && (dir[DIR_Attr] & AM_DIR)) {	/* Is it a sub-dir? */
                    if (dclst < 2) {
                        res = FR_DISK_ERR;
                    } else {
                        memcpy(&sdj, &dj, sizeof (DIR));	/* Check if the sub-directory is empty or not */
                        sdj.sclust = dclst;
                        res = dir_sdi(&sdj, buf, dir, 2);		/* Exclude dot entries */
                        dir = buf + ((sdj.index % (bm_->BLOCK_SIZE / SZ_DIR)) * SZ_DIR);
                        res = dir_read(&sdj, buf, dir);	/* Read an item */
                        if (res == FR_OK)   {
                            res = FR_NOT_ENABLED;        /* Not empty directory */
                        }
                        if (res == FR_NO_FILE)  {
                            res = FR_OK;	        /* Empty */
                        }
                    }
                }
                if (res == FR_OK) {
                    res = dir_remove(buf, dir, &dj);		/* Remove the directory entry */
                    if (res == FR_OK) {
                        if (dclst)  {                       /* Remove the cluster chain if exist */
                            res = remove_chain(dclst);
                        }
                        if (res == FR_OK)   {
                            res = sync_fs();
                        }
                    }
                }
            }
            return res;
        }

/**
 * Seek File R/W Pointer
 */

        FRESULT lseek (
            ::uint32_t ofs		/* File pointer from top of file */
        )
        {
            CLUST clst;
            ::uint32_t bcs, sect, ifptr;

            if (!fs)    {
                return FR_NOT_ENABLED;		/* Check file system */
            }
            if (!(fs->flag & FA_OPENED))    {		/* Check if opened */
                return FR_NOT_OPENED;
            }

            if (ofs > fs->fsize)    {
                ofs = fs->fsize;	/* Clip offset with the file size */
            }
            ifptr = fs->fptr;
            fs->fptr = 0;
            if (ofs > 0) {
                bcs = (::uint32_t)fs->csize * bm_->BLOCK_SIZE;	/* Cluster size (byte) */
                if (ifptr > 0 && (ofs - 1) / bcs >= (ifptr - 1) / bcs) {	/* When seek to same or following cluster, */
                    fs->fptr = (ifptr - 1) & ~(bcs - 1);	/* start from the current cluster */
                    ofs -= fs->fptr;
                    clst = fs->curr_clust;
                }
                else {							/* When seek to back cluster, */
                    clst = fs->org_clust;			/* start from the first cluster */
                    fs->curr_clust = clst;
                }
                while (ofs > bcs) {				/* Cluster following loop */
                    clst = get_fat(clst);		/* Follow cluster chain */
                    if (clst <= 1 || clst >= fs->n_fatent)  {
                        goto fe_abort;
                    }
                    fs->curr_clust = clst;
                    fs->fptr += bcs;
                    ofs -= bcs;
                }
                fs->fptr += ofs;
                sect = clust2sect(clst);		/* Current sector */
                if (!sect)  {
                    goto fe_abort;
                }
                fs->dsect = sect + (fs->fptr / bm_->BLOCK_SIZE & (fs->csize - 1));
            }

            return FR_OK;

        fe_abort:
            fs->flag = 0;
            return FR_DISK_ERR;
        }

/**
 * Create a Directory
 */

        FRESULT mkdir (
            const char* path		/* Pointer to the directory path */
        )
        {
            FRESULT res;
            DIR dj;
            block_data_t *dir;
            ::uint32_t dsc, dcl, pcl;
            block_data_t sfn[12], buf[bm_->BLOCK_SIZE];


            dj.fn = sfn;
            res = follow_path(&dj, buf, dir, path); 	/* Follow the file path */
            if (res == FR_OK)   {
                res = FR_NO_PATH;
            }
            if (res == FR_NO_FILE && (dj.fn[NS] & NS_DOT))  {
                res = FR_NO_PATH;
            }
            if (res == FR_NO_FILE) {				/* Can create a new directory */
                dcl = create_chain(0);      		/* Allocate a cluster for the new directory table */
                res = FR_OK;
                if (dcl == 0)   {
                    res = FR_NO_FILE;
                }
                if (dcl == 1)   {
                    res = FR_NO_FILE;
                }
                if (dcl == 0xFFFFFFFF)  {
                    res = FR_DISK_ERR;
                }
                if (res == FR_OK) {					/* Initialize the new directory table */
                    dsc = clust2sect(dcl);
                    dir = buf;
                    memset(dir, 0, bm_->BLOCK_SIZE);
                    memset(dir+DIR_Name, ' ', 11);	/* Create "." entry */
                    dir[DIR_Name] = '.';
                    dir[DIR_Attr] = AM_DIR;
                    st_clust(dir, dcl);
                    memcpy(dir+SZ_DIR, dir, SZ_DIR); 	/* Create ".." entry */
                    dir[SZ_DIR+1] = '.';
                    pcl = dj.sclust;
                    if (fs->fs_type == FS_FAT32 && pcl == fs->dirbase)    {
                        pcl = 0;
                    }
                    st_clust(dir+SZ_DIR, pcl);
                    bm_->write(buf, dsc);
                }
                if (res == FR_OK)   {
                    res = dir_register(&dj, buf, dir);	/* Register the object to the directoy */
                }
                if (res != FR_OK) {
                    remove_chain(dcl);			        /* Could not register, remove cluster chain */
                } else {
                    dir = buf+((::uint16_t)((dj.index % (bm_->BLOCK_SIZE / SZ_DIR)) * SZ_DIR));
                    dir[DIR_Attr] = AM_DIR;				/* Attribute */
//                    ST_uint32_t(dir+DIR_WrtTime, tm);		/* Created time */
                    st_clust(dir, dcl);					/* Table start cluster */
                    bm_->write(buf, dj.sect);
                }
            }
            return res;
        }

    private:

/**
 * File attribute bits for directory entry
 */
        enum {
            AM_RDO	=	0x01,	/*	Read	only	*/
            AM_HID	=	0x02,	/*	Hidden	*/
            AM_SYS	=	0x04,	/*	System	*/
            AM_VOL	=	0x08,	/*	Volume	label	*/
            AM_DIR	=	0x10,	/*	Directory	*/
            AM_ARC	=	0x20,	/*	Archive	*/
            AM_MASK	=	0x3F	/*	Mask	of	defined	bits	*/
        };

/**
 * FAT sub type (FATFS.fs_type)
 */
        enum {
            FS_FAT12	=	1,
            FS_FAT16	=	2,
            FS_FAT32	=	3
        };

/**
 * File status flag (FATFS.flag)
 */
        enum {
            FA_OPENED	=	0x01,
            FA_WPRT	    =	0x02,
            FA__WIP	    =	0x40
        };

/**
 * Name status flags
 */
        enum {
            NS          =   11,		/* Index of name status byte in fn[] */
            NS_LOSS		=   0x01,	/* Out of 8.3 format */
            NS_LFN		=   0x02,	/* Force to create LFN entry */
            NS_LAST		=   0x04,	/* Last segment */
            NS_BODY		=   0x08,	/* Lower case flag (body) */
            NS_EXT		=   0x10,	/* Lower case flag (ext) */
            NS_DOT		=   0x20	/* Dot entry */
        };

/**
 * FatFs refers the members in the FAT structures with byte offset instead
 * of structure member because there are incompatibility of the packing option
 * between various compilers.
 */
        enum {
            BS_jmpBoot	    =	0,
            BS_OEMName	    =	3,
            BPB_BytsPerSec	=	11,
            BPB_SecPerClus	=	13,
            BPB_RsvdSecCnt	=	14,
            BPB_NumFATs	    =	16,
            BPB_RootEntCnt	=	17,
            BPB_TotSec16	=	19,
            BPB_Media	    =	21,
            BPB_FATSz16	    =	22,
            BPB_SecPerTrk	=	24,
            BPB_NumHeads	=	26,
            BPB_HiddSec	    =	28,
            BPB_TotSec32	=	32,
            BS_55AA	        =	510,
            BS_DrvNum	    =	36,
            BS_BootSig	    =	38,
            BS_VolID	    =	39,
            BS_VolLab	    =	43,
            BS_FilSysType	=	54,
            BPB_FATSz32	    =	36,
            BPB_ExtFlags	=	40,
            BPB_FSVer	    =	42,
            BPB_RootClus	=	44,
            BPB_FSInfo	    =	48,
            BPB_BkBootSec	=	50,
            BS_DrvNum32	    =	64,
            BS_BootSig32	=	66,
            BS_VolID32	    =	67,
            BS_VolLab32	    =	71,
            BS_FilSysType32	=	82,
            MBR_Table	    =	446,
            DIR_Name	    =	0,
            DIR_Attr	    =	11,
            DIR_NTres	    =	12,
            DIR_CrtTime	    =	14,
            DIR_CrtDate	    =	16,
            DIR_FstClusHI	=	20,
            DIR_WrtTime	    =	22,
            DIR_WrtDate	    =	24,
            DIR_FstClusLO	=	26,
            DIR_FileSize	=	28,
            SZ_DIR			=   32,		/* Size of a directory entry */
            LLE             =   0x40,	/* Last long entry flag in LDIR_Ord */
            DDE             =   0xE5,	/* Deleted directory entry mark in DIR_Name[0] */
            NDDE            =   0x05,	/* Replacement of the character collides with DDE */
            FSI_LeadSig     =   0,		/* FSI: Leading signature (4) */
            FSI_StrucSig    =   484,	/* FSI: Structure signature (4) */
            FSI_Free_Count	=   488,	/* FSI: Number of free clusters (4) */
            FSI_Nxt_Free    =   492		/* FSI: Last allocated cluster (4) */
        };

        typename Debug::self_pointer_t debug_;
        typename BlockMemory::self_pointer_t bm_;
        Fat<OsModel, BlockMemory, Debug> *fs;

/**
 * FAT Attributes
 */
        block_data_t	fs_type;	    /* FAT sub type */
        block_data_t	flag;		    /* File status flags */
        block_data_t	csize;		    /* Number of sectors per cluster */
        ::uint16_t	n_rootdir;	    /* Number of root directory entries (0 on FAT32) */
        CLUST	n_fatent;	    /* Number of FAT entries (= number of clusters + 2) */
        block_data_t	n_fats;		    /* Number of FAT copies (1 or 2) */
        ::uint32_t	fatbase;	    /* FAT start sector */
        ::uint32_t	dirbase;	    /* Root directory start sector (Cluster# on FAT32) */
        ::uint32_t	database;	    /* Data start sector */
        ::uint32_t	fptr;		    /* File R/W pointer */
        ::uint32_t	fsize;		    /* File size */
        CLUST	org_clust;	    /* File start cluster */
        CLUST	curr_clust;	    /* File current cluster */
        ::uint32_t	dsect;		    /* File current data sector */
        ::uint32_t	dir_sect;		/* Sector number containing the directory entry */
        ::uint16_t	dir_index;		/* Pointer to the directory entry in the win[] */
        ::uint32_t	last_clust;		/* Last allocated cluster */
        ::uint32_t	free_clust;		/* Number of free clusters */
        block_data_t    fsi_sect;       /* Sector in which FSInfo is stored */

        ::uint16_t LD_uint16_t(block_data_t* ptr) {
            return (::uint16_t)(((::uint16_t)*((block_data_t*)(ptr)+1)<<8)|(::uint16_t)*(block_data_t*)(ptr));
        }

        ::uint32_t LD_uint32_t(block_data_t* ptr) {
            return (::uint32_t)(((::uint32_t)*((block_data_t*)(ptr)+3)<<24)|((::uint32_t)*((block_data_t*)(ptr)+2)<<16)|((::uint16_t)*((block_data_t*)(ptr)+1)<<8)|*(block_data_t*)(ptr));
        }

        block_data_t* ST_uint16_t(block_data_t* ptr,::uint16_t val)  {
        	*(block_data_t*)(ptr)=(block_data_t)(val);
        	*((block_data_t*)(ptr)+1)=(block_data_t)((::uint16_t)(val)>>8);
        	return ptr;
        }

        block_data_t* ST_uint32_t(block_data_t* ptr,::uint32_t val)  {
        	*(block_data_t*)(ptr)=(block_data_t)(val);
        	*((block_data_t*)(ptr)+1)=(block_data_t)((::uint16_t)(val)>>8);
        	*((block_data_t*)(ptr)+2)=(block_data_t)((::uint32_t)(val)>>16);
        	*((block_data_t*)(ptr)+3)=(block_data_t)((::uint32_t)(val)>>24);
        	return ptr;
        }

        ::uint32_t LD_CLUST(block_data_t* dir)   {
            return (((::uint32_t)LD_uint16_t(dir+DIR_FstClusHI)<<16) | LD_uint16_t(dir+DIR_FstClusLO));
        }

        void st_clust (
            block_data_t* dir,	/* Pointer to the directory entry */
            ::uint32_t cl	/* Value to be set */
        )
        {
            ST_uint16_t(dir+DIR_FstClusLO, cl);
            ST_uint16_t(dir+DIR_FstClusHI, cl >> 16);
        }

/**
 * FAT handling - Stretch or Create a cluster chain
 */

        ::uint32_t create_chain (	/* 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster# */
            ::uint32_t clst			/* Cluster# to stretch. 0 means create a new chain. */
        )
        {
            ::uint32_t cs, ncl, scl;
            FRESULT res;

            if (clst == 0) {		/* Create a new chain */
                scl = fs->last_clust;			/* Get suggested start point */
                if (!scl || scl >= fs->n_fatent)    {
                    scl = 1;
                }
            }
            else {					/* Stretch the current chain */
                cs = get_fat(clst);			/* Check the cluster status */
                if (cs < 2) {
                    return 1;			/* Invalid value */
                }
                if (cs == 0xFFFFFFFF)   {
                    return cs;	/* A disk error occurred */
                }
                if (cs < fs->n_fatent)  {
                    return cs;	/* It is already followed by next cluster */
                }
                scl = clst;
            }

            ncl = scl;				/* Start cluster */
            for (;;) {
                ncl++;							/* Next cluster */
                if (ncl >= fs->n_fatent) {		/* Check wrap around */
                    ncl = 2;
                    if (ncl > scl)  {
                        return 0;	/* No free cluster */
                    }
                }
                cs = get_fat(ncl);			/* Get the cluster status */
                if (cs == 0)    {
                    break;				/* Found a free cluster */
                }
                if (cs == 0xFFFFFFFF || cs == 1)    {
                    return cs;      /* An error occurred */
                }
                if (ncl == scl) {
                    return 0;		/* No free cluster */
                }
            }

            res = put_fat(ncl, 0x0FFFFFFF);	/* Mark the new cluster "last link" */

            if(res == FR_OK)    {
                res = erase_cluster(ncl);   /* Erase contents of the cluster */
            }
            if (res == FR_OK && clst != 0) {
                res = put_fat(clst, ncl);	/* Link it to the previous one if needed */
            }
            if (res == FR_OK) {
                fs->last_clust = ncl;			/* Update FSINFO */
                if (fs->free_clust != 0xFFFFFFFF) {
                    fs->free_clust--;
                }
            } else {
                ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;
            }

            return ncl;		/* Return new cluster number or error code */
        }

/**
 * FAT handling - Remove a cluster chain
 */

        FRESULT remove_chain (
            ::uint32_t clst			/* Cluster# to remove a chain from */
        )
        {
            FRESULT res;
            ::uint32_t nxt;

            if (clst < 2 || clst >= fs->n_fatent) {	/* Check range */
                res = FR_DISK_ERR;

            } else {
                res = FR_OK;
                while (clst < fs->n_fatent) {			/* Not a last link? */
                    nxt = get_fat(clst);			    /* Get cluster status */
                    if (nxt == 0)   {
                        break;				/* Empty cluster? */
                    }
                    if (nxt == 1)   {
                        res = FR_DISK_ERR;   /* Internal error? */
                        break;
                    }
                    if (nxt == 0xFFFFFFFF)  {
                        res = FR_DISK_ERR;  /* Disk error? */
                        break;
                    }
                    res = put_fat(clst, 0);			/* Mark the cluster "empty" */
                    if (res != FR_OK)   {
                        break;
                    }
                    if (fs->free_clust != 0xFFFFFFFF) {	/* Update FSINFO */
                        fs->free_clust++;
                    }
                    clst = nxt;	/* Next cluster */
                }
            }

            return res;
        }

/**
 * Erase all sectors belonging to the cluster
 */

        FRESULT erase_cluster (
            CLUST clst          /* Cluster to be erased */
        )
        {
            FRESULT res;
            res = FR_OK;
            block_data_t buf[bm_->BLOCK_SIZE];
            memset(buf, 0, sizeof(buf));
            ::uint32_t sect;
            sect = clust2sect(clst);
            bm_->write(buf,sect, fs->csize);

            return res;
        }

/**
 * FAT access - Read value of a FAT entry
 */

// Test get_fat() with FAT12 and FAT16 [later]

        CLUST get_fat (	/* 1:IO error, Else:Cluster status */
            CLUST clst	/* Cluster# to get the link information */
        )
        {
            ::uint16_t wc, bc, ofs;
            block_data_t buf[bm_->BLOCK_SIZE];

            if (clst < 2 || clst >= fs->n_fatent)   {	/* Range check */
                return 1;
            }
            switch (fs->fs_type) {
            case FS_FAT12 :
                bc = (::uint16_t)clst;
                bc += bc / 2;
                ofs = bc % bm_->BLOCK_SIZE; bc /= bm_->BLOCK_SIZE;
                if (ofs != bm_->BLOCK_SIZE-1) {
                    bm_->read(buf, fs->fatbase + bc);
                    wc = LD_uint16_t(buf+ofs);
                }
                else {
                    bm_->read(buf, fs->fatbase + bc);
                    block_data_t tmp = buf[bm_->BLOCK_SIZE-1];
                    bm_->read(buf, fs->fatbase + bc + 1);
                    buf[1] = tmp;
                    wc = LD_uint16_t(buf);
                }

                return (clst & 1) ? (wc >> 4) : (wc & 0xFFF);
            case FS_FAT16 :
                bm_->read(buf, fs->fatbase + clst / 256);
                return LD_uint16_t(buf+(::uint16_t)(((::uint16_t)clst % 256) * 2));
            case FS_FAT32 :
                bm_->read(buf, fs->fatbase + clst / 128);
                return LD_uint32_t(buf+(::uint16_t)(((::uint16_t)clst % 128) * 4)) & 0x0FFFFFFF;
            }

            return 1;	/* An error occured at the disk I/O layer */
        }

/**
 * FAT access - Change value of a FAT entry
 */

// Test put_fat() with FAT12 and FAT16

        FRESULT put_fat (
            ::uint32_t clst,	/* Cluster# to be changed in range of 2 to fs->n_fatent - 1 */
            ::uint32_t val	/* New value to mark the cluster */
        )
        {
            unsigned int bc;
            unsigned int i;
            ::uint32_t fat_copy_sect;
            block_data_t *p;
            FRESULT res;
            block_data_t buf[bm_->BLOCK_SIZE];

            if (clst < 2 || clst >= fs->n_fatent) {	/* Check range */
                res = FR_NO_FILE;
            } else {
                i = 0;
                while (i<fs->n_fats)    {
                    switch (fs->fs_type) {
                    case FS_FAT12 :
                        bc = (unsigned int)clst;
                        bc += bc / 2;
                        fat_copy_sect = fs->fatbase + (::uint32_t)(((fs->n_fatent + bm_->BLOCK_SIZE -1 ) / bm_->BLOCK_SIZE) * i);
                        if(bm_->read(buf, fat_copy_sect + (bc/bm_->BLOCK_SIZE))) {
                            break;
                        }
                        p = &buf[bc % bm_->BLOCK_SIZE];
                        *p = (clst & 1) ? ((*p & 0x0F) | ((block_data_t)val << 4)) : (block_data_t)val;
                        if(bm_->write(buf, fat_copy_sect + (bc/bm_->BLOCK_SIZE))) {
                            break;
                        }
                        bc++;
                        if(bm_->read(buf, fat_copy_sect + (bc/bm_->BLOCK_SIZE)))  {
                            break;
                        }
                        p = &buf[bc % bm_->BLOCK_SIZE];
                        *p = (clst & 1) ? (block_data_t)(val >> 4) : ((*p & 0xF0) | ((block_data_t)(val >> 8) & 0x0F));
                        if(bm_->write(buf, fat_copy_sect + (bc/bm_->BLOCK_SIZE))) {
                            break;
                        }
                        res = FR_OK;
                        break;

                    case FS_FAT16 :
                        fat_copy_sect = fs->fatbase + (::uint32_t)(((fs->n_fatent * 2 + bm_->BLOCK_SIZE -1 ) / bm_->BLOCK_SIZE) * i);
                        if(bm_->read(buf, fat_copy_sect + (clst / (bm_->BLOCK_SIZE / 2))))  {
                            break;
                        }
                        p = &buf[clst * 2 % bm_->BLOCK_SIZE];
                        ST_uint16_t(p, (::uint16_t)val);
                        if(bm_->write(buf, fat_copy_sect + (clst / (bm_->BLOCK_SIZE / 2)))) {
                            break;
                        }
                        res = FR_OK;
                        break;

                    case FS_FAT32 :
                        fat_copy_sect = fs->fatbase + (::uint32_t)(((fs->n_fatent * 4 + bm_->BLOCK_SIZE -1 ) / bm_->BLOCK_SIZE) * i);
                        if(bm_->read(buf, fat_copy_sect + (clst / (bm_->BLOCK_SIZE / 4))))  {
                            break;
                        }
                        p = &buf[clst * 4 % bm_->BLOCK_SIZE];
                        val |= LD_uint32_t(p) & 0xF0000000;
                        ST_uint32_t(p, val);
                        if(bm_->write(buf, fat_copy_sect + (clst / (bm_->BLOCK_SIZE / 4)))) {
                            break;
                        }
                        res = FR_OK;
                        break;

                    default :
                        res = FR_DISK_ERR;
                    }
                    i++;
                }
            }

            return res;
        }

/**
 * Get sector# from cluster#
 */

        ::uint32_t clust2sect (	/* !=0: Sector number, 0: Failed - invalid cluster# */
            CLUST clst		/* Cluster# to be converted */
        )
        {

            clst -= 2;
            if (clst >= (fs->n_fatent - 2)) {
                return 0;		/* Invalid cluster# */
            }
            return (::uint32_t)clst * fs->csize + fs->database;
        }

/**
 * Check a sector if it is an FAT boot record
 */

        block_data_t check_fs (	/* 0:The FAT boot record, 1:Valid boot record but not an FAT, 2:Not a boot record, 3:Error */
            block_data_t *buf,	/* Working buffer */
            ::uint32_t sect	/* Sector# (lba) to check if it is an FAT boot record or not */
        )
        {
            if(bm_->read(buf, sect)==SUCCESS) {
                if(buf[BS_55AA]!=0x55 || buf[BS_55AA+1]!=0xAA) {
                    return 2;
                }
                if(buf[BS_FilSysType]==0x46 && buf[BS_FilSysType+1]==0x41) {
                    return 0;
                }
                if(buf[BS_FilSysType32]==0x46 && buf[BS_FilSysType32+1]==0x41) {
                    return 0;
                }
            }
            else {
                return 3;
            }
            return 1;
        }

/**
 * Synchronize file system and strage device
 */

        FRESULT sync_fs (	/* FR_OK: successful, FR_DISK_ERR: failed */
        )
        {
            FRESULT res;
            block_data_t buf[bm_->BLOCK_SIZE];

            /* Update FSINFO sector if needed */
            if (fs->fs_type == FS_FAT32) {
                /* Create FSINFO structure */
                memset(buf, 0, bm_->BLOCK_SIZE);
                ST_uint16_t(buf+BS_55AA, 0xAA55);
                ST_uint32_t(buf+FSI_LeadSig, 0x41615252);
                ST_uint32_t(buf+FSI_StrucSig, 0x61417272);
                ST_uint32_t(buf+FSI_Free_Count, fs->free_clust);
                ST_uint32_t(buf+FSI_Nxt_Free, fs->last_clust);
                /* Write it into the FSINFO sector */
                bm_->write(buf, fs->fsi_sect);
            }
            return res;
        }

/**
 * Synchronize the File
 */

        FRESULT file_sync ()
        {
            FRESULT res = FR_OK;
            block_data_t *dir;
            block_data_t buf[bm_->BLOCK_SIZE];

            /* Update the directory entry */
            bm_->read(buf, fs->dir_sect);
            dir = buf + (SZ_DIR * (fs->dir_index % (bm_->BLOCK_SIZE / SZ_DIR)));
            dir[DIR_Attr] |= AM_ARC;					/* Set archive bit */
            ST_uint32_t(dir+DIR_FileSize, fs->fsize);		/* Update file size */
            st_clust(dir, fs->org_clust);					/* Update start cluster */
            bm_->write(buf, fs->dir_sect);
            return res;
        }

/**
 * Directory handling - Reserve directory entry
 */

        FRESULT dir_alloc (
            DIR* dp,	/* Pointer to the directory object */
            block_data_t nent,	/* Number of contiguous entries to allocate (1-21) */
            block_data_t *dir,  /* Store info of new directory */
            block_data_t *buf   /* Buffer containing sector read */
        )
        {
            FRESULT res;
            block_data_t n;

            res = dir_sdi(dp, buf, dir, 0);
            dir = buf+((::uint16_t)((dp->index % (bm_->BLOCK_SIZE / SZ_DIR)) * SZ_DIR));
            if (res == FR_OK) {
                n = 0;
                do {
                    bm_->read(buf, dp->sect);

                    if (dir[0] == DDE || dir[0] == 0) {	/* Is it a blank/deleted dir entry (DDE)? */
                        if (++n == nent)    {
                            break;	/* A block of contiguous entries is found */
                        }
                    } else {
                        n = 0;					/* Not a blank entry. Restart to search */
                    }
                    res = dir_next(dp); 		/* Next entry with table stretch enabled */
                    dir = buf+(((::uint16_t)dp->index % (bm_->BLOCK_SIZE/SZ_DIR)) * SZ_DIR);
                } while (res == FR_OK);
            }

            /* check this out, replcase FR_NOT_ENABLED */
            return res;
        }

/**
 * Directory handling - Set directory index
 */

        FRESULT dir_sdi (
            DIR* dp,		/* Pointer to directory object */
            block_data_t *dir,      /* Store info of new directory */
            block_data_t *buf,      /* Buffer containing sector read */
            unsigned int idx		/* Index of directory table */
        )
        {
            ::uint32_t clst, sect;
            unsigned int ic;

            dp->index = (::uint16_t)idx;	/* Current index */
            clst = dp->sclust;		/* Table start cluster (0:root) */
            if (clst == 1 || clst >= fs->n_fatent)  {
                return FR_DISK_ERR;  /* Check start cluster range */
            }
            if (!clst && fs->fs_type == FS_FAT32)   {
                clst = fs->dirbase;     /* Replace cluster# 0 with root cluster# if in FAT32 */
            }
            if (clst == 0) {	/* Static table (root-directory in FAT12/16) */
                if (idx >= fs->n_rootdir)   {
                    return FR_DISK_ERR;  /* Is index out of range? */
                }
                sect = fs->dirbase;
            }
            else {				/* Dynamic table (root-directory in FAT32 or sub-directory) */
                ic = bm_->BLOCK_SIZE / SZ_DIR * fs->csize;	/* Entries per cluster */
                while (idx >= ic) {	/* Follow cluster chain */
                    clst = get_fat(clst);				/* Get next cluster */
                    if (clst == 0xFFFFFFFF) {
                        return FR_DISK_ERR;	/* Disk error */
                    }
                    if (clst < 2 || clst >= fs->n_fatent)   {
                        return FR_DISK_ERR;  /* Reached to end of table or internal error */
                    }
                    idx -= ic;
                }
                sect = clust2sect(clst);
            }
            dp->clust = clst;	/* Current cluster# */
            if (!sect)  {
                return FR_DISK_ERR;
            }
            dp->sect = sect + idx / (bm_->BLOCK_SIZE / SZ_DIR);					/* Sector# of the directory entry */
            dir = buf + (idx % (bm_->BLOCK_SIZE / SZ_DIR)) * SZ_DIR;	/* Ptr to the entry in the sector */

            return FR_OK;
        }

/**
 * Directory handling - Move directory index next
 */

        FRESULT dir_next (	/* FR_OK:Succeeded, FR_NO_FILE:End of table */
            DIR *dj			/* Pointer to directory object */
        )
        {
            CLUST clst;
            ::uint16_t i;

            i = dj->index + 1;
            if (!i || !dj->sect)    {	/* Report EOT when index has reached 65535 */
                return FR_NO_FILE;
            }

            if (!(i % (bm_->BLOCK_SIZE / SZ_DIR))) {		/* Sector changed? */
                dj->sect++;			/* Next sector */

                if (dj->clust == 0) {	/* Static table */
                    if (i >= fs->n_rootdir)	/* Report EOT when end of table */
                        return FR_NO_FILE;
                }
                else {					/* Dynamic table */
                    if (((i / (bm_->BLOCK_SIZE / SZ_DIR)) & (fs->csize-1)) == 0) {	/* Cluster changed? */
                        clst = get_fat(dj->clust);		/* Get next cluster */
                        if (clst <= 1)  {
                            return FR_DISK_ERR;
                        }
                        if (clst == 0xFFFFFFFF) {
                            return FR_DISK_ERR;
                        }
                        if (clst >= fs->n_fatent)   {	/* When it reached end of dynamic table */
                            clst = create_chain(dj->clust);
                            if (clst == 0)  {
                                return FR_NO_FILE;
                            }
                            if (clst == 1)  {
                                return FR_NO_FILE;
                            }
                            if (clst == 0xFFFFFFFF) {
                                return FR_DISK_ERR;
                            }
                        }
                        dj->clust = clst;				/* Initialize data for new cluster */
                        dj->sect = clust2sect(clst);
                    }
                }
            }

            dj->index = i;

            return FR_OK;
        }

/**
 * Directory handling - Find an object in the directory
 */

        FRESULT dir_find (
            DIR *dj,		/* Pointer to the directory object linked to the file name */
            block_data_t *buf,      /* Buffer to read sector */
            block_data_t *dir		/* 32-byte working buffer */
        )
        {
            FRESULT res;
            block_data_t c;
            res = dir_sdi(dj, buf, dir, 0);			/* Rewind directory object */
            if (res != FR_OK)   {
                return res;
            }
            do {
                if(bm_->read(buf, dj->sect)==SUCCESS) {
                    dir = buf+(((::uint16_t)dj->index % (bm_->BLOCK_SIZE/SZ_DIR)) * SZ_DIR);
                }
                else    {
                    break;
                }
                c = dir[DIR_Name];	/* First character */
                if (c == 0) {
                    res = FR_NO_FILE;
                    break;
                }	                /* Reached to end of table */
                if (!(dir[DIR_Attr] & AM_VOL) && !memcmp(dir, dj->fn, 11)) { /* Is it a valid entry? */
                    break;
                }
                res = dir_next(dj);					/* Next entry */
            } while (res == FR_OK);
            return res;
        }

/**
 * Register an object to the directory
 */

        FRESULT dir_register (	/* FR_OK:Successful, FR_NOT_ENABLED:No free entry, FR_DISK_ERR:Disk error */
            DIR* dp,			/* Target directory with object name to be created */
            block_data_t* buf,          /* Buffer to read sector */
            block_data_t* dir           /* Store directory entry */
        )
        {
            FRESULT res;
            res = dir_alloc(dp, 1, dir, buf);		/* Allocate an entry for SFN */

            dir = buf+((::uint16_t)((dp->index % (bm_->BLOCK_SIZE / SZ_DIR)) * SZ_DIR));   // Check if needed? [later]

            if (res == FR_OK) {				    /* Set SFN entry */
                bm_->read(buf, dp->sect);
                if (res == FR_OK) {
                    memset(dir, 0, SZ_DIR);	    /* Clean the entry */
                    memcpy(dir, dp->fn, 11);	/* Put SFN */
                }
            }

            return res;
        }

/**
 * Read an object from the directory
 */

        FRESULT dir_read (
            DIR* dp,		/* Pointer to the directory object */
            block_data_t* buf,      /* Buffer to read sector */
            block_data_t* dir       /* Store directory entry */
        )
        {
            FRESULT res;
            block_data_t a, c;

            res = FR_NO_FILE;
            while (dp->sect) {
                bm_->read(buf, dp->sect);
                dir = buf+(((::uint16_t)dp->index % (bm_->BLOCK_SIZE/SZ_DIR)) * SZ_DIR);  /* Ptr to the dir entry of current index */
                c = dir[DIR_Name];
                if (c == 0) {       /* Reached to end of table */
                    res = FR_NO_FILE;
                    break;
                }
                a = dir[DIR_Attr] & AM_MASK;
                if (c != DDE && (c != '.') && (a == AM_VOL)) {   /* Is it a valid entry? */
                    break;
                }
                res = dir_next(dp);				/* Next entry */
                if (res != FR_OK) break;
            }

            if (res != FR_OK)   {
                dp->sect = 0;
            }

            return res;
        }

/**
 * Remove an object from the directory
 */

        FRESULT dir_remove (	/* FR_OK: Successful, FR_DISK_ERR: A disk error */
            block_data_t* buf,          /* Buffer to read sector */
            block_data_t* dir,          /* Pointer to the directory entry */
            DIR* dp				/* Directory object pointing the entry to be removed */
        )
        {
            FRESULT res;
            res = dir_sdi(dp, buf, dir, dp->index);
            dir = buf+(((::uint16_t)dp->index % (bm_->BLOCK_SIZE/SZ_DIR)) * SZ_DIR);  /* Ptr to the dir entry of current index */
            if (res == FR_OK) {
                bm_->read(buf, dp->sect);
                memset(dir, 0, SZ_DIR);	/* Clear and mark the entry "deleted" */
                *dir = DDE;
                bm_->write(buf, dp->sect);
            }
            return res;
        }

/**
 * Pick a segment and create the object name in directory form
 */

        FRESULT create_name (
            DIR *dj,			/* Pointer to the directory object */
            const char **path	/* Pointer to pointer to the segment in the path string */
        )
        {
            block_data_t c, ni, si, i, *sfn;
            const char *p;

            /* Create file name in directory form */
            sfn = dj->fn;

            memset(sfn, ' ', 11);
            si = i = 0; ni = 8;
            p = *path;
            for (;;) {
                c = p[si++];
                if (c <= ' ' || c == '/')   {
                    break;	/* Break on end of segment */
                }
                if (c == '.' || i >= ni) {
                    if (ni != 8 || c != '.')    {
                        break;
                    }
                    i = 8;
                    ni = 11;
                    continue;
                }
                    if (is_lower(c))    {
                        c -= 0x20;	/* toupper */
                    }
                    sfn[i++] = c;
            }
            *path = &p[si];						/* Rerurn pointer to the next segment */
            sfn[11] = (c <= ' ') ? 1 : 0;		/* Set last segment flag if end of path */
            return FR_OK;
        }

/**
 * Follow a file path
 */

        FRESULT follow_path (	/* FR_OK(0): successful, !=0: error code */
            DIR *dj,			/* Directory object to return last directory and found object */
            block_data_t *buf,          /* Buffer to read sector */
            block_data_t *dir,			/* SZ_DIR-byte working buffer */
            const char *path	/* Full-path string to find a file or directory */
        )
        {
            FRESULT res;

            while (*path == ' ')    {
                path++;		/* Skip leading spaces */
            }
            if (*path == '/')   {
                path++;			/* Strip heading separator */
            }
            dj->sclust = 0;						/* Set start directory (always root dir) */
            dir = buf+(((::uint16_t)0 % (bm_->BLOCK_SIZE/SZ_DIR)) * SZ_DIR);      // May want to get rid of this [later]

            if ((block_data_t)*path <= ' ') {			/* Null path means the root directory */
                res = dir_sdi(dj, buf, dir, 0);
                dir[0] = 0;
            }
            else {							            /* Follow path */
                for (;;) {
                    res = create_name(dj, &path);   	/* Get a segment */
                    if (res != FR_OK)   {
                        break;
                    }
                    res = dir_find(dj, buf, dir);		/* Find it */
                    dir = buf+(((::uint16_t)dj->index % (bm_->BLOCK_SIZE/SZ_DIR)) * SZ_DIR);
                    if (res != FR_OK) {				    /* Could not find the object */
                        if (res == FR_NO_FILE && !*(dj->fn+11)) {
                            res = FR_NO_PATH;
                        }
                        break;
                    }

                    if (*(dj->fn+11))   {
                        break;	    	/* Last segment match. Function completed. */
                    }

                    if (!(dir[DIR_Attr] & AM_DIR)) {    /* Cannot follow because it is a file */
                        res = FR_NO_PATH;
                        break;
                    }
                    dj->sclust = LD_CLUST(dir);
                }
            }

            return res;
        }

/**
 * Mount/Unmount a Locical Drive
 */

        FRESULT mount ()
        {
            block_data_t fmt, buf[bm_->BLOCK_SIZE];
            ::uint32_t bsect, fsize, tsect, mclst;

            fs = 0;
            if (!this)  {
                return FR_OK;				/* Unregister fs object */
            }

            /* Search FAT partition on the drive */
            bsect = 0;
            fmt = check_fs(buf, bsect);			/* Check sector 0 as an SFD format */
            if (fmt == 1) {						/* Not an FAT boot record, it may be FDISK format */
                /* Check a partition listed in top of the partition table */
                if (buf[446+4]) {					/* Is the partition existing? */
                    bsect = LD_uint32_t(&buf[446+8]);	/* Partition offset in LBA */
                    fmt = check_fs(buf, bsect);	/* Check the partition */
                }
            }
            if (fmt == 3)   {
                return FR_DISK_ERR;
            }
            if (fmt)    {
                return FR_NO_FILESYSTEM;	/* No valid FAT patition is found */
            }

            /* Initialize the file system object */
            fsize = LD_uint16_t(buf+BPB_FATSz16);				/* Number of sectors per FAT */
            if (!fsize) {
                fsize = LD_uint32_t(buf+BPB_FATSz32);
            }
            this->n_fats = buf[BPB_NumFATs];
            fsize *= buf[BPB_NumFATs];						/* Number of sectors in FAT area */
            this->fatbase = bsect + LD_uint16_t(buf+BPB_RsvdSecCnt); /* FAT start sector (lba) */
            this->csize = buf[BPB_SecPerClus];					/* Number of sectors per cluster */
            this->n_rootdir = LD_uint16_t(buf+BPB_RootEntCnt);		/* Number of root directory entries, FAT32(0) */
            tsect = LD_uint16_t(buf+BPB_TotSec16);				/* Number of sectors on the file system */
            if (!tsect) {
                tsect = LD_uint32_t(buf+BPB_TotSec32);
            }
            mclst = (tsect						/* Last cluster# + 1, mclst holds total number of clusters */
                - LD_uint16_t(buf+BPB_RsvdSecCnt) - fsize - this->n_rootdir / 16
                ) / this->csize + 2;
            this->n_fatent = (CLUST)mclst;
            fmt = FS_FAT32;							        // Forcing FAT32 [later]

            this->fs_type = fmt;		/* FAT sub-type */

            if (_FS_FAT32 && fmt == FS_FAT32)   {
                this->dirbase = LD_uint32_t(buf+(BPB_RootClus));	/* Root directory start cluster */
            }
            else    {
                this->dirbase = this->fatbase + fsize;          /* Root directory start sector (lba) */
            }
            this->database = this->fatbase + fsize + this->n_rootdir / 16;	/* Data start sector (lba) */
            this->flag = 0;

            this->last_clust = this->free_clust = 0xFFFFFFFF;
            this->fsi_sect = LD_uint32_t(buf+BPB_FSInfo);

            if(!this->fsi_sect && fmt==FS_FAT32) {
                bm_->read(buf, this->fsi_sect);
                if (LD_uint16_t(buf+BS_55AA) == 0xAA55	/* Load FSINFO data if available */
                && LD_uint32_t(buf+FSI_LeadSig) == 0x41615252
                && LD_uint32_t(buf+FSI_StrucSig) == 0x61417272) {
                    this->free_clust = LD_uint32_t(buf+FSI_Free_Count);
                    this->last_clust = LD_uint32_t(buf+FSI_Nxt_Free);
                }
            }

            fs = this;
            return FR_OK;
        }

};

}   // namespace wiselib
