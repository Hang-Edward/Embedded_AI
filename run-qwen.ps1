param(
    [string]$Port = "COM11"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$runner = Join-Path $projectRoot "run-pc.ps1"

& $runner -Port $Port -ExtraArgs "--qwen"
exit $LASTEXITCODE
