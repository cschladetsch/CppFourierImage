#!/usr/bin/env pwsh
#Requires -Version 7
# Run script for CppFourier - builds and runs the application
# Arguments not matched below (or after --) are passed to the application.

[CmdletBinding(PositionalBinding = $false)]
param(
    [Alias('b')][switch]$BuildOnly,
    [Alias('n')][switch]$NoBuild,
    [Alias('d')][switch]$DebugBuild,
    [Alias('c')][switch]$Clean,
    [Alias('j')][int]$Jobs = 0,
    [Alias('h')][switch]$Help,
    [Parameter(ValueFromRemainingArguments)][string[]]$AppArgs = @()
)

if ($Help) {
    Write-Host 'Usage: ./r.ps1 [options] [-- app_args]'
    Write-Host 'Options:'
    Write-Host "  -BuildOnly, -b    Only build, don't run"
    Write-Host '  -NoBuild, -n      Skip building, just run'
    Write-Host '  -DebugBuild, -d   Build in Debug mode'
    Write-Host '  -Clean, -c        Clean before building'
    Write-Host '  -Jobs, -j N       Use N parallel jobs for building'
    Write-Host '  -Help, -h         Show this help message'
    Write-Host ''
    Write-Host 'Arguments after -- are passed to the application'
    Write-Host 'Example: ./r.ps1 -d -- --size 1024'
    Write-Host ''
    Write-Host 'Application options:'
    Write-Host '  --size N    Set maximum image size for processing (default: 512)'
    exit 0
}

if ($BuildOnly -and $NoBuild) {
    Write-Host 'Error: Cannot use -BuildOnly and -NoBuild together' -ForegroundColor Red
    exit 1
}

Push-Location $PSScriptRoot
try {
    if (-not $NoBuild) {
        Write-Host 'Building CppFourier...' -ForegroundColor Blue
        $buildArgs = @{}
        if ($DebugBuild) { $buildArgs.DebugBuild = $true }
        if ($Clean)      { $buildArgs.Clean = $true }
        if ($Jobs -gt 0) { $buildArgs.Jobs = $Jobs }

        & (Join-Path $PSScriptRoot 'b.ps1') @buildArgs
        if ($LASTEXITCODE -ne 0) {
            Write-Host 'Build failed' -ForegroundColor Red
            exit 1
        }
        Write-Host 'Build successful' -ForegroundColor Green
    }

    if ($BuildOnly) { exit 0 }

    $ExeName = if ($IsWindows) { 'fourier_viewer.exe' } else { 'fourier_viewer' }
    $Exe = Join-Path Bin $ExeName
    if (-not (Test-Path $Exe)) {
        Write-Host "Error: Executable not found at Bin/$ExeName" -ForegroundColor Red
        Write-Host 'Try running without -NoBuild' -ForegroundColor Yellow
        exit 1
    }

    # For WSL2, check and set display
    if ($IsLinux -and (Test-Path /proc/version) -and (Select-String -Quiet -Pattern microsoft -Path /proc/version)) {
        Write-Host 'Detected WSL2 environment' -ForegroundColor Yellow

        if (-not $env:DISPLAY) {
            Write-Host 'Setting DISPLAY=:0' -ForegroundColor Yellow
            $env:DISPLAY = ':0'
        }

        xset q *> $null
        if ($LASTEXITCODE -ne 0) {
            Write-Host 'Warning: Cannot connect to X server' -ForegroundColor Red
            Write-Host 'Make sure your X server (VcXsrv, Xming, etc.) is running' -ForegroundColor Yellow
            Write-Host "And that 'Disable access control' is enabled" -ForegroundColor Yellow
        }
    }

    Write-Host 'Running CppFourier...' -ForegroundColor Blue
    Write-Host "Arguments: $AppArgs" -ForegroundColor Yellow
    Write-Host '----------------------------------------'

    & $Exe @AppArgs
    $ExitCode = $LASTEXITCODE

    Write-Host '----------------------------------------'
    if ($ExitCode -eq 0) {
        Write-Host 'Application exited successfully' -ForegroundColor Green
    } else {
        Write-Host "Application exited with code: $ExitCode" -ForegroundColor Red
    }
    exit $ExitCode
} finally {
    Pop-Location
}
