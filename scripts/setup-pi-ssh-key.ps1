param(
    [string]$HostAddress = "172.20.10.6",
    [string]$User = "ch"
)

$ErrorActionPreference = "Stop"

$publicKeyPath = Join-Path $env:USERPROFILE ".ssh\id_ed25519.pub"
if (-not (Test-Path $publicKeyPath)) {
    throw "Public key was not found: $publicKeyPath"
}

$publicKey = Get-Content -Raw -Path $publicKeyPath
$target = "$User@$HostAddress"

Write-Host "Target: $target"
Write-Host "Public key: $publicKeyPath"
Write-Host ""
Write-Host "You may be asked for the Raspberry Pi password once."
Write-Host "The password is typed into SSH directly and will not be shown in Codex."
Write-Host ""

$remoteCommand = @"
umask 077
mkdir -p ~/.ssh
touch ~/.ssh/authorized_keys
grep -qxF '$publicKey' ~/.ssh/authorized_keys || printf '%s\n' '$publicKey' >> ~/.ssh/authorized_keys
chmod 700 ~/.ssh
chmod 600 ~/.ssh/authorized_keys
echo embedded-ai-key-installed
"@

$publicKey | ssh $target $remoteCommand

Write-Host ""
Write-Host "Done. Testing key-based login..."
ssh -o BatchMode=yes -o ConnectTimeout=3 $target "echo embedded-ai-ok"

Write-Host ""
Write-Host "SSH key login is ready."
