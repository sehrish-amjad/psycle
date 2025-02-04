# Microsoft Developer Studio Project File - Name="dsp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=dsp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dsp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dsp.mak" CFG="dsp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dsp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "dsp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dsp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\container\src" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "dsp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\container\src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "dsp - Win32 Release"
# Name "dsp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\adsr.c
# End Source File
# Begin Source File

SOURCE=.\src\alignedalloc.c
# End Source File
# Begin Source File

SOURCE=.\src\dither.c
# End Source File
# Begin Source File

SOURCE=.\src\envelope.c
# End Source File
# Begin Source File

SOURCE=.\src\fft.c
# End Source File
# Begin Source File

SOURCE=.\src\filter.c
# End Source File
# Begin Source File

SOURCE=.\src\filtercoeff.c
# End Source File
# Begin Source File

SOURCE=.\src\linear.c
# End Source File
# Begin Source File

SOURCE=.\src\mersennetwister.c
# End Source File
# Begin Source File

SOURCE=.\src\multiresampler.c
# End Source File
# Begin Source File

SOURCE=.\src\noteperiods.c
# End Source File
# Begin Source File

SOURCE=.\src\notestab.c
# End Source File
# Begin Source File

SOURCE=".\src\operations-noopt.c"
# End Source File
# Begin Source File

SOURCE=".\src\operations-sse2.c"
# End Source File
# Begin Source File

SOURCE=.\src\operations.c
# End Source File
# Begin Source File

SOURCE=.\src\quantize.c
# End Source File
# Begin Source File

SOURCE=.\src\resampler.c
# End Source File
# Begin Source File

SOURCE=.\src\rms.c
# End Source File
# Begin Source File

SOURCE=".\src\sinc-sse2.c"
# End Source File
# Begin Source File

SOURCE=.\src\sinc.c
# End Source File
# Begin Source File

SOURCE=".\src\spline-sse2.c"
# End Source File
# Begin Source File

SOURCE=.\src\spline.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\adsr.h
# End Source File
# Begin Source File

SOURCE=.\src\alignedalloc.h
# End Source File
# Begin Source File

SOURCE=.\src\dither.h
# End Source File
# Begin Source File

SOURCE=.\src\dsptypes.h
# End Source File
# Begin Source File

SOURCE=.\src\envelope.h
# End Source File
# Begin Source File

SOURCE=.\src\fft.h
# End Source File
# Begin Source File

SOURCE=.\src\filter.h
# End Source File
# Begin Source File

SOURCE=.\src\filtercoeff.h
# End Source File
# Begin Source File

SOURCE=.\src\linear.h
# End Source File
# Begin Source File

SOURCE=.\src\mersennetwister.h
# End Source File
# Begin Source File

SOURCE=.\src\multiresampler.h
# End Source File
# Begin Source File

SOURCE=.\src\noteperiods.h
# End Source File
# Begin Source File

SOURCE=.\src\notestab.h
# End Source File
# Begin Source File

SOURCE=.\src\operations.h
# End Source File
# Begin Source File

SOURCE=.\src\quantize.h
# End Source File
# Begin Source File

SOURCE=.\src\resampler.h
# End Source File
# Begin Source File

SOURCE=.\src\rms.h
# End Source File
# Begin Source File

SOURCE=".\src\sinc-sse2.h"
# End Source File
# Begin Source File

SOURCE=.\src\sinc.h
# End Source File
# Begin Source File

SOURCE=".\src\spline-sse2.h"
# End Source File
# Begin Source File

SOURCE=.\src\spline.h
# End Source File
# Begin Source File

SOURCE=.\src\sse_mathfun.h
# End Source File
# End Group
# End Target
# End Project
