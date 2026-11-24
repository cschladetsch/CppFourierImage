#!/usr/bin/env pwsh
#Requires -Version 7
# Install dependencies for CppFourier on Ubuntu/Debian

if (-not $IsLinux -or -not (Get-Command apt-get -ErrorAction SilentlyContinue)) {
    Write-Host 'This script installs packages with apt-get and only works on Ubuntu/Debian (including WSL2).' -ForegroundColor Red
    exit 1
}

function Install-Packages([string[]]$Packages) {
    sudo apt-get install -y @Packages
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Failed to install: $($Packages -join ' ')" -ForegroundColor Red
        exit 1
    }
}

Write-Host 'Installing CppFourier dependencies...'

# Update package list
sudo apt-get update

# Install build tools
Install-Packages build-essential, cmake, git, git-lfs, python3

# Install OpenGL and windowing libraries
Install-Packages libgl1-mesa-dev, libglu1-mesa-dev, libx11-dev, libxi-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev, libxxf86vm-dev

# Install Intel TBB (Threading Building Blocks)
Install-Packages libtbb-dev

# Install image libraries
Install-Packages libpng-dev, libjpeg-dev

# Install GTest
Install-Packages libgtest-dev

Write-Host 'Dependencies installed successfully!'
Write-Host ''
Write-Host "Note: If you're using WSL2, make sure you have:"
Write-Host '1. An X server installed (VcXsrv, Xming, etc.)'
Write-Host '2. WSLg enabled or DISPLAY environment variable set'
