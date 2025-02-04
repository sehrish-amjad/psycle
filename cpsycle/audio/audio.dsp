# Microsoft Developer Studio Project File - Name="audio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=audio - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "audio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "audio.mak" CFG="audio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "audio - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "audio - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "audio - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\file\src" /I "..\container\src" /I "..\dsp\src" /I "..\lua53\src" /I "..\vst-2.4\vst2.x" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\file\src" /I "..\container\src" /I "..\dsp\src" /I "..\lua53\src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
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

# Name "audio - Win32 Release"
# Name "audio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\array.c
# End Source File
# Begin Source File

SOURCE=.\src\audiorecorder.c
# End Source File
# Begin Source File

SOURCE=.\src\buffer.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\buffercontext.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\connections.c
# End Source File
# Begin Source File

SOURCE=.\src\custommachine.c
# End Source File
# Begin Source File

SOURCE=.\src\dummy.c
# End Source File
# Begin Source File

SOURCE=.\src\duplicator.c
# End Source File
# Begin Source File

SOURCE=.\src\duplicator2.c
# End Source File
# Begin Source File

SOURCE=.\src\duplicatormap.c
# End Source File
# Begin Source File

SOURCE=.\src\eventdrivers.c
# End Source File
# Begin Source File

SOURCE=.\src\exclusivelock.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\fileoutdriver.c
# End Source File
# Begin Source File

SOURCE=.\src\inputmap.c
# End Source File
# Begin Source File

SOURCE=.\src\instrument.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\instruments.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\kbddriver.c
# End Source File
# Begin Source File

SOURCE=.\src\ladspaplugin.c
# End Source File
# Begin Source File

SOURCE=.\src\library.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\lock.c
# End Source File
# Begin Source File

SOURCE=.\src\luaarray.c
# End Source File
# Begin Source File

SOURCE=.\src\luaenvelope.c
# End Source File
# Begin Source File

SOURCE=.\src\luaimport.c
# End Source File
# Begin Source File

SOURCE=.\src\luaplugin.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\luawaveosc.c
# End Source File
# Begin Source File

SOURCE=.\src\machine.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\machinefactory.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\machineinfo.c
# End Source File
# Begin Source File

SOURCE=.\src\machineparam.c
# End Source File
# Begin Source File

SOURCE=.\src\machineproxy.c
# End Source File
# Begin Source File

SOURCE=.\src\machines.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\master.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\mixer.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\dsp\src\operations.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\pattern.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\patternentry.c
# End Source File
# Begin Source File

SOURCE=.\src\patternevent.c
# End Source File
# Begin Source File

SOURCE=.\src\patternio.c
# End Source File
# Begin Source File

SOURCE=.\src\patterns.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\patternstrackstate.c
# End Source File
# Begin Source File

SOURCE=.\src\player.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\plugin.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\plugin_interface.cpp

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\plugincatcher.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\preset.c
# End Source File
# Begin Source File

SOURCE=.\src\presetio.c
# End Source File
# Begin Source File

SOURCE=.\src\presets.c
# End Source File
# Begin Source File

SOURCE=.\src\psy2.c
# End Source File
# Begin Source File

SOURCE=.\src\psy2converter.c
# End Source File
# Begin Source File

SOURCE=.\src\psy3.c
# End Source File
# Begin Source File

SOURCE=.\src\psyclescript.c
# End Source File
# Begin Source File

SOURCE=.\src\sample.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\sampleiterator.c
# End Source File
# Begin Source File

SOURCE=.\src\sampler.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\samples.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\sequence.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\sequencer.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\silentdriver.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\song.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\songio.c
# End Source File
# Begin Source File

SOURCE=.\src\vstplugin.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\waveio.c

!IF  "$(CFG)" == "audio - Win32 Release"

!ELSEIF  "$(CFG)" == "audio - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\waveosc.c
# End Source File
# Begin Source File

SOURCE=.\src\waveosctables.c
# End Source File
# Begin Source File

SOURCE=.\src\wavsongio.c
# End Source File
# Begin Source File

SOURCE=.\src\wire.c
# End Source File
# Begin Source File

SOURCE=.\src\xm.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\array.h
# End Source File
# Begin Source File

SOURCE=.\src\audiorecorder.h
# End Source File
# Begin Source File

SOURCE=.\src\buffer.h
# End Source File
# Begin Source File

SOURCE=.\src\buffercontext.h
# End Source File
# Begin Source File

SOURCE=.\src\connections.h
# End Source File
# Begin Source File

SOURCE=.\src\constants.h
# End Source File
# Begin Source File

SOURCE=.\src\custommachine.h
# End Source File
# Begin Source File

SOURCE=.\src\dummy.h
# End Source File
# Begin Source File

SOURCE=.\src\duplicator.h
# End Source File
# Begin Source File

