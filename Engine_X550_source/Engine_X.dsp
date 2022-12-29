# Microsoft Developer Studio Project File - Name="Engine_X" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Engine_X - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Engine_X.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Engine_X.mak" CFG="Engine_X - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Engine_X - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Engine_X - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Engine_X - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_MSVC6"
# PROP BASE Intermediate_Dir "Release_MSVC6"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_MSVC6"
# PROP Intermediate_Dir "Release_MSVC6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /I ".\dxsdk\sdk8\include" /D "NDEBUG" /D "GLQUAKE" /D "WIN32" /D "_WINDOWS" /Fr /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /O2 /I "./dxsdk/sdk8/include" /D "NDEBUG" /D "GLQUAKE" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comctl32.lib dsound.lib dxguid.lib gdi32.lib libcurl.lib kernel32.lib ole32.lib oleaut32.lib opengl32.lib shell32.lib strmiids.lib user32.lib winmm.lib wsock32.lib libpng.lib zlib.lib libjpeg.lib /nologo /subsystem:windows /profile /map /machine:I386 /out:"..\dx8_Engine_X_473.exe" /libpath:".\fmod" /libpath:".\curl" /libpath:".\zlib" /libpath:".\png" /libpath:".\jpeg" /libpath:".\dxsdk\sdk8\lib"
# ADD LINK32 advapi32.lib gdi32.lib kernel32.lib ole32.lib oleaut32.lib shell32.lib user32.lib winmm.lib wsock32.lib /nologo /subsystem:windows /profile /map /machine:I386 /out:"..\Engine_X_550.exe"

!ELSEIF  "$(CFG)" == "Engine_X - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Debug_MSVC6"
# PROP BASE Intermediate_Dir "Debug_MSVC6"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug_MSVC6"
# PROP Intermediate_Dir "Debug_MSVC6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /ZI /Od /I ".\dxsdk\sdk8\include" /I ".\png" /I ".\zlib" /I ".\fmod" /I ".\curl" /I ".\jpeg" /I ".\dxsdk\sdk7\include" /D "NDEBUG" /D "GLQUAKE" /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /Fr /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /ZI /Od /I "./dxsdk/sdk8/include" /D "GLQUAKE" /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comctl32.lib dsound.lib dxguid.lib gdi32.lib libcurl.lib kernel32.lib ole32.lib oleaut32.lib opengl32.lib shell32.lib strmiids.lib user32.lib winmm.lib wsock32.lib libpng.lib zlib.lib libjpeg.lib Comctl32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"..\dx8_Engine_X_473_dbg.exe" /libpath:".\fmod" /libpath:".\curl" /libpath:".\zlib" /libpath:".\png" /libpath:".\jpeg" /libpath:".\dxsdk\sdk8\lib"
# ADD LINK32 advapi32.lib gdi32.lib kernel32.lib ole32.lib oleaut32.lib shell32.lib user32.lib winmm.lib wsock32.lib /nologo /subsystem:windows /incremental:yes /map /debug /machine:I386 /out:"..\Engine_X_550_dbg.exe"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Engine_X - Win32 Release"
# Name "Engine_X - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Base"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\engine.h
# End Source File
# Begin Source File

SOURCE=.\gl_local.h
# End Source File
# Begin Source File

SOURCE=.\hwgamma_com.h
# End Source File
# Begin Source File

SOURCE=.\mp3_com.h
# End Source File
# Begin Source File

SOURCE=.\quakedef.h
# End Source File
# Begin Source File

SOURCE=.\renderer_com.h
# End Source File
# Begin Source File

SOURCE=.\sound_com.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# Begin Source File

SOURCE=.\winquake.h
# End Source File
# End Group
# Begin Group "Audio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mp3.c
# End Source File
# Begin Source File

SOURCE=.\mp3_win.c
# End Source File
# Begin Source File

SOURCE=.\mp3audio.h
# End Source File
# Begin Source File

SOURCE=.\snd_dma.c
# End Source File
# Begin Source File

SOURCE=.\snd_mem.c
# End Source File
# Begin Source File

SOURCE=.\snd_mix.c
# End Source File
# Begin Source File

SOURCE=.\snd_win.c
# End Source File
# Begin Source File

SOURCE=.\sound.h
# End Source File
# Begin Source File

SOURCE=.\winquake_sound.h
# End Source File
# End Group
# Begin Group "Client State"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cl_chase.c
# End Source File
# Begin Source File

