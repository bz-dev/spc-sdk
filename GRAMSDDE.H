/**************************************************************************
*   FILENAME:	gramsdde.h
*   AUTHOR:	David Abrams 11-22-91 (from gramsdde.inc)
*   MRU:	10-25-92 Add infostc, ABControl posting choices, xfrflag=XFABC.
*	Collect DDE Transfer Protocol Definitions.
*	Must #include <windows.h> before this file.
*
*   Copyright (C) 1991-1992 Galactic Industries Corp.  All Rights Reserved.
***************************************************************************
*
*  This structure comes at the beginning of a GRAMS-file-format DDE data block.
*  The image of a GRAMS file (such as an .SPC format file) immediately follows.
*  If the first xfrchk four bytes of a DDE transfer block are not: 254,24,6,18
*  (=0xFE,^X,^F,^R or 0x120618FE) then the data block is not a GRAMS format
*  file.  This means that CF_TEXT data can double as a GRAMS file image so that
*  servers which do not support this GRAMS format automatically send text data.
*
*  If the check words match, then the XFRBEG area is 512 bytes long and is
*  immediately followed by an exact image of a GRAMS disk file. Note that
*  it is important that the xfrname contain the filename extension (such as
*  .SPC, .PKS, .GDB, etc) so that the type of GRAMS file is known.  The
*  extension is not case sensitive.  The xfrmtd list is a series of file path
*  names separated by spaces, tabs, or commas and ending with a final zero.
*  The filenames may optionally include disk and directory and should always
*  include the extension.
*
*  The xfrchan and xfrexpr settings are needed only for unsolicited (POKE) data
*  and allow GRAMS to determine the channel to be used to process the data.
*  If xfrchan is non-zero then it determines the channel.  Otherwise, the
*  channel with a matching experiment code is used.  Otherwise, the experiment
*  implied by the xfrname extension is used to determine the channel or channel
*  1 is used.
*
*  Note that many settings be "unspecified" (0) so that GRAMS' default settings
*  are used instead of the supplied settings.  Also some settings are duplicated
*  in the SPC header and can be supplied there rather than in this block.
*  Most 0 values for parameters are "unspecified" so GRAMS uses its own
*  defaults.  The xfrflag byte cannot be defaulted an must have a valid setting
*  (usually 0).
*
*  Note that xfrpost is equivalent to fpost in the SPC header.	Also equivalent:
*  xfrproc=fprocs, xfrlevl=flevel, xfrsamp=fsampin xfrfac=ffactor,
*  xfrmtd=fmethod   The method list can have comma-separated filenames with
*  extensions.	However, only the *.MTD method name is placed back in the SPC's
*  fmethod for processing.
***************************************************************************/

/* Four check bytes at beginning of data block to flag disk image. */

#define VXFRCHK0  254
#define VXFRCHK1  24
#define VXFRCHK2  6
#define VXFRCHK3  18

typedef struct
    {
    BYTE  xfrchk[4]; /* Must be 120618FE Hex (0xFE, Ctrl-X, Ctrl-F, Ctrl-R) */
    DWORD xfrsize;   /* Data size including 512 for header (0=use actual size)*/
    BYTE  xfrflag;   /* Flags (must always be specified, see below) */
    BYTE  xfrchan;   /* Channel to use (1-4 or 0 if unspecified) */
    WORD  xfrfree;   /* Unused (may be used by collect server) */
    WORD  xfrform;   /* Clipboard format (not currently used) */
    BYTE  xfrexpr;   /* Experiment code to use plus one (0 if unspecified) */
    BYTE  xfrpost;   /* Posting disposition (see below) */
    BYTE  xfrproc;   /* Processing type (see below) */
    BYTE  xfrlevl;   /* Calibration level plus one (1 = not calibration data) */
    WORD  xfrsamp;   /* Sub-method injection sample number (1=first) */
    float xfrfac;    /* Floating-point concentration factor (IEEE 32-bit) */
    char  xfrmtd[48*3];  /* Method/program/reference/library/archive fnm list */
    BYTE xfrabcm;    /* Mode argument when xfrflag=XFABC */
    BYTE xfrabcn;    /* Database number for channel when xfrflag=XFABC */
    WORD xfrabck;    /* Database type key when xfrflag=XFABC */
    WORD xfrabcf;    /* Database dbflags (LSB) & dbftflg (MSB) creation flags*/
    BYTE  xfrresv[82];	 /* Reserved (must be zero) */
    char  xfrname[256];  /* File path name (with optional disk and directory) */
    } XFRBEG;