SOURCE=.\src\duplicator2.h
# End Source File
# Begin Source File

SOURCE=.\src\duplicatormap.h
# End Source File
# Begin Source File

SOURCE=.\src\eventdrivers.h
# End Source File
# Begin Source File

SOURCE=.\src\exclusivelock.h
# End Source File
# Begin Source File

SOURCE=.\src\fileoutdriver.h
# End Source File
# Begin Source File

SOURCE=.\src\inputmap.h
# End Source File
# Begin Source File

SOURCE=.\src\instrument.h
# End Source File
# Begin Source File

SOURCE=.\src\instruments.h
# End Source File
# Begin Source File

SOURCE=.\src\kbddriver.h
# End Source File
# Begin Source File

SOURCE=.\src\ladspa.h
# End Source File
# Begin Source File

SOURCE=.\src\ladspaplugin.h
# End Source File
# Begin Source File

SOURCE=.\src\library.h
# End Source File
# Begin Source File

SOURCE=.\src\lock.h
# End Source File
# Begin Source File

SOURCE=.\src\luaarray.h
# End Source File
# Begin Source File

SOURCE=.\src\luaenvelope.h
# End Source File
# Begin Source File

SOURCE=.\src\luaimport.h
# End Source File
# Begin Source File

SOURCE=.\src\luaplugin.h
# End Source File
# Begin Source File

SOURCE=.\src\luawaveosc.h
# End Source File
# Begin Source File

SOURCE=.\src\machine.h
# End Source File
# Begin Source File

SOURCE=.\src\machinedefs.h
# End Source File
# Begin Source File

SOURCE=.\src\machinefactory.h
# End Source File
# Begin Source File

SOURCE=.\src\machineinfo.h
# End Source File
# Begin Source File

SOURCE=.\src\machineparam.h
# End Source File
# Begin Source File

SOURCE=.\src\machineproxy.h
# End Source File
# Begin Source File

SOURCE=.\src\machines.h
# End Source File
# Begin Source File

SOURCE=.\src\master.h
# End Source File
# Begin Source File

SOURCE=.\src\mixer.h
# End Source File
# Begin Source File

SOURCE=.\src\pattern.h
# End Source File
# Begin Source File

SOURCE=.\src\patternentry.h
# End Source File
# Begin Source File

SOURCE=.\src\patternevent.h
# End Source File
# Begin Source File

SOURCE=.\src\patternio.h
# End Source File
# Begin Source File

SOURCE=.\src\patterns.h
# End Source File
# Begin Source File

SOURCE=.\src\patternstrackstate.h
# End Source File
# Begin Source File

SOURCE=.\src\player.h
# End Source File
# Begin Source File

SOURCE=.\src\plugin.h
# End Source File
# Begin Source File

SOURCE=.\src\plugin_interface.h
# End Source File
# Begin Source File

SOURCE=.\src\plugincatcher.h
# End Source File
# Begin Source File

SOURCE=.\src\preset.h
# End Source File
# Begin Source File

SOURCE=.\src\presetio.h
# End Source File
# Begin Source File

SOURCE=.\src\presets.h
# End Source File
# Begin Source File

SOURCE=.\src\psy2.h
# End Source File
# Begin Source File

SOURCE=.\src\psy2converter.h
# End Source File
# Begin Source File

SOURCE=.\src\psy3.h
# End Source File
# Begin Source File

SOURCE=.\src\psyclescript.h
# End Source File
# Begin Source File

SOURCE=.\src\sample.h
# End Source File
# Begin Source File

SOURCE=.\src\sampleiterator.h
# End Source File
# Begin Source File

SOURCE=.\src\sampler.h
# End Source File
# Begin Source File

SOURCE=.\src\samples.h
# End Source File
# Begin Source File

SOURCE=.\src\sequence.h
# End Source File
# Begin Source File

SOURCE=.\src\sequencer.h
# End Source File
# Begin Source File

SOURCE=.\src\silentdriver.h
# End Source File
# Begin Source File

SOURCE=.\src\song.h
# End Source File
# Begin Source File

SOURCE=.\src\songio.h
# End Source File
# Begin Source File

SOURCE=.\src\vstplugin.h
# End Source File
# Begin Source File

SOURCE=.\src\waveio.h
# End Source File
# Begin Source File

SOURCE=.\src\waveosc.h
# End Source File
# Begin Source File

SOURCE=.\src\waveosctables.h
# End Source File
# Begin Source File

SOURCE=.\src\wavsongio.h
# End Source File
# Begin Source File

SOURCE=.\src\wire.h
# End Source File
# Begin Source File

SOURCE=.\src\xm.h
# End Source File
# Begin Source File

SOURCE=.\src\xmdefs.h
# End Source File
# End Group
# End Target
# End Project
