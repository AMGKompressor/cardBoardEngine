@echo off
setlocal
set "OUT=%~1"
set "ROOT=%~2"
if "%OUT%"=="" goto :usage
if "%ROOT%"=="" goto :usage

REM Primary art folder: %ROOT%assets\textures (loaded at runtime via ../assets/textures from game\)
if not exist "%ROOT%assets\textures\board8x8.png" (
  echo Post-build ERROR: missing %ROOT%assets\textures\ — pull the full repo / correct branch.
  exit /b 1
)

REM Optional mirror beside the exe (fallback if ../assets is unreachable)
if not exist "%OUT%textures\" mkdir "%OUT%textures\"
xcopy /Y /I "%ROOT%assets\textures\*.png" "%OUT%textures\"

if exist "%ROOT%assets\sounds\" (
  if not exist "%OUT%sounds\" mkdir "%OUT%sounds\"
  xcopy /Y /I "%ROOT%assets\sounds\*.*" "%OUT%sounds\"
)

if not exist "%OUT%shaders\" mkdir "%OUT%shaders\"
xcopy /Y /I "%ROOT%game\shaders\*.*" "%OUT%shaders\"
if errorlevel 1 (
  echo Post-build ERROR: could not copy game\shaders to "%OUT%shaders\"
  exit /b 1
)

if exist "%ROOT%lib\SDL2-2.0.22\lib\x86\SDL2.dll" (
  xcopy /Y "%ROOT%lib\SDL2-2.0.22\lib\x86\*.dll" "%OUT%"
) else (
  echo Post-build WARNING: lib\SDL2-2.0.22\lib\x86\ not found — copy SDL2.dll next to the exe manually.
)

if exist "%ROOT%lib\SDL2_image-2.6.1\lib\x86\SDL2_image.dll" (
  xcopy /Y "%ROOT%lib\SDL2_image-2.6.1\lib\x86\*.dll" "%OUT%"
) else (
  echo Post-build WARNING: SDL2_image DLLs missing — PNG loading will fail at runtime.
)

if exist "%ROOT%lib\glew-2.1.0\lib\x86\glew32.dll" (
  xcopy /Y "%ROOT%lib\glew-2.1.0\lib\x86\glew32.dll" "%OUT%"
)

if exist "%ROOT%lib\FMOD\lib\x86\fmod.dll" (
  xcopy /Y "%ROOT%lib\FMOD\lib\x86\fmod.dll" "%OUT%"
) else (
  echo Post-build WARNING: lib\FMOD\lib\x86\fmod.dll missing — copy it from the FMOD SDK next to the exe.
)

exit /b 0

:usage
echo Usage: win32_deploy_game_assets.cmd OUT_DIR PROJECT_DIR
exit /b 1
