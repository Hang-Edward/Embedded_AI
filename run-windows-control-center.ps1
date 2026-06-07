param()

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe = Join-Path $projectRoot "dist\windows_control_center\embedded_ai_control_center.exe"

if (-not (Test-Path $exe)) {
    Write-Host "Windows Control Center executable was not found." -ForegroundColor Yellow
    Write-Host "Build it first:"
    Write-Host "  powershell -ExecutionPolicy Bypass -File .\scripts\build-windows-control-center.ps1"
    exit 1
}

Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe)
