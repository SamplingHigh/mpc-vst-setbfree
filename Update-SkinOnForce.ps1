param(
    [string]$ForceHost = $env:FORCE_HOST,
    [string]$ForceUser = "root",
    [string]$ForcePassword = $env:FORCE_PASSWORD,
    [string]$WslDistro = "Ubuntu-24.04"
)

$ErrorActionPreference = "Stop"
$project = $PSScriptRoot
$logFile = Join-Path $project "Update-SkinOnForce.log"
try {
    Start-Transcript -LiteralPath $logFile -Force | Out-Null
}
catch {
    # Logging must never prevent a skin update.
}
$hostFile = Join-Path $project ".force-host"
$passwordFile = Join-Path $project ".force-password"
$hostKeyFile = Join-Path $project ".force-hostkey"
if (-not $ForceHost -and (Test-Path -LiteralPath $hostFile)) {
    $ForceHost = (Get-Content -Raw -LiteralPath $hostFile).Trim()
}
if (-not $ForceHost) {
    $ForceHost = "force.local"
}
if (-not $ForcePassword -and (Test-Path -LiteralPath $passwordFile)) {
    $ForcePassword = (Get-Content -Raw -LiteralPath $passwordFile).Trim()
}
$skinName = "Force Runtime - VST - setBfree Organ"
$skinRoot = Join-Path $project "build\skin\$skinName"
$archive = Join-Path $project "build\setbfree-skin-update.tgz"
$remoteScript = Join-Path $project "update-skin-remote.sh"
$remoteArchive = "/tmp/setbfree-skin-update.tgz"

Write-Host "Generating the skin only..."
& wsl.exe -d $WslDistro --cd $project -- bash ./build-skin.sh
if ($LASTEXITCODE -ne 0) {
    throw "Skin generation failed. See $logFile for the complete output."
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

$winScpCandidates = @(
    (Get-Command WinSCP.com -ErrorAction SilentlyContinue).Source,
    "C:\Program Files\WinSCP\WinSCP.com",
    "C:\Program Files (x86)\WinSCP\WinSCP.com",
    (Join-Path $env:LOCALAPPDATA "Programs\WinSCP\WinSCP.com")
)
$winScp = $winScpCandidates | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -First 1

if ($ForcePassword -and $winScp) {
    if (-not (Test-Path -LiteralPath $hostKeyFile)) {
        throw "Automatic login needs the trusted Force fingerprint in $hostKeyFile."
    }
    $hostKey = (Get-Content -Raw -LiteralPath $hostKeyFile).Trim()
    $encodedUser = [Uri]::EscapeDataString($ForceUser)
    $encodedPassword = [Uri]::EscapeDataString($ForcePassword)
    $winScpScript = Join-Path $project "build\winscp-skin-update.txt"
    @(
        "option batch abort"
        "option confirm off"
        "open scp://${encodedUser}:${encodedPassword}@${ForceHost}/ -hostkey=`"$hostKey`""
        "put `"$archive`" /tmp/"
        "put `"$remoteScript`" /tmp/"
        "call sh /tmp/update-skin-remote.sh"
        "exit"
    ) | Set-Content -LiteralPath $winScpScript -Encoding ASCII
    try {
        Write-Host "Uploading and installing the skin with the saved Force login..."
        & $winScp "/ini=nul" "/script=$winScpScript"
        if ($LASTEXITCODE -ne 0) {
            throw "Automatic WinSCP skin update failed."
        }
    }
    finally {
        Remove-Item -LiteralPath $winScpScript -Force -ErrorAction SilentlyContinue
    }
}
else {
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
}

Write-Host "Done. Only the skin was replaced; the plugin binary and MPC.settings were not changed."
try {
    Stop-Transcript | Out-Null
}
catch {
}
