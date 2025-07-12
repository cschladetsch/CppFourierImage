#!/bin/bash

# Run script for CppFourier - builds and runs the application

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_ONLY=0
NO_BUILD=0
BUILD_ARGS=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--build-only)
            BUILD_ONLY=1
            shift
            ;;
        -n|--no-build)
            NO_BUILD=1
            shift
            ;;
        -d|--debug)
            BUILD_ARGS="$BUILD_ARGS -d"
            shift
            ;;
        -c|--clean)
            BUILD_ARGS="$BUILD_ARGS -c"
            shift
            ;;
        -j|--jobs)
            BUILD_ARGS="$BUILD_ARGS -j $2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: ./r [options] [-- app_args]"
            echo "Options:"
            echo "  -b, --build-only    Only build, don't run"
            echo "  -n, --no-build      Skip building, just run"
            echo "  -d, --debug         Build in Debug mode"
            echo "  -c, --clean         Clean before building"
            echo "  -j, --jobs N        Use N parallel jobs for building"
            echo "  -h, --help          Show this help message"
            echo ""
            echo "Arguments after -- are passed to the application"
            echo "Example: ./r -d -- --size 1024"
            echo ""
            echo "Application options:"
            echo "  --size N    Set maximum image size for processing (default: 512)"
            exit 0
            ;;
        --)
            shift
            APP_ARGS="$@"
            break
            ;;
        *)
            # Unknown arguments are passed to the application
            APP_ARGS="$@"
            break
            ;;
    esac
done

# Check for conflicting options
if [ $BUILD_ONLY -eq 1 ] && [ $NO_BUILD -eq 1 ]; then
    echo -e "${RED}Error: Cannot use --build-only and --no-build together${NC}"
    exit 1
fi

# Build if needed
if [ $NO_BUILD -eq 0 ]; then
    echo -e "${BLUE}Building CppFourier...${NC}"
    if ./b $BUILD_ARGS; then
        echo -e "${GREEN}Build successful${NC}"
    else
        echo -e "${RED}Build failed${NC}"
        exit 1
    fi
fi

# Exit if build-only
if [ $BUILD_ONLY -eq 1 ]; then
    exit 0
fi

# Check if executable exists
if [ ! -f "Bin/fourier_viewer" ]; then
    echo -e "${RED}Error: Executable not found at Bin/fourier_viewer${NC}"
    echo -e "${YELLOW}Try running without --no-build option${NC}"
    exit 1
fi

# For WSL2, check and set display
if grep -qi microsoft /proc/version 2>/dev/null; then
    echo -e "${YELLOW}Detected WSL2 environment${NC}"
    
    # Check if DISPLAY is set
    if [ -z "$DISPLAY" ]; then
        echo -e "${YELLOW}Setting DISPLAY=:0${NC}"
        export DISPLAY=:0
    fi
    
    # Check if X server is accessible
    if ! xset q &>/dev/null; then
        echo -e "${RED}Warning: Cannot connect to X server${NC}"
        echo -e "${YELLOW}Make sure your X server (VcXsrv, Xming, etc.) is running${NC}"
        echo -e "${YELLOW}And that 'Disable access control' is enabled${NC}"
    fi
fi

# Run the application
echo -e "${BLUE}Running CppFourier...${NC}"
echo -e "${YELLOW}Arguments: $APP_ARGS${NC}"
echo "----------------------------------------"

# Run with or without arguments from Bin directory
if [ -z "$APP_ARGS" ]; then
    ./Bin/fourier_viewer
else
    ./Bin/fourier_viewer $APP_ARGS
fi

# Capture exit code
EXIT_CODE=$?

# Report exit status
if [ $EXIT_CODE -eq 0 ]; then
    echo "----------------------------------------"
    echo -e "${GREEN}Application exited successfully${NC}"
else
    echo "----------------------------------------"
    echo -e "${RED}Application exited with code: $EXIT_CODE${NC}"
fi

exit $EXIT_CODE