SPC Data File - Software Developer's Kit
Copyright (c)1996-1997 Galactic Industries Corp. - All rights reserved.

The files enclosed in this developer's kit are provided for the purpose of 
developing software routines to translate analytical laboratory instrumentation
data (i.e spectra & chromatograms) into Galactic Industries' SPC file format.  
Uses of this information for any other purposes without the express written
permission of Galactic Industries Corp. is prohibited.

Galactic Industries Corp. has made it's best efforts to insure that the 
information enclosed is correct.  However, Galactic makes no warranties, either
express or implied as to the completeness, usability and correctness of the 
information.  By using this information, it is implied that the user understands
and accepts these conditions.

The documentation is provided in both Microsoft Word 97 and Adobe Acrobat formats.
A file reader for Adobe Acrobat files can be donloaded for a variety of operating
systems at http://www.adobe.com/prodindex/acrobat/readstep.html. 

The main directory of the Developer's Kit contains:

File		Description
----		-----------
README.TXT	This file.
GSPC_UDF.DOC	Galactic Universal Data File Spec. (Microsoft Word 97)
GSPC_UDF.PDF	Galactic Universal Data File Spec. (Adobe Acrobat)
GDRV_UDI.DOC	Galactic Universal Driver Interface Spec. (Microsoft Word 97)
GDRV_UDI.PDF	Galactic Universal Driver Interface Spec. (Adobe Acrobat)
GRAMSCNV.EXE	Galactic Convert Application (V2.10, 16-bit) 
SPC.H		C language header - Galactic SPC file format
GCDLL.H		C language header - Galactic File Converters
GCDLLERR.H	C language header - Errors for Galactic File Converters
COPYFILE.DEF	Example converter - 16-bit DEF file
COPYFILE.C	Example converter - source
COPYFILE.H	Example converter - header
COPYFILE.RC	Example converter - resource
COPYFILE.CN4	Compiled 16-bit example converter
COPYF16.MAK 	Make file for example converter
COPYF32.DEF	Example converter - 32 bit DEF file
GRAMSDDE.H	C language header - GRAMS/386/32 DDE Definitions


The DATA directory of the Developer's Kit contains:

File		Description
----		-----------
S_EVENX.SPC	Single spectrum, evenly spaced X
S_XY.SPC	Single spectrum, variable spaced X (X & Y arrays)
M_EVENZ.SPC	Multifile spectrum, evenly spaced X, evenly spaced Z
M_ORD.SPC	Multifile spectrum, evenly spaced X, ordered Z
M_XYXY.SPC	Multifile spectrum, variable size X/Y arrays for every spectrum
4D_MAP.SPC	Multifile, 4D spectral mapping data
FT-IR.SPC	Example FT-IR spectrum with log text
RAMAN.SPC	Example FT Raman spectrum with log text
NMR_FID.SPC	Example NMR FID with log text
NMR_SPC.SPC	Example NMR spectrum with log text
NIR.SPC		Example NIR spectra with log text
NIR.CFL		Example calibration data for NIR spectra
MS.SPC		Example single Mass spectrum

