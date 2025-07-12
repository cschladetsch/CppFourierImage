#!/bin/bash

# Build script for CppFourier

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default build type
BUILD_TYPE="Release"

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
        -h|--help)
            echo "Usage: ./b [options]"
            echo "Options:"
            echo "  -d, --debug     Build in Debug mode"
            echo "  -r, --release   Build in Release mode (default)"
            echo "  -c, --clean     Clean build directory before building"
            echo "  -j, --jobs N    Use N parallel jobs for building"
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

echo -e "${GREEN}Building CppFourier in ${BUILD_TYPE} mode with ${JOBS} jobs${NC}"

# Create build directory if it doesn't exist
mkdir -p build

# Change to build directory
cd build || exit 1

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
if cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..; then
    echo -e "${GREEN}Configuration successful${NC}"
else
    echo -e "${RED}Configuration failed${NC}"
    exit 1
fi

# Build
echo -e "${YELLOW}Building...${NC}"
if make -j${JOBS}; then
    echo -e "${GREEN}Build successful${NC}"
    
    # Check if executable exists
    if [ -f "fourier_viewer" ]; then
        echo -e "${GREEN}Executable created: build/fourier_viewer${NC}"
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