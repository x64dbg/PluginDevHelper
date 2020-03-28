# PluginDevHelper

Simple utility that allows you to automatically unload/reload an x64dbg plugin while developing with Visual Studio.

## Installation

1. Start `PluginDevServer` and keep it running in the background
2. Install the `PluginDevHelper` plugin
3. Nativate to you plugin's Properties -> Build Events in Visual Studio
4. Pre-Link Event -> Command Line: `if exist "$(ProjectDir)PluginDevBuildTool.exe" ("$(ProjectDir)PluginDevBuildTool.exe" unload "$(TargetPath)")`
5. Post-Build Event -> Command Line: `if exist "$(ProjectDir)PluginDevBuildTool.exe" ("$(ProjectDir)PluginDevBuildTool.exe" reload "$(TargetPath)")`
6. Create a symbolic link ([mklink](https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/mklink) or [Link Shell Extension](https://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html)) to you plugin's output file in x64dbg's `plugins` directory
7. Copy `PluginDevBuildTool.exe` next to your `.vcxproj` file

## Expected behavior

When building the plugin with Visual Studio and the output file is loaded in an open x64dbg instance:

```
1>------ Build started: Project: XmmYmm, Configuration: Release Win32 ------
1>plugin.cpp
1>PluginDevBuildTool: Failed to obtain exclusive access, requesting to unload XmmYmm.dp32
1>PluginDevBuildTool: Exclusive access to XmmYmm.dp32 obtained!
1>   Creating library C:\Projects\XmmYmm\bin\x32\XmmYmm.lib and object C:\Projects\XmmYmm\bin\x32\XmmYmm.exp
1>Generating code
1>Finished generating code
1>XmmYmm.vcxproj -> C:\Projects\XmmYmm\bin\x32\XmmYmm.dp32
1>PluginDevBuildTool: Requested reload of XmmYmm.dp32
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

```

From x64dbg's perspective:

```
[PluginDevHelper] PluginDevHelper: plugunload "XmmYmm"
[PLUGIN] XmmYmm.dp32 unloaded
[PluginDevHelper] PluginDevHelper: plugload "XmmYmm"
[PLUGIN, XmmYmm] Expression function "xmm" registered!
[PLUGIN] XmmYmm v1 Loaded!
```