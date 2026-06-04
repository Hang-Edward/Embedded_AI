param(
    [string]$Port = "COM11",
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$msysBin = Join-Path $env:USERPROFILE "scoop\apps\msys2\current\ucrt64\bin"
$exe = Join-Path $projectRoot "build-msys2-shell\src\pc\embedded_ai_pc_bridge.exe"

if (-not (Test-Path $exe)) {
    Write-Error "Executable not found: $exe. Build the project first."
}

if (-not (Test-Path $msysBin)) {
    Write-Error "MSYS2 UCRT64 bin directory not found: $msysBin"
}

$env:PATH = "$msysBin;$env:PATH"
& $exe $Port @ExtraArgs
exit $LASTEXITCODE
