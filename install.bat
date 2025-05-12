@echo off
REM Install script for Lua Module System
echo Installing Lua Module System...

REM Create required directories
echo Creating directories...
if not exist "%~dp0..\..\Data\SKSE\Plugins\Scripts" mkdir "%~dp0..\..\Data\SKSE\Plugins\Scripts"
if not exist "%~dp0..\..\Data\SKSE\Plugins\LuaModules" mkdir "%~dp0..\..\Data\SKSE\Plugins\LuaModules"

REM Copy the DLL
echo Copying plugin DLL...
copy /Y "%~dp0build\release\HelloLua.dll" "%~dp0..\..\Data\SKSE\Plugins\"

REM Copy example scripts
echo Copying example scripts...
copy /Y "%~dp0Scripts\*.lua" "%~dp0..\..\Data\SKSE\Plugins\Scripts\"

REM Copy module template to modules folder
echo Copying module template...
copy /Y "%~dp0docs\ModuleTemplate.lua" "%~dp0..\..\Data\SKSE\Plugins\LuaModules\"

echo Lua Module System installation completed!
pause