SOURCE=.\cl_demo.c
# End Source File
# Begin Source File

SOURCE=.\cl_main.c
# End Source File
# Begin Source File

SOURCE=.\cl_sbar.c
# End Source File
# Begin Source File

SOURCE=.\cl_sbar.h
# End Source File
# Begin Source File

SOURCE=.\cl_screen.c
# End Source File
# Begin Source File

SOURCE=.\cl_screen.h
# End Source File
# Begin Source File

SOURCE=.\cl_tent.c
# End Source File
# Begin Source File

SOURCE=.\cl_updatescreen.c
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\com_byteorder.c
# End Source File
# Begin Source File

SOURCE=.\com_filesystem.c
# End Source File
# Begin Source File

SOURCE=.\com_filesystem_closed.c
# End Source File
# Begin Source File

SOURCE=.\com_gamedir.c
# End Source File
# Begin Source File

SOURCE=.\com_messages.c
# End Source File
# Begin Source File

SOURCE=.\com_parse.c
# End Source File
# Begin Source File

SOURCE=.\com_string_fs.c
# End Source File
# Begin Source File

SOURCE=.\com_stringf.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\crc.c
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# End Group
# Begin Group "Cvars And Commands"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cmd.c
# End Source File
# Begin Source File

SOURCE=.\cmd.h
# End Source File
# Begin Source File

SOURCE=.\cvar.c
# End Source File
# Begin Source File

SOURCE=.\cvar.h
# End Source File
# Begin Source File

SOURCE=.\cvar_extensions.c
# End Source File
# Begin Source File

SOURCE=.\cvar_listings.c
# End Source File
# Begin Source File

SOURCE=.\cvar_registration.c
# End Source File
# Begin Source File

SOURCE=.\cvar_registry.c
# End Source File
# Begin Source File

SOURCE=.\cvar_registry.h
# End Source File
# Begin Source File

SOURCE=.\cvar_set.c
# End Source File
# End Group
# Begin Group "Extensions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\nehahra.c
# End Source File
# Begin Source File

SOURCE=.\nehahra.h
# End Source File
# Begin Source File

SOURCE=.\security.c
# End Source File
# Begin Source File

SOURCE=.\security.h
# End Source File
# Begin Source File

SOURCE=.\webdownload.c
# End Source File
# Begin Source File

SOURCE=.\webdownload.h
# End Source File
# End Group
# Begin Group "Host Server VM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bspfile.h
# End Source File
# Begin Source File

SOURCE=.\cl_parse.c
# End Source File
# Begin Source File

SOURCE=.\cl_parse_baseline.c
# End Source File
# Begin Source File

SOURCE=.\cl_parse_clientdata.c
# End Source File
# Begin Source File

SOURCE=.\cl_parse_serverinfo.c
# End Source File
# Begin Source File

SOURCE=.\cl_parse_update.c
# End Source File
# Begin Source File

SOURCE=.\gamehacks.c
# End Source File
# Begin Source File

SOURCE=.\gamehacks.h
# End Source File
# Begin Source File

SOURCE=.\host.c
# End Source File
# Begin Source File

SOURCE=.\host_cmd.c
# End Source File
# Begin Source File

SOURCE=.\pr_cmds.c
# End Source File
# Begin Source File

SOURCE=.\pr_comp.h
# End Source File
# Begin Source File

SOURCE=.\pr_edict.c
# End Source File
# Begin Source File

SOURCE=.\pr_exec.c
# End Source File
# Begin Source File

SOURCE=.\progdefs.h
# End Source File
# Begin Source File

SOURCE=.\progs.h
# End Source File
# Begin Source File

SOURCE=.\protocol.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\sv_main.c
# End Source File
# Begin Source File

SOURCE=.\sv_move.c
# End Source File
# Begin Source File

SOURCE=.\sv_phys.c
# End Source File
# Begin Source File

SOURCE=.\sv_user.c
# End Source File
# Begin Source File

SOURCE=.\sv_world.c
# End Source File
# Begin Source File

SOURCE=.\sv_world.h
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cl_input.c
# End Source File
# Begin Source File

SOURCE=.\in_win.c
# End Source File
# Begin Source File

SOURCE=.\in_win_directinput.c
# End Source File
# Begin Source File

SOURCE=.\in_win_joystick.c
# End Source File
# Begin Source File

SOURCE=.\in_win_keyboard.c
# End Source File
# Begin Source File

SOURCE=.\in_win_mouse.c
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\keys.c
# End Source File
# Begin Source File

SOURCE=.\keys.h
# End Source File
# Begin Source File

SOURCE=.\winquake_input.h
# End Source File
# End Group
# Begin Group "Media"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\draw_shapes_2d.c
# End Source File
# Begin Source File

SOURCE=.\draw_shapes_3d.c
# End Source File
# Begin Source File

SOURCE=.\draw_textures_2d.c
# End Source File
# Begin Source File

SOURCE=.\draw_textures_3d.c
# End Source File
# Begin Source File

SOURCE=.\image.c
# End Source File
# Begin Source File

SOURCE=.\image.h
# End Source File
# Begin Source File

SOURCE=.\insertions_3d.c
# End Source File
# Begin Source File

SOURCE=.\location.c
# End Source File
# Begin Source File

SOURCE=.\location.h
# End Source File
# Begin Source File

SOURCE=.\model.c
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=.\model_alias.c
# End Source File
# Begin Source File

SOURCE=.\model_alias.h
# End Source File
# Begin Source File

SOURCE=.\model_alias_anorm_dots.h
# End Source File
# Begin Source File

SOURCE=.\model_alias_gen.h
# End Source File
# Begin Source File

SOURCE=.\model_alias_mesh.c
# End Source File
# Begin Source File

SOURCE=.\model_alias_render.c
# End Source File
# Begin Source File

SOURCE=.\model_alias_runframe.c
# End Source File
# Begin Source File

SOURCE=.\model_alias_vertex_lights.c
# End Source File
# Begin Source File

SOURCE=.\model_brush.c
# End Source File
# Begin Source File

SOURCE=.\model_brush.h
# End Source File
# Begin Source File

SOURCE=.\model_brush_efrags.c
# End Source File
# Begin Source File

SOURCE=.\model_brush_funcs.c
# End Source File
# Begin Source File

SOURCE=.\model_brush_funcs.h
# End Source File
# Begin Source File

SOURCE=.\model_brush_render.c
# End Source File
# Begin Source File

SOURCE=.\model_brush_render_lighting.c
# End Source File
# Begin Source File

SOURCE=.\model_brush_render_liquids.c
# End Source File
# Begin Source File

SOURCE=.\model_brush_render_sky.c
# End Source File
# Begin Source File

SOURCE=.\model_brush_surface_pointer.c
# End Source File
# Begin Source File

SOURCE=.\model_md3.c
# End Source File
# Begin Source File

SOURCE=.\model_md3.h
# End Source File
# Begin Source File

SOURCE=.\model_md3_render.c
# End Source File
# Begin Source File

SOURCE=.\model_sprite.c
# End Source File
# Begin Source File

SOURCE=.\model_sprite.h
# End Source File
# Begin Source File

SOURCE=.\model_sprite_gen.h
# End Source File
# Begin Source File

SOURCE=.\model_sprite_render.c
# End Source File
# Begin Source File

SOURCE=.\movie.c
# End Source File
# Begin Source File

SOURCE=.\movie.h
# End Source File
# Begin Source File

SOURCE=.\movie_avi.c
# End Source File
# Begin Source File

SOURCE=.\movie_avi.h
# End Source File
# Begin Source File

SOURCE=.\particle_calculations.c
# End Source File
# Begin Source File

SOURCE=.\particle_render.c
# End Source File
# Begin Source File

SOURCE=.\wad.c
# End Source File
# Begin Source File

SOURCE=.\wad.h
# End Source File
# Begin Source File

SOURCE=.\wad3.c
# End Source File
# End Group
# Begin Group "Memory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\memory.c
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\memory_imagework.c
# End Source File
# Begin Source File

SOURCE=.\memory_local.h
# End Source File
# Begin Source File

SOURCE=.\memory_mcache.c
# End Source File
# Begin Source File

SOURCE=.\memory_temp.c
# End Source File
# Begin Source File

SOURCE=.\memory_zone.c
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\iplog.c
# End Source File
# Begin Source File

SOURCE=.\iplog.h
# End Source File
# Begin Source File

SOURCE=.\net.h
# End Source File
# Begin Source File

SOURCE=.\net_dgrm.c
# End Source File
# Begin Source File

