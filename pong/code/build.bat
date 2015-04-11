@echo off
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cd
cl  /MTd -nologo -Gm- -GR- -EHa- -Oi -W4 -wd4201 -wd4100 -wd4505 -FC -Z7 -Fm  ..\code\win32_pong.cpp /link -opt:ref User32.lib gdi32.lib winmm.lib

popd