param()

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe = Join-Path $projectRoot "dist\windows_control_center\embedded_ai_control_center.exe"

if (-not (Test-Path $exe)) {
    Write-Host "Windows Control Center executable was not found:" -ForegroundColor Yellow
    Write-Host "  $exe"
    Write-Host ""
    Write-Host "This repository is expected to include a ready-to-run distribution under:" -ForegroundColor Yellow
    Write-Host "  dist\windows_control_center\"
    Write-Host ""
    Write-Host "If your local copy does not contain that folder, please first make sure you obtained the complete project package." -ForegroundColor Yellow
    Write-Host "Only rebuild when you intentionally want to regenerate the Windows distribution:" -ForegroundColor Yellow
    Write-Host "  powershell -ExecutionPolicy Bypass -File .\scripts\build-windows-control-center.ps1"
    exit 1
}

Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe)