SOURCE=.\net_dgrm.h
# End Source File
# Begin Source File

SOURCE=.\net_loop.c
# End Source File
# Begin Source File

SOURCE=.\net_loop.h
# End Source File
# Begin Source File

SOURCE=.\net_main.c
# End Source File
# Begin Source File

SOURCE=.\net_vcr.c
# End Source File
# Begin Source File

SOURCE=.\net_vcr.h
# End Source File
# Begin Source File

SOURCE=.\net_win.c
# End Source File
# Begin Source File

SOURCE=.\net_wins.c
# End Source File
# Begin Source File

SOURCE=.\net_wins.h
# End Source File
# Begin Source File

SOURCE=.\net_wipx.c
# End Source File
# Begin Source File

SOURCE=.\net_wipx.h
# End Source File
# Begin Source File

SOURCE=.\winquake_net.h
# End Source File
# End Group
# Begin Group "Renderer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dx8_fakegl.h
# End Source File
# Begin Source File

SOURCE=.\dx8_gl_fakegl.c
# End Source File
# Begin Source File

SOURCE=.\dx8_libci.c

!IF  "$(CFG)" == "Engine_X - Win32 Release"

!ELSEIF  "$(CFG)" == "Engine_X - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_manager.c
# End Source File
# Begin Source File

SOURCE=.\renderer.c
# End Source File
# Begin Source File

SOURCE=.\renderer.h
# End Source File
# End Group
# Begin Group "Scene"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cl_view.c
# End Source File
# Begin Source File

SOURCE=.\cl_view_blends.c
# End Source File
# Begin Source File

SOURCE=.\gl_frustum.c
# End Source File
# Begin Source File

SOURCE=.\gl_rmain.c
# End Source File
# Begin Source File

SOURCE=.\gl_scene_fog.c
# End Source File
# Begin Source File

SOURCE=.\gl_scene_render.c
# End Source File
# Begin Source File

SOURCE=.\gl_scene_setup.c
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# End Group
# Begin Group "System"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sys.h
# End Source File
# Begin Source File

SOURCE=.\sys_win.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_activate.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_clipboard.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_clock.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_filesystem.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_interact.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_main.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_messages.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_registry.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_term.c
# End Source File
# Begin Source File

SOURCE=.\sys_win_version.c
# End Source File
# Begin Source File

SOURCE=.\version.c
# End Source File
# End Group
# Begin Group "Texture Processing"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gl_palette_gamma.c
# End Source File
# Begin Source File

SOURCE=.\gl_texmgr.c
# End Source File
# Begin Source File

SOURCE=.\gl_texmgr_kill.c
# End Source File
# Begin Source File

SOURCE=.\gl_texmgr_support.c
# End Source File
# Begin Source File

SOURCE=.\gl_texture.h
# End Source File
# Begin Source File

SOURCE=.\gl_texture_scrap.c
# End Source File
# Begin Source File

SOURCE=.\gl_texturing.c
# End Source File
# Begin Source File

SOURCE=.\texture_funcs.c
# End Source File
# Begin Source File

SOURCE=.\texture_funcs.h
# End Source File
# End Group
# Begin Group "User Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\conproc.c
# End Source File
# Begin Source File

SOURCE=.\conproc.h
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\listbox_fixed.c
# End Source File
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# End Group
# Begin Group "Video"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vid.h
# End Source File
# Begin Source File

SOURCE=.\vid_common_gl.c
# End Source File
# Begin Source File

SOURCE=.\vid_gamma.c
# End Source File
# Begin Source File

SOURCE=.\vid_modes.c
# End Source File
# Begin Source File

SOURCE=.\vid_modes_populate.c
# End Source File
# Begin Source File

SOURCE=.\vid_paint_screen.c
# End Source File
# Begin Source File

SOURCE=.\vid_shared.c
# End Source File
# Begin Source File

SOURCE=.\vid_videomodes_menu.c
# End Source File
# Begin Source File

SOURCE=.\vid_wgl.c
# End Source File
# Begin Source File

SOURCE=.\winquake_video_modes.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Engine_X.ico
# End Source File
# Begin Source File

SOURCE=.\Engine_X.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\changelog.txt
# End Source File
# Begin Source File

SOURCE=.\devnotes.txt
# End Source File
# Begin Source File

SOURCE=.\todolist.txt
# End Source File
# End Target
# End Project
