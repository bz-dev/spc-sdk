########################################################################
#
#    PROGRAM: copyf16.mak
#
#    PURPOSE: NMAKE.EXE file to build sample 16-bit converter DLL
#
#    AUTHOR:   David Abrams  August 20, 1996
#
#    MRU:
#
#   COMMENTS:  make with debug=1 to build debug version
#
########################################################################


##### Module Macro #####
DLLNAME   = copyfile
DLLSRCS   = $(DLLNAME).c

##### C7 Macro #####
C7	   = 1

##### Library Macro #####
## one 64K code, far data pointers (COMPACT model)
DLLLIBS    = libw cdllcew
DLLMOD	   = -ACw

## Note the LARGE model could be used if code grows > 64K
##	DLLLIBS    = libw ldllcew
##	DLLMOD	   = -ALw


##### Include Macro #####
DLLINCLS  = $(DLLNAME).h gcdll.h gcdllerr.h

##### Resource Macro #####
DLLRCFILES=

##### DEBUG Defined #####
!ifdef debug
DEBUG	= 1
!else
DEBUG	= 0
!endif

##### Build Option Macros #####
!if $(DEBUG)
DDEF	= -DDEBUG
CLOPT	= -Zid -Od -nologo
MOPT	= -Zi
LOPT	= /CO /LI /MAP
!else
DDEF	=
CLOPT	= -Os
LOPT	=
!endif

##### General Macros #####
DEF	=

##### Tool Macros #####
CC2	= cl -c $(DLLMOD) -G2sw -Zp -W3 $(CLOPT) $(DDEF) $(DEF)
LINK	= link /NOD /NOE $(LOPT)
RC	= rc $(DDEF) $(DEF)
HC	= hc

##### Inference Rules #####

.c.obj:
    $(CC2) $*.c

.rc.res:
    $(RC) -r $*.rc

##### Main (default) Target #####
goal:  $(DLLNAME).cn4 $(DLLNAME).res

##### Dependents For Goal and Command Line #####
$(DLLNAME).obj: $(DLLNAME).c
    $(CC2) $*.c

$(DLLNAME).res: $(DLLNAME).rc
    rc -r $(DLLNAME).rc

$(DLLNAME).cn4: $(DLLSRCS:.c=.obj) $(DLLNAME).def $(DLLNAME).res
    $(LINK) @<<
    $(DLLSRCS:.c=.obj),
    $(DLLNAME).cn4,
    $(DLLNAME).map,
    $(DLLLIBS),
    $(DLLNAME).def
<<
!if $(DEBUG)
    $(RC) -K -T -31 $(DLLNAME).rc $(DLLNAME).cn4 
!else
    $(RC) -T -31 $(DLLNAME).rc $(DLLNAME).cn4 
!endif
!if $(DEBUG)
!if !$(C7)
    cvpack -p $(DLLNAME).cn4
!endif
    mapsym $(DLLNAME).map
!endif


##### Dependents #####
$(DLLSRCS:.c=.obj): $(DLLINCLS)

##### Clean Directory #####
clean:
    -del *.obj
    -del *.res
    -del *.exe
    -del *.dll
    -del *.lib
    -del *.sym
    -del *.map
