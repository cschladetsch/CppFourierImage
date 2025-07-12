#!/bin/bash

# Build script for CppFourier

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default build type and compiler
BUILD_TYPE="Release"
COMPILER="clang++"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -c|--clean)
            echo -e "${YELLOW}Cleaning build directory...${NC}"
            rm -rf build
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --gcc)
            COMPILER="g++"
            shift
            ;;
        -h|--help)
            echo "Usage: ./b [options]"
            echo "Options:"
            echo "  -d, --debug     Build in Debug mode"
            echo "  -r, --release   Build in Release mode (default)"
            echo "  -c, --clean     Clean build directory before building"
            echo "  -j, --jobs N    Use N parallel jobs for building"
            echo "  --gcc           Use g++ instead of clang++ (default)"
            echo "  -h, --help      Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Set default number of jobs if not specified
if [ -z "$JOBS" ]; then
    JOBS=$(nproc)
fi

echo -e "${GREEN}Building CppFourier in ${BUILD_TYPE} mode with ${JOBS} jobs using ${COMPILER}${NC}"

# Check if ninja is available
if ! command -v ninja &> /dev/null; then
    echo -e "${RED}Error: ninja build system not found${NC}"
    echo "Please install ninja-build:"
    echo "  sudo apt-get install ninja-build"
    exit 1
fi

# Check if clang++ is available (if using clang)
if [ "$COMPILER" = "clang++" ] && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: clang++ not found${NC}"
    echo "Please install clang:"
    echo "  sudo apt-get install clang"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p build

# Change to build directory
cd build || exit 1

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"

# Determine C compiler based on C++ compiler
if [ "$COMPILER" = "clang++" ]; then
    C_COMPILER="clang"
else
    C_COMPILER="gcc"
fi

if cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
         -DCMAKE_CXX_COMPILER=${COMPILER} \
         -DCMAKE_C_COMPILER=${C_COMPILER} \
         -GNinja ..; then
    echo -e "${GREEN}Configuration successful${NC}"
else
    echo -e "${RED}Configuration failed${NC}"
    exit 1
fi

# Build
echo -e "${YELLOW}Building...${NC}"
if ninja -j${JOBS}; then
    echo -e "${GREEN}Build successful${NC}"
    
    # Check if executable exists in Bin directory
    if [ -f "../Bin/fourier_viewer" ]; then
        echo -e "${GREEN}Executable created: Bin/fourier_viewer${NC}"
    else
        echo -e "${RED}Warning: Executable not found${NC}"
        exit 1
    fi
else
    echo -e "${RED}Build failed${NC}"
    exit 1
fi

# Return to original directory
cd ..

echo -e "${GREEN}Build complete!${NC}"