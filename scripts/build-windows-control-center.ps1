param(
    [string]$BuildDir = "build-qt",
    [string]$DistDir = "dist\windows_control_center"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$msysShell = Join-Path $env:USERPROFILE "scoop\apps\msys2\current\msys2_shell.cmd"
$ucrtBin = Join-Path $env:USERPROFILE "scoop\apps\msys2\current\ucrt64\bin"
$buildPath = Join-Path $projectRoot $BuildDir
$distPath = Join-Path $projectRoot $DistDir
$deployTool = Join-Path $ucrtBin "windeployqt6.exe"

function Convert-ToMsysPath {
    param([Parameter(Mandatory = $true)][string]$Path)

    $normalized = $Path -replace "\\", "/"
    if ($normalized -match "^([A-Za-z]):/(.*)$") {
        return "/" + $matches[1].ToLower() + "/" + $matches[2]
    }
    return $normalized
}

if (-not (Test-Path $msysShell)) {
    throw "MSYS2 shell was not found: $msysShell"
}

if (-not (Test-Path $deployTool)) {
    throw "Qt deploy tool was not found: $deployTool. Install mingw-w64-ucrt-x86_64-qt6-base first."
}

Write-Host "Project: $projectRoot"
Write-Host "Build directory: $buildPath"
Write-Host "Dist directory: $distPath"

Get-Process embedded_ai_control_center -ErrorAction SilentlyContinue | Stop-Process -Force

$escapedProject = Convert-ToMsysPath $projectRoot

& $msysShell -defterm -no-start -ucrt64 -c "cd '$escapedProject' && cmake -S . -B '$BuildDir' -G Ninja -DBUILD_WINDOWS_CONTROL_CENTER=ON -DBUILD_LEGACY_IMGUI_GUI=OFF && cmake --build '$BuildDir'"
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$exe = Join-Path $buildPath "software\windows_control_center\embedded_ai_control_center.exe"
if (-not (Test-Path $exe)) {
    throw "Build finished, but the Qt executable was not produced: $exe"
}

New-Item -ItemType Directory -Force -Path $distPath | Out-Null
Get-ChildItem -LiteralPath $distPath -Force | Remove-Item -Recurse -Force
Copy-Item $exe $distPath -Force

$distExe = Join-Path $distPath "embedded_ai_control_center.exe"
& $deployTool --release --no-translations $distExe
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

# windeployqt handles Qt plugins, but MSYS2/MinGW runtime and plugin-side
# dependencies still need to sit beside the executable for double-click launch.
# Scan both the main executable and deployed plugin DLLs, otherwise plugins such
# as imageformats/qjpeg.dll may load but fail at runtime because libjpeg is absent.
$deployBinaries = @($distExe) + @(Get-ChildItem -LiteralPath $distPath -Recurse -File -Filter *.dll | ForEach-Object { $_.FullName })
$dllNames = New-Object System.Collections.Generic.HashSet[string]

foreach ($binary in $deployBinaries) {
    $escapedBinary = Convert-ToMsysPath $binary
    $lddOutput = & $msysShell -defterm -no-start -ucrt64 -c "ldd '$escapedBinary'"
    if ($LASTEXITCODE -ne 0) {
        throw "ldd dependency scan failed for $binary."
    }
    foreach ($line in $lddOutput) {
        if ($line -match "=> /ucrt64/bin/([^ ]+\.dll)") {
            [void]$dllNames.Add($matches[1])
        }
    }
}

$copied = 0
foreach ($dllName in $dllNames) {
    $source = Join-Path $ucrtBin $dllName
    $target = Join-Path $distPath $dllName
    if ((Test-Path $source) -and -not (Test-Path $target)) {
        Copy-Item $source $distPath -Force
        ++$copied
    }
}

Write-Host "Copied MSYS2 runtime DLLs: $copied newly copied, $($dllNames.Count) required"

Write-Host ""
Write-Host "Built and deployed:" -ForegroundColor Green
Write-Host "  $distExe"
