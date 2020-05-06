@echo off
setlocal EnableDelayedExpansion

set ARCHIVE=NotoSansCJKjp-hinted.zip

if not exist NotoSansCJKjp-hinted.zip (
    curl https://noto-website-2.storage.googleapis.com/pkgs/!ARCHIVE! -o !ARCHIVE!
    if !ERRORLEVEL! neq 0 goto error
    powershell Expand-Archive -Path !ARCHIVE! -DestinationPath NotoSansCJKjp -Force
    if !ERRORLEVEL! neq 0 goto error
)

echo setup Noto fonts successfully
pause
exit /b 0

:error
echo failed to setup Noto fonts
pause
exit /b 1

endlocal