/***************************************************************************
* These are the xfrflag flag definitions.
* When XFMORE is set, then GRAMS assumes that the run will continue to produce
* more files in the future and requests another DDE file from the server.
* When XFDISK is set, then GRAMS reads the file from disk to do any processing.
* When XFSUBF is set, GRAMS keeps multifile open and adds subfiles to end.
* The XFHOTLK allows server to switch to hot link protocol at run time.
* Whether or not XFNOERROR is set, GRAMS returns any processing error code
* in the DDE_FAPPSTATUS bits of wStatus (lpdwResult from DdeClientTranaction).
* A zero code indicates no error.  See ERROR.INC for a list of code numbers.
* The XFHOMED flag allows Array Basic programs and other processing files
* normally located in GRAMS' "home" directory to be located instead in the
* data file's directory without having to specify complete path names for files.
***************************************************************************/

#define XFNONE	0
#define XFMORE	1   /* This is not the last data file to be sent for the run */
#define XFDISK	2   /* Data file is on the disk (and nothing follows xfrbeg) */
#define XFSUBF	4   /* Data is a subfile in a series from a multifile */
#define XFNSHOW 8   /* Do not show collect data in GRAMS even if foreground */
#define XFHOTLK 16  /* GRAMS should use the hot-link protocol for rest of run */
#define XFNOERR 32  /* GRAMS should skip showing error messages for data */
#define XFHOMED 64  /* Sets "home" directory to match data's during processing*/
#define XFABC	128 /* Alternate ABC-style processing and posting via @colPost*/

/***************************************************************************
* These are the xfrproc post processing type definitions.
* Beware that PPDEFLT (0) is reserved for "unspecified".  Otherwise, PPKNOWN=1
* MUST be added into the sum of the bit settings.  For example, to specify that
* compute (PPCOMP) and search (PPSRCH) be used, the xfrproc values must be
* set to:  PPCOMP+PPSRCH+PPKNOWN = 66.	For no processing: PPNONE+PPKNOWN = 1.
* Most of the bit settings may be added together to specify multiple actions
* as long as 1 is added to the result.	However, PPTRANS, PPABSRB, and PPKMUNK
* are mutually exclusive.  Only one of the three may be added into the sum.

* Also, "?" within filename denotes the experiment's beginning letter:
*  "S"=SPC (General), "C"=CGM, "G"=GC, "L"=LC, "H"=HC (HPLC), "F"=FIR (FTIR),
*  "I"=IR (NIR), "V"=VIS, "U"=UV (UV/VIS), "X"=XRY (X-Ray), "M"=MS (Mass Spec),
*  "N"=NMR, "R"=RMN (Raman), "E"=EFL (Excitation Fluorescence),
*  "A"=AAS (Atomic), "D"=DAS (Diode Array), "O"=OTR (Other).
* Although the experiment letters are limited to "ACDEFGHILMNORSXUV",
* they need not be used for the types of experiments implied above.
* The code number for the first "S" experiment is 0 and for the "O" last is 16.

* Array Basic .ABP or .DLL program extensions are used (rather than .AB or .ABC)
* so that the special post processing programs do not show in the DoProg menu.
* During any post processing, the file is temporarily the current #S database.
* Processing is done in the order listed below.  In some systems, the PPCOMP
* progam may also do the background ratio and output transmission, absorbance,
* or k-munk spectra.  If the xfrmtd list has a SPC-type spectrum filename,
* then it its assumed to the desired background spectrum.  (Otherwise, the
* spectrum with the same experiment type in GRAMS' background list is used.)
* If PPPEAK is specified, the a *.MTD method filename from the xfrmtd list
* is used as the peak picking method.  (If none is present, then the default
* method chosen in GRAMS's Peak menu is used.)  If PPSRCH is specified than
* a LIB-type library database filename in the xfrmtd list is searched.	(If
* no library is listed, then the library or libraries from GRAMS's library list
; with matching experiments are used.)	Finally, any *.AB* or *.DLL Array Basic
; program filenames listed in xfrmtd are executed in order.
***************************************************************************/

