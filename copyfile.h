/*************************************************************************
   FILENAME:	copyfile.h
   AUTHOR:	David Abrams    August 19, 1996
   MRU:		9-17-96 update for first beta release
                9-23-96 remove extraneous debugging code from GCConvert() 1.02
   DESC:	Defines for sample of Galactic file converter
		written in 'C'

		Compile with FLAT32=1 for 32-bit version

   Copyright (C) 1996 Galactic Industries Corp.  All Rights Reserved.
**************************************************************************/

/* Following defines set converter version in resource and as returned by */
/* GCGetInfo call.  They must be updated anytime the CN4 or CN5 convert   */
/* file changes!                                                          */

#define GCVHI 1			/* numeric version must track version */
#define GCVLO 02		/* string below                       */
#define GCVSV 0			/* leave as 0 - sub/sub version not used */

#ifdef FLAT32
#define GCVT "1.02D5\0"		/* IMPORTANT!  Add "D5" after 32-bit version */
#else
#define GCVT "1.02D4\0"		/* IMPORTANT!  Add "D4" after 16-bit version */
#endif

#define GVT "4.00"		// does not change as version changes

/* misc defines */

#define		MEMALLOC	4096L	// size of first allocated memory block
#define		RBUFSZ		4096	// local read file buffer size

#define 	MAXOF		32	// number of available open files

// defines for uFlag in FILESTC structure

#define		DISK	1
#define		MEMRD	2
#define		MEMWRT	3

// Structure used to hide memory/disk file I/O differences from converter
// One strucutre is used for the source file and one for the destination file

typedef struct 	{
		UINT	uFlag;		// 0=free, 1=file, 2=mem read, 3=mem wrt
		LPSTR 	lpFName;	// needed to delete disk file
#ifdef FLAT32
		HANDLE	hFile;		// disk file pointer (Win32)
#else
		HFILE	hFile;		// disk file pointer (Win16)
#endif
		HGLOBAL	hMem;		// memory file handle (or 0)
		DWORD	dFileLen;	// length of data in memory file
		DWORD	dAlloc;		// bytes allocated to memory file
		DWORD	dCur;		// current memory file pointer
		LPBYTE  lpMem;		// pointer to beginning of global memory
		} FILESTC, *pFILESTC;

/*          Function declarations for encapsulated file I/O        */
/*                                                                 */
/* By using these functions in place of the Win3.1 API equivalents */
/* the converter will handle source or destination files in memory */
/* or on disk with no additonal code in the coverter body.         */

int cnvOpen (lpGCCONVERTS);
int cnvCreate (lpGCCONVERTS);
HGLOBAL cnvClose (int);
VOID cnvDelete (int);
LONG cnvSeek (int, LONG, int);
UINT cnvRead (int, LPVOID, UINT);
UINT cnvWrite (int, LPVOID, UINT);

/* resource STRINGTABLE defines */

#define IDS_CNVDESC	100
#define IDS_DATAEXT	101
#define IDS_IMPPROMPT	102
#define IDS_EXPPROMPT	103

#define IDS_STRING104	104
#define IDS_STRING105	105
#define IDS_STRING106	106
#define IDS_STRING107	107
#define IDS_STRING108	108
#define IDS_STRING109	109
#define IDS_STRING110	110
#define IDS_STRING111	111
#define IDS_STRING112	112
#define IDS_STRING113	113
#define IDS_STRING114	114
#define IDS_STRING115	115
