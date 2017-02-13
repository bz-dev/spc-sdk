/****************************************************************************
®RM132¯
    PROGRAM: 	copyfile.c

    PURPOSE: 	Sample Galactic Converter *.CN4/CN5 DLL source code

    AUTHOR:   	David Abrams  August 30, 1996

    MRU:	V102 remove debugging code

    COMMENTS:	Compile with "FLAT32" defined for 32-bit version
		DEBUG defined if debug version.

		Note that all routines should be reentrant when compiled
		for FLAT32 so that multiple treads can convert several
		files simultanously.  The GCConvert() call uses a
		dynamically allocated buffer for the source file to
		allow multiple simultaneous conversions.

    FUNCTIONS:

        Exported Functions
	------------------
	GCGetInfo () 	- returns converter information
	GCCheckData()	- check data file to see if handled by this converter
	GCConvert()	- imports/exports foreign data file to Galactic data file

   Copyright (C) 1996 Galactic Industries Corp.  All Rights Reserved.
*******************************************************************************/

#include <windows.h>
#include <memory.h>
#include <stdio.h>

#include "gcdll.h"			// defines for converter dll's
#include "gcdllerr.h"			// error codes for converter dll's

#include "copyfile.h"			// defines resource ID's

/*********************************/
/*                               */
/*    Global Static Variables    */
/*                               */
/*********************************/

LPCSTR	crs = "Copyright (C) 1996 Galactic Industries Corp.  All Rights Reserved.";

HANDLE 	ghDLLInst;			// this DLL's instance handle

BOOL	bSInit   = FALSE;		// TRUE if sFileStc initialized
BOOL	bSLoaded = FALSE;		// TRUE if resource strings loaded
char	szDesc[MAXSTRING];		// storage for strings loaded
char	szExt[MAXSTRING];		// from resource
char	szImpPmt[MAXSTRING];
char	szExpPmt[MAXSTRING];

// In order to handle files on disk or in memory transparently, this 
// converter encapsulates all file handling into a set of subroutines
// which hide the type of file being accessed.  The following set of
// structures are used byt he file handling subroutines to track an open
// file.  Under 16-bit Windows a DLL has a single data segment for all
// callers of the DLL so multiple structures are declared allowing the
// DLL to be used by multiple callers.  These structures are used in pairs,
// one for the input file and one for the output file.  The constant MAXOF
// determines how many open files are allowed.  Under Win32 each DLL has
// its own data segment for each instance of the DLL so only two structures
// are required but multiple tasks may call into the converter DLL 
// simultaneously from the same program.

FILESTC	sFileStc[MAXOF];		// structures used to track open files

/*****************************************************************************/
/*****************************************************************************/
/*									     */
/*                       Exported Functions      			     */
/*									     */
/*****************************************************************************/
/*****************************************************************************/

/****************************************************************************
   FUNCTION: LibMain(HANDLE, WORD, WORD, LPSTR) or DllMain()

   PURPOSE:  The LibMain function should perform additional initialization
             tasks required by the DLL.  LibMain should return a value of 1 
	     if the initialization is successful.

*******************************************************************************/

#ifdef FLAT32

INT APIENTRY DllMain (HANDLE hModule, DWORD called, LPVOID lpReserved)
   {
   if (called == DLL_PROCESS_ATTACH)
      {
      if (!bSInit)
         {
         memset (sFileStc, 0,  MAXOF * (sizeof (FILESTC))); 
	 bSInit = TRUE;
	 }
      ghDLLInst = hModule;
      }
   return (TRUE);
   }

#else

