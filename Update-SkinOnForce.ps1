param(
    [string]$ForceHost = $env:FORCE_HOST,
    [string]$ForceUser = "root",
    [string]$WslDistro = "Ubuntu-24.04"
)

$ErrorActionPreference = "Stop"
$project = $PSScriptRoot
$hostFile = Join-Path $project ".force-host"
if (-not $ForceHost -and (Test-Path -LiteralPath $hostFile)) {
    $ForceHost = (Get-Content -Raw -LiteralPath $hostFile).Trim()
}
if (-not $ForceHost) {
    $ForceHost = "force.local"
}
$skinName = "Force Runtime - VST - setBfree Organ"
$skinRoot = Join-Path $project "build\skin\$skinName"
$archive = Join-Path $project "build\setbfree-skin-update.tgz"
$remoteScript = Join-Path $project "update-skin-remote.sh"
$remoteArchive = "/tmp/setbfree-skin-update.tgz"

Write-Host "Generating the skin only..."
$resolvedProject = (Resolve-Path -LiteralPath $project).Path
if ($resolvedProject -notmatch '^([A-Za-z]):\\(.*)$') {
    throw "The project must be on a Windows drive that WSL mounts below /mnt."
}
$wslProject = "/mnt/$($Matches[1].ToLower())/$($Matches[2].Replace('\', '/'))"
& wsl.exe -d $WslDistro -- bash "$wslProject/build-skin.sh"
if ($LASTEXITCODE -ne 0) {
    throw "Skin generation failed."
}

$pluginSkins = Join-Path $skinRoot "Plugin Skins"
if (-not (Test-Path -LiteralPath (Join-Path $pluginSkins "TUI.json"))) {
    throw "The generated Plugin Skins/TUI.json file is missing."
}

if (Test-Path -LiteralPath $archive) {
    Remove-Item -LiteralPath $archive -Force
}
& tar.exe -czf $archive -C $skinRoot "Plugin Skins"
if ($LASTEXITCODE -ne 0) {
    throw "Could not create the skin archive."
}

Write-Host "Uploading the skin (SSH password may be requested)..."
& scp.exe -O $archive $remoteScript "${ForceUser}@${ForceHost}:/tmp/"
if ($LASTEXITCODE -ne 0) {
    throw "Skin upload failed."
}

Write-Host "Installing the skin and restarting MPC (SSH password may be requested again)..."
& ssh.exe -tt "${ForceUser}@${ForceHost}" "sh /tmp/update-skin-remote.sh"
if ($LASTEXITCODE -ne 0) {
    throw "Remote skin update failed."
}

Write-Host "Done. Only the skin was replaced; the plugin binary and MPC.settings were not changed."
