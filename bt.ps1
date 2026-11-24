#!/usr/bin/env pwsh
#Requires -Version 7
# Configure, build, and run the unit tests

Push-Location $PSScriptRoot
try {
    cmake -S . -B build -DBUILD_TESTS=ON
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    cmake --build build
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    # -C is needed by multi-config generators (e.g. Visual Studio); single-config ones ignore it
    ctest --test-dir build -C Debug --output-on-failure
    exit $LASTEXITCODE
} finally {
    Pop-Location
}
