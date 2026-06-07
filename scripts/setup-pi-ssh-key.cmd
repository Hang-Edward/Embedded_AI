@echo off
setlocal

set "PI_USER=ch"
set "PI_HOST=172.20.10.6"
set "PUBLIC_KEY=%USERPROFILE%\.ssh\id_ed25519.pub"

echo Embedded AI Raspberry Pi SSH key setup
echo.
echo Target: %PI_USER%@%PI_HOST%
echo Public key: %PUBLIC_KEY%
echo.

if not exist "%PUBLIC_KEY%" (
    echo ERROR: public key not found.
    echo Run ssh-keygen first or ask Codex to generate the key again.
    echo.
    pause
    exit /b 1
)

echo The next command will ask for the Raspberry Pi password once.
echo When typing the password, the cursor may not move and no characters will appear. This is normal.
echo.

type "%PUBLIC_KEY%" | ssh %PI_USER%@%PI_HOST% "umask 077; mkdir -p ~/.ssh; touch ~/.ssh/authorized_keys; cat >> ~/.ssh/authorized_keys; sort -u ~/.ssh/authorized_keys -o ~/.ssh/authorized_keys; chmod 700 ~/.ssh; chmod 600 ~/.ssh/authorized_keys; echo embedded-ai-key-installed"

echo.
echo Testing key-based login...
ssh -o BatchMode=yes -o ConnectTimeout=3 %PI_USER%@%PI_HOST% "echo embedded-ai-ok"

echo.
if errorlevel 1 (
    echo SSH key login test failed. Please copy the error above and send it to Codex.
) else (
    echo SSH key login is ready.
)
echo.
pause