int FAR PASCAL LibMain (HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
   {
   _fmemset (sFileStc, 0,  MAXOF * (sizeof (FILESTC)));  // init file structs
   ghDLLInst = hModule;
   return (TRUE);
   }

#endif


/****************************************************************************

    FUNCTION: GCGetInfo (lpGCGETINFOS)

    RETURNS:  Fills in GCGETINFOS structure and returns TRUE.  Used by
	      converter client to see what format converter supports and
	      converter limitations/requirements.  Returns FALSE if
	      resource strings cannot be loaded.

****************************************************************************/

DllExport BOOL WINAPI EXPORT GCGetInfo (lpGCGETINFOS GCGetInfoS)
   {
   if (!bSLoaded)	// only load strings once
      {
      if (LoadString (ghDLLInst, IDS_CNVDESC, szDesc, MAXSTRING) == 0)
         return (FALSE);
      if (LoadString (ghDLLInst, IDS_DATAEXT, szExt, MAXSTRING) == 0)
         return (FALSE);
      if (LoadString (ghDLLInst, IDS_EXPPROMPT, szExpPmt, MAXSTRING) == 0)
         return (FALSE);
      if (LoadString (ghDLLInst, IDS_IMPPROMPT, szImpPmt, MAXSTRING) == 0)
         return (FALSE);
      bSLoaded = TRUE;
      }

   GCGetInfoS->GCSpecLevel = SPECLEVEL;		// DLL converter spec level
   GCGetInfoS->GCInfoFlags = GCI_EXPORTOK;	// we support export
   GCGetInfoS->GCVersion = (LPSTR) GCVT;	// GCVT = "X.XXD4\0" version
   GCGetInfoS->GCDescription = (LPSTR) szDesc;  // general converter description
   GCGetInfoS->GCImpExt  = (LPSTR) szExt;	// import extensions
   GCGetInfoS->GCImpPrompt  = (LPSTR) szImpPmt;	// prompts for import 
   GCGetInfoS->GCExpPrompt  = (LPSTR) szExpPmt; // and export dialogs
   return (TRUE);
   }

/****************************************************************************

    FUNCTION: GCCheckData 

    RETURNS:  TRUE if format recognized by converter, FALSE if
	      unsupported format

****************************************************************************/
DllExport BOOL WINAPI EXPORT GCCheckData (lpGCCHECKDATAS GCCheckDataS)
   {
   return (FALSE);
   }

/****************************************************************************

    FUNCTION: GCConvert 

    RETURNS:  0 if no error, 1 if cancel, > 1 if error (see GDLLERR.H)

	      Does file conversion.

****************************************************************************/
DllExport UINT WINAPI EXPORT GCConvert (lpGCCONVERTS GCImportS)
   {
   int 	 	iIn, iOut; 				// internal cnv file handles
   int   	iRet; 
   UINT  	uRead, uWrite;
   DWORD 	dTotal = 0L;
   char  	cbuf[80];
   HGLOBAL	hBuf = NULL;		// read file buffer handle
   LPVOID	lpBuf;			// read file buffer pointer

#ifdef DEBUG
_asm{int 3};				// can be used to invoke debugger
#endif

   if (GCImportS->GCCvtFlags && GCC_EXPORT)
      return (GCE_NOEXP);		// export not supported

   if ((iIn = cnvOpen (GCImportS)) == HFILE_ERROR)	// open source
      return (GCE_OPEN);

   if ((iOut = cnvCreate (GCImportS)) == HFILE_ERROR)	// open dest
      {
      cnvClose (iIn);
      return (GCE_CREATE);
      }

   // iRet is the return value.  It is always positive.  We initialize
   // it to -1 to flag that no error has occured and the converter is
   // not done yet

   iRet = -1;					// wait for done or error

   // allocate buffer for read buf (do not use static buffer so reentrant)

   if ((hBuf = GlobalAlloc (GHND, RBUFSZ)) == NULL)
      iRet = GCE_MEMORY;
   else
      {
      lpBuf = GlobalLock (hBuf);
      if (lpBuf == NULL)
         iRet = GCE_MEMORY;
      }

   while (iRet < 0)				// stops when error or done
      {
      uRead = cnvRead (iIn, lpBuf, RBUFSZ);	// read a buffer from source
      if (uRead == HFILE_ERROR)
         iRet = GCE_READ;			// done due to read error
      else if (uRead == 0)
         iRet = GCE_OK;				// done due to file copied
      else
         {
         uWrite = cnvWrite (iOut, lpBuf, uRead);
         if (uWrite == HFILE_ERROR)
            iRet = GCE_WRITE;			// done due to write error
         else if (uRead != uWrite)
            iRet = GCE_DISKFULL;		// done due to no room
         else dTotal += (DWORD) uWrite;
         }

      if (GCImportS->GCCallback != NULL)	// allow multitasking & abort
         {
         sprintf(cbuf, "Bytes written: %li", dTotal);
         if ((GCImportS->GCCallback ((LPSTR) cbuf)) == TRUE)
            iRet = GCE_CANCEL;
         }
      }

   if (hBuf != NULL)
      {
      GlobalUnlock (hBuf);			// release read buffer
      GlobalFree (hBuf);
      }

   cnvClose (iIn);				// close source file

   if (iRet != GCE_OK)				// if error
      {	
      cnvDelete (iOut);				// delete output file
      return (iRet);
      }

   GCImportS->GCDestMemFSize = cnvSeek (iOut, 0L, SEEK_END);  // save length
   GCImportS->GCDestMemFile = cnvClose (iOut);	// close output, save handle

   return (GCE_OK);
   }


/*****************************************************************************/
/*****************************************************************************/
/*									     */
/*                       File Handling Functions      			     */
/*									     */
/*****************************************************************************/
/*****************************************************************************/

/****************************************************************************

    FUNCTION: cnvOpen() 

    DESCRIPTION: Opens an exising data file either from disk or in memory

    PARAMETERS:  Takes a pointer to the GCCONVERTS structure passed to
		 GCConvert().  Opens the "Src" file

    RETURNS:  An integer file handle between 1 and MAXOF if sucessful or
	      HFILE_ERROR if error.  The returned file handle must be used 
	      in all other cnvXXXXX() calls (except cnvCreate)

****************************************************************************/

int cnvOpen (lpGCCONVERTS GCCnvS)
   {
   int i = 0;
   pFILESTC pFS;

   while (sFileStc[i].uFlag != 0 && i < MAXOF)	// find an empty structure
      i++;					// to remember file info in

   if (i >= MAXOF)				// no free handles
      return (HFILE_ERROR);

   pFS = &sFileStc[i];

   if (GCCnvS->GCSrcFileNm == NULL)		// no filename means source
      {						// file is in a memory block
      pFS->uFlag = MEMRD;
      pFS->lpFName = NULL;

#ifdef FLAT32
      pFS->hFile = (HANDLE) NULL;
#else
      pFS->hFile = (HFILE) NULL;
#endif

      pFS->hMem = GCCnvS->GCSrcMemFile;
      pFS->dFileLen  = GCCnvS->GCSrcMemFSize;

      if ((pFS->lpMem = (LPBYTE) GlobalLock (GCCnvS->GCSrcMemFile)) == NULL)
         {
         pFS->uFlag = 0;
         return (HFILE_ERROR);
         }

      pFS->dCur = 0L;
      pFS->dAlloc = GlobalSize (GCCnvS->GCSrcMemFile);
      return (++i);
      }
   else						// handle real disk file
      {
      pFS->uFlag = DISK;
      pFS->lpFName = GCCnvS->GCSrcFileNm;
      pFS->hMem = NULL;
      pFS->dFileLen  = 0L;
      pFS->dAlloc = 0L;
      pFS->dCur = 0L;
      pFS->lpMem = NULL;

#ifdef FLAT32

      pFS->hFile = CreateFile (pFS->lpFName, 
			       GENERIC_READ,
			       FILE_SHARE_READ,
			       NULL,
			       OPEN_EXISTING,
			       FILE_ATTRIBUTE_NORMAL,
			       NULL);

      if (pFS->hFile == INVALID_HANDLE_VALUE)
         {
         pFS->uFlag = 0;
	 pFS->hFile = (HANDLE) NULL;
         return (HFILE_ERROR);
         }

#else

      pFS->hFile = _lopen (pFS->lpFName, OF_READ | OF_SHARE_DENY_WRITE);
      if (pFS->hFile == HFILE_ERROR)
         {
         pFS->uFlag = 0;
	 pFS->hFile = (HFILE) NULL;
         return (HFILE_ERROR);
         }

#endif

      return (++i);
      }
   }

/****************************************************************************

    FUNCTION: cnvCreates() 

    DESCRIPTION: Creates a new data file either on disk or in memory.
		 Allocates intial memory if memory file.

    PARAMETERS:  Takes a pointer to the GCCONVERTS structure passed to
		 GCConvert().  Creates the "Dest" file

    RETURNS:  An integer file handle between 1 and MAXOF if sucessful or
	      HFILE_ERROR if error.  The returned file handle must be used 
	      in all other cnvXXXXX() calls (except cnvOpen)

****************************************************************************/

int cnvCreate (lpGCCONVERTS GCCnvS)
   {
   int i = 0;
   pFILESTC pFS;

   while (sFileStc[i].uFlag != 0 && i < MAXOF)	// find an empty structure
      i++;					// to remember file info in

   if (i >= MAXOF)				// no free handles
      return (HFILE_ERROR);

   pFS = &sFileStc[i];

   if (GCCnvS->GCDestFileNm == NULL)		// no filename means source
      {						// file is in a memory block
      pFS->uFlag = MEMWRT;
      pFS->lpFName = NULL;

#ifdef FLAT32
      pFS->hFile = (HANDLE) NULL;
#else
      pFS->hFile = (HFILE) NULL;
#endif

      if ((pFS->hMem = GlobalAlloc (GHND, MEMALLOC)) == NULL)
         {
         pFS->uFlag = 0;
         return (HFILE_ERROR);
         }

      pFS->dFileLen  = 0L;
      pFS->dAlloc = GlobalSize (pFS->hMem);

      if ((pFS->lpMem = (LPBYTE) GlobalLock (pFS->hMem)) == NULL)
         {
	 GlobalFree (pFS->hMem);
	 pFS->hMem = NULL;
         pFS->uFlag = 0;
         return (HFILE_ERROR);
         }

      pFS->dCur = 0L;
      return (++i);
      }
   else						// handle real disk file
      {
      pFS->uFlag = DISK;
      pFS->lpFName = GCCnvS->GCDestFileNm;
      pFS->hMem = NULL;
      pFS->dFileLen  = 0L;
      pFS->dAlloc = 0L;
      pFS->dCur = 0L;
      pFS->lpMem = NULL;

#ifdef FLAT32

      pFS->hFile = CreateFile (pFS->lpFName, 
			       GENERIC_WRITE,
			       FILE_SHARE_READ,
			       NULL,
			       CREATE_ALWAYS,
			       FILE_ATTRIBUTE_NORMAL,
			       NULL);

      if (pFS->hFile == INVALID_HANDLE_VALUE)
         {
         pFS->uFlag = 0;
	 pFS->hFile = (HANDLE) NULL;
         return (HFILE_ERROR);
         }

#else

      pFS->hFile = _lcreat (pFS->lpFName, 0);
      if (pFS->hFile == HFILE_ERROR)
         {
         pFS->uFlag = 0;
         pFS->hFile = (HFILE) NULL;
         return (HFILE_ERROR);
         }

#endif

      return (++i);
      }
   }

/****************************************************************************

    FUNCTION: cnvClose() 

    DESCRIPTION: Closes an open data file either on disk or in memory.

    PARAMETERS:  Takes an integer convert file handle

    RETURNS:  The global memory handle if a memory file or NULL otherwise

****************************************************************************/

HGLOBAL cnvClose (int i)
   {
   pFILESTC pFS;
   HGLOBAL  hGMem;

   if (i < 1 || i > MAXOF)				// bad handles
      return (NULL);

   pFS = &sFileStc[--i];

   if (pFS->uFlag == DISK)
      {

#ifdef FLAT32
      CloseHandle (pFS->hFile);
      pFS->hFile = (HANDLE) NULL;
#else
      _lclose (pFS->hFile);
      pFS->hFile = (HFILE) NULL;
#endif

      pFS->uFlag = 0;
      return (NULL);
      }
   else if (pFS->uFlag == MEMRD || pFS->uFlag == MEMWRT)
      {
      hGMem = pFS->hMem;
      GlobalUnlock (hGMem);
      pFS->hMem = NULL;
      pFS->dCur = 0L;
      pFS->lpMem = NULL;
      pFS->uFlag = 0;
      return (hGMem);
      }
   else
      return (NULL);
   }

/****************************************************************************

    FUNCTION: cnvDelete() 

    DESCRIPTION: Deletes an open data file either on disk or in memory.

    PARAMETERS:  Takes an integer convert file handle

    RETURNS:  

****************************************************************************/

void cnvDelete (int i)
   {
#ifndef FLAT32
   OFSTRUCT of;
#endif   
   pFILESTC pFS;

   if (i < 1 || i > MAXOF)				// bad handle
      return;

   pFS = &sFileStc[--i];

   if (pFS->uFlag == DISK)
      {

#ifdef FLAT32
      DeleteFile (pFS->hFile);
      pFS->hFile = (HANDLE) NULL;
#else
      _lclose (pFS->hFile);
      pFS->hFile = (HFILE) NULL;
      OpenFile (pFS->lpFName, &of, OF_DELETE);
#endif

      pFS->uFlag = 0;
      return;
      }
   else if (pFS->uFlag == MEMWRT)		// cannot delete source file
      {
      GlobalUnlock (pFS->hMem);
      GlobalFree (pFS->hMem);
      pFS->hMem = NULL;
      pFS->dCur = 0L;
      pFS->lpMem = NULL;
      pFS->dFileLen  = 0L;
      pFS->dAlloc = 0L;
      pFS->uFlag = 0;
      return;
      }
   else
      return;
   }

/****************************************************************************

    FUNCTION: cnvSeek() 

    DESCRIPTION: Moves the file pointer of an open data file either on
		 disk or in memory. 

    PARAMETERS:  i = integer convert file handle
		 lOffset = the number of bytes the file pointer is to be moved. 
	         iOrigin = starting position and direction of the file pointer.
			   this parameter must be one of the following values: 
 			Value 		Meaning
			SEEK_SET	Moves lOffset bytes from the beginning of the file.
			SEEK_CUR	Moves lOffset bytes from its current position.
			SEEK_END	Moves lOffset bytes from the end of the file.

    RETURNS:     If the function succeeds, the return value specifies the
	         new offset of the pointer, in bytes, from the beginning of
                 the file.   If the function fails, the return value is
                 HFILE_ERROR.

    NOTES:       If the pointer is moved past the end of the file, this 
		 function does not allocate additional memory if memory
		 file.  Memory is allocated on cnvWrite().

****************************************************************************/

LONG cnvSeek (int  i, LONG lOffset, int iOrigin)
   {
   pFILESTC pFS;

   if (i < 1 || i > MAXOF)				// bad handles
      return (HFILE_ERROR);

   pFS = &sFileStc[--i];

#ifdef FLAT32

   if (pFS->uFlag == DISK)
      return (SetFilePointer (pFS->hFile, lOffset, NULL, iOrigin));

#else

   if (pFS->uFlag == DISK)
      return (_llseek (pFS->hFile, lOffset, iOrigin));

#endif

   else if (pFS->uFlag == MEMRD || pFS->uFlag == MEMWRT)
      {
      switch (iOrigin) 
         { 
         case SEEK_SET: 
            pFS->dCur = lOffset;
	    break;

         case SEEK_CUR: 
            pFS->dCur += lOffset;
	    break;

         case SEEK_END: 
            pFS->dCur = pFS->dFileLen + lOffset;
	    break;

         default: 
            return (HFILE_ERROR);
         }

      if (pFS->dCur < 0L)
         pFS->dCur = 0L;
      return (pFS->dCur);
      }
   else
      return (HFILE_ERROR);
   }


/****************************************************************************

    FUNCTION: cnvRead() 

    DESCRIPTION: Reads from an open data file either on disk or in memory. 

    PARAMETERS:  i = integer convert file handle
		 lpBuffer = buffer to return bytes read in
	         uBytes = bytes to read in buffer (0 to 0xfffe)

    RETURNS:     If the function succeeds, the return value specifies the
	         actual number of bytes read into the buffer.  If the
		 function fails, the return value is HFILE_ERROR.

****************************************************************************/

UINT cnvRead (int i, LPVOID lpBuffer, UINT uBytes)
   {
   pFILESTC pFS;
   LPBYTE lpCur;

#ifdef FLAT32
   UINT uBytesRead;
#else
   BYTE _huge *lpCurH;
#endif

   if (i < 1 || i > MAXOF)				// bad handles
      return ((UINT) HFILE_ERROR);

   pFS = &sFileStc[--i];

#ifdef FLAT32

   if (pFS->uFlag == DISK)
      {
      if (ReadFile (pFS->hFile, lpBuffer, uBytes, &uBytesRead, NULL) == FALSE)
         return ((UINT) HFILE_ERROR);
      else
         return (uBytesRead);
      }

#else

   if (pFS->uFlag == DISK)
      return (_lread (pFS->hFile, lpBuffer, uBytes));

#endif

   else if (pFS->uFlag == MEMRD || pFS->uFlag == MEMWRT)
      {
      if (pFS->dCur >= pFS->dFileLen)			// cannot read past EOF
         return (0);

      if ((pFS->dCur + (DWORD) uBytes) > pFS->dFileLen)
         uBytes = (UINT) (pFS->dFileLen - pFS->dCur);

#ifdef FLAT32

      lpCur = pFS->lpMem + pFS->dCur;
      memcpy (lpBuffer, lpCur, uBytes);
#else

      lpCurH = pFS->lpMem;
      lpCur = &lpCurH[pFS->dCur];
      _fmemcpy (lpBuffer, lpCur, uBytes);

#endif


      pFS->dCur += uBytes;
      return (uBytes);
      }
   else
      return ((UINT) HFILE_ERROR);
   }


/****************************************************************************

    FUNCTION: cnvWrite() 

    DESCRIPTION: Writes to an open data file either on disk or in memory. 

    PARAMETERS:  i = integer convert file handle
		 lpBuffer = buffer with bytes to write in
	         uBytes = bytes to write in buffer (0 to 0xfffe)

    RETURNS:     If the function succeeds, the return value specifies the
	         actual number of bytes written.  If the function fails,
		 the return value is HFILE_ERROR.

****************************************************************************/

UINT cnvWrite (int i, LPVOID lpBuffer, UINT uBytes)
   {
   pFILESTC pFS;
   LPBYTE   lpCur;
   HGLOBAL  hReAlloc;
   DWORD    dNewAlloc;

#ifdef FLAT32
   UINT uBytesRead;
#else
   BYTE _huge *lpCurH;
#endif

   if (i < 1 || i > MAXOF)				// bad handles
      return ((UINT) HFILE_ERROR);

   pFS = &sFileStc[--i];

   if (uBytes == 0)
      return (0);


#ifdef FLAT32

   if (pFS->uFlag == DISK)
      {
      if (WriteFile (pFS->hFile, lpBuffer, uBytes, &uBytesRead, NULL) == FALSE)
         return ((UINT) HFILE_ERROR);
      else 
         return (uBytesRead);
      }

#else

   if (pFS->uFlag == DISK)
      return (_lwrite (pFS->hFile, lpBuffer, uBytes));

#endif

   else if (pFS->uFlag == MEMRD || pFS->uFlag == MEMWRT)
      {
      if (pFS->dCur + (DWORD) uBytes > pFS->dAlloc)	// need to alloc memory?
         {
	 GlobalUnlock (pFS->hMem);
	 pFS->lpMem = NULL;

         if (((pFS->dCur + (DWORD) uBytes) - pFS->dAlloc) < MEMALLOC)
            dNewAlloc = pFS->dAlloc + MEMALLOC;
         else
            dNewAlloc = pFS->dCur + (DWORD) uBytes + MEMALLOC;

	 hReAlloc = GlobalReAlloc (pFS->hMem, dNewAlloc, GHND);
         if (hReAlloc != NULL)
	    pFS->hMem = hReAlloc;

         pFS->dAlloc = GlobalSize (pFS->hMem);

         if ((pFS->lpMem = (LPBYTE) GlobalLock (pFS->hMem)) == NULL)
            {
	    GlobalFree (pFS->hMem);
	    pFS->hMem = NULL;
            pFS->uFlag = 0;
            return ((UINT) HFILE_ERROR);
            }
         }

       // ok we may have enough memory to write the file

      if (pFS->dCur > pFS->dAlloc)		// out of memory
         return (0);

      if ((pFS->dCur + (DWORD) uBytes) > pFS->dAlloc)	// can we write it all?
         uBytes = (UINT) (pFS->dAlloc - pFS->dCur);

#ifdef FLAT32

      lpCur = pFS->lpMem + pFS->dCur;
      memcpy (lpCur, lpBuffer, uBytes);

#else

      lpCurH = pFS->lpMem;
      lpCur = &lpCurH[pFS->dCur];
      _fmemcpy (lpCur, lpBuffer, uBytes);

#endif

      pFS->dCur += uBytes;

      if (pFS->dCur > pFS->dFileLen)	// if file is longer, update lenght ctr
         pFS->dFileLen = pFS->dCur;

      return (uBytes);
      }
   else
      return ((UINT) HFILE_ERROR);
   }