#define PPDEFLT 0    /* Use default settings (xfrproc is "unspecified") */
#define PPKNOWN 1    /* MUST BE ADDED TO THE SUM OF SETTINGS INCLUDED BELOW! */

#define PPNONE	0    /* No post processing */
#define PPCOMP	1    /* Compute (run PPCOMP?.ABP) */
#define PPDLLC	2    /* Compute with DLL (run PPCOMP?.DLL) */
#define PPTRANS 4    /* Transmission (run PPTRANS?.ABP) */
#define PPABSRB 8    /* Absorbance (run PPABSRB?.ABP) */
#define PPKMUNK 4+8  /* Kuebelka-Munk (run PPKMUNK?.ABP) */
#define PPPEAK	32   /* GRAMS built-in peak picking and reporting */
#define PPSRCH	64   /* Library Search associated w/experiment's LIB driver */
#define PPUSER	128  /* General user-written post program (run PPUSER?.ABP) */

/***************************************************************************
* These are the xfrpost posting file dispositon settings.  For PSTAPPD, the data
* is appended to a .GDB general archive database (or .DBS custom database) if
* one is listed in xfrmtd.  Otherwise it is appended to the current #R database.
***************************************************************************/

#define PSTDEFT 0    /* Use default setting (xfrpost is "unspecified") */
#define PSTSAVE 1    /* Save file to disk (but remove from memory) */
#define PSTAPPD 2    /* Append to end of #R database or xfrmtd .GDB database */
#define PSTMERG 3    /* Merge into current #R database row */
#define PSTBACK 4    /* Save as new background for experiment */
#define PSTNONE 5    /* Do not save after processing (if any) */
#define PSTKEEP 6    /* Do not save and keep in memory as #S */
#define PSTBOTH 7    /* Both disk save & keep in #S memory (ABC Driver Only!) */
#define PSTASK	8    /* Query user: save, keep, or both (ABC Driver Only!) */

/***************************************************************************
* This structure defines extra information passed to @ColData.
* The size of the infostc MUST BE 512 BYTES (SPCHSZ)!
* The infospc offset must come at the beginning!
***************************************************************************/

typedef struct
    {
    BYTE  infospc[440]; /* spchdr information (see spc.inc) */
    BYTE  inforsv[8];	/* reserved (must be zero) */
    WORD  infokey;	/* database type key (0=SPC) */
    BYTE  infonum;	/* database number for channel (1=1stSlot,<0=SubDB) */
    BYTE  infoflg;	/* cdbflag settings (including CDFSHOW, see db.inc) */
    WORD  infofl2;	/* reserved for future flags (must be zero) */
    WORD  infoprg;	/* procprog (MENU.ABM line or -1:/scans,-2:T->A,-3:T->K)*/
    DWORD infonpt;	/* maximum number of points to collect */
    DWORD infosub;	/* maximum number of subfiles to collect */
    DWORD infoscn;	/* maximum number of scans per subfile */
    DWORD infobuf;	/* number of preallocated cbufstc subfile buffers */
    double infoxbeg;	/* default display left X (only if <> infoxend) */
    double infoxend;	/* default display right X */
    double infoymin;	/* default display bottom Y (only if <> infoymax) */
    double infoymax;	/* default display top Y (must be > minimum) */
    float  infozbeg;	/* default display begin Z (only if <> infozend) */
    float  infozend;	/* default display end Z */
    } INFOSTC;

