#!/bin/bash

# Install dependencies for CppFourier on Ubuntu/Debian

echo "Installing CppFourier dependencies..."

# Update package list
sudo apt-get update

# Install build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    python3

# Install OpenGL and windowing libraries
sudo apt-get install -y \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxi-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxxf86vm-dev

# Install FFTW3
sudo apt-get install -y libfftw3-dev

# Install image libraries
sudo apt-get install -y \
    libpng-dev \
    libjpeg-dev

# Install GTest
sudo apt-get install -y libgtest-dev

echo "Dependencies installed successfully!"
echo ""
echo "Note: If you're using WSL2, make sure you have:"
echo "1. An X server installed (VcXsrv, Xming, etc.)"
echo "2. WSLg enabled or DISPLAY environment variable set"