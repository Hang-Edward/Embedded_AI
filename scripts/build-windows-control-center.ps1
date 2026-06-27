param(
    [string]$BuildDir = "build-qt",
    [string]$DistDir = "dist\windows_control_center"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$msysShell = Join-Path $env:USERPROFILE "scoop\apps\msys2\current\msys2_shell.cmd"
$ucrtBin = Join-Path $env:USERPROFILE "scoop\apps\msys2\current\ucrt64\bin"
$ucrtRoot = Join-Path $env:USERPROFILE "scoop\apps\msys2\current\ucrt64"
$buildPath = Join-Path $projectRoot $BuildDir
$distPath = Join-Path $projectRoot $DistDir
$deployTool = Join-Path $ucrtBin "windeployqt6.exe"
$webView2Loader = Join-Path $projectRoot "third_party\webview2\1.0.4022.49\build\native\x64\WebView2Loader.dll"

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
# 保留部署目录并覆盖同名文件，避免构建脚本清空用户可能放入的辅助文件。
Copy-Item $exe $distPath -Force

$distExe = Join-Path $distPath "embedded_ai_control_center.exe"
& $deployTool --release --no-translations $distExe
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

if (-not (Test-Path $webView2Loader)) {
    throw "WebView2 loader DLL was not found: $webView2Loader"
}
Copy-Item $webView2Loader $distPath -Force

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

# 某些 MSYS2 windeployqt 版本会在扫描阶段保留部署目录中的旧主程序。
# 最后再次覆盖 exe，并以哈希校验，确保启动的一定是本次构建版本。
Copy-Item $exe $distExe -Force
Start-Sleep -Milliseconds 250
$buildHash = (Get-FileHash -Algorithm SHA256 $exe).Hash
$distHash = (Get-FileHash -Algorithm SHA256 $distExe).Hash
if ($buildHash -ne $distHash) {
    # 中文注释：个别环境里 windeployqt 对主程序句柄释放稍慢，这里重试一次，避免误报失败。
    Start-Sleep -Milliseconds 500
    Copy-Item $exe $distExe -Force
    Start-Sleep -Milliseconds 250
    $distHash = (Get-FileHash -Algorithm SHA256 $distExe).Hash
}
if ($buildHash -ne $distHash) {
    throw "Deployment verification failed: built and deployed executable hashes differ after retry."
}

Write-Host "Copied MSYS2 runtime DLLs: $copied newly copied, $($dllNames.Count) required"

Write-Host ""
Write-Host "Built and deployed:" -ForegroundColor Green
Write-Host "  $distExe"
