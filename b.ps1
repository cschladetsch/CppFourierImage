#!/usr/bin/env pwsh
#Requires -Version 7
# Build script for CppFourier

[CmdletBinding(PositionalBinding = $false)]
param(
    [Alias('d')][switch]$DebugBuild,
    [Alias('r')][switch]$Release,
    [Alias('c')][switch]$Clean,
    [Alias('j')][int]$Jobs = [Environment]::ProcessorCount,
    [switch]$Gcc,
    [Alias('h')][switch]$Help
)

if ($Help) {
    Write-Host 'Usage: ./b.ps1 [options]'
    Write-Host 'Options:'
    Write-Host '  -DebugBuild, -d   Build in Debug mode'
    Write-Host '  -Release, -r      Build in Release mode (default)'
    Write-Host '  -Clean, -c        Clean build directory before building'
    Write-Host '  -Jobs, -j N       Use N parallel jobs for building'
    Write-Host '  -Gcc              Use g++ instead of clang++ (default)'
    Write-Host '  -Help, -h         Show this help message'
    exit 0
}

$BuildType = if ($DebugBuild -and -not $Release) { 'Debug' } else { 'Release' }
$Compiler  = if ($Gcc) { 'g++' } else { 'clang++' }
$CCompiler = if ($Gcc) { 'gcc' } else { 'clang' }
$ExeName   = if ($IsWindows) { 'fourier_viewer.exe' } else { 'fourier_viewer' }

Push-Location $PSScriptRoot
try {
    if ($Clean) {
        Write-Host 'Cleaning build directory...' -ForegroundColor Yellow
        if (Test-Path build) { Remove-Item -Recurse -Force build }
    }

    Write-Host "Building CppFourier in $BuildType mode with $Jobs jobs using $Compiler" -ForegroundColor Green

    if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
        Write-Host 'Error: ninja build system not found' -ForegroundColor Red
        Write-Host 'Please install ninja-build (e.g. sudo apt-get install ninja-build)'
        exit 1
    }

    if (-not (Get-Command $Compiler -ErrorAction SilentlyContinue)) {
        Write-Host "Error: $Compiler not found" -ForegroundColor Red
        Write-Host 'Install it, or use -Gcc to build with g++'
        exit 1
    }

    Write-Host 'Configuring with CMake...' -ForegroundColor Yellow
    cmake -S . -B build `
        "-DCMAKE_BUILD_TYPE=$BuildType" `
        "-DCMAKE_CXX_COMPILER=$Compiler" `
        "-DCMAKE_C_COMPILER=$CCompiler" `
        -G Ninja
    if ($LASTEXITCODE -ne 0) {
        Write-Host 'Configuration failed' -ForegroundColor Red
        exit 1
    }
    Write-Host 'Configuration successful' -ForegroundColor Green

    Write-Host 'Building...' -ForegroundColor Yellow
    ninja -C build "-j$Jobs"
    if ($LASTEXITCODE -ne 0) {
        Write-Host 'Build failed' -ForegroundColor Red
        exit 1
    }
    Write-Host 'Build successful' -ForegroundColor Green

    if (Test-Path (Join-Path Bin $ExeName)) {
        Write-Host "Executable created: Bin/$ExeName" -ForegroundColor Green
    } else {
        Write-Host 'Warning: Executable not found' -ForegroundColor Red
        exit 1
    }

    Write-Host 'Build complete!' -ForegroundColor Green
} finally {
    Pop-Location
}
