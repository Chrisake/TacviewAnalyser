@echo off
setlocal
set PRESETS=("winx64-msvc-debug" "winx64-msvc-release" "linuxx64-clang-debug" "linuxx64-gcc-release") 
set LOGDIR=log

:: Create the log directory if it doesn't exist
if not exist "%LOGDIR%" (
    mkdir "%LOGDIR%"
)

for %%p in %PRESETS% do (
    set TIMESTAMP=%date:~10,4%-%date:~4,2%-%date:~7,2%_%time:~0,2%-%time:~3,2%-%time:~6,2%
    set TIMESTAMP=%TIMESTAMP: =0%  :: Replace spaces with zero (for times like 9:00)
    cmake --preset=%%p >> "%LOGDIR%/%%p_%TIMESTAMP%.log" 2>&1
    cmake --build . --preset=%%p >> "%LOGDIR%/%%p_%TIMESTAMP%.log" 2>&1
)
endlocal
