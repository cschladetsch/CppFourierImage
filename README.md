# CppFourier - Fourier Image Analyzer

A C++23 application that visualizes images using Fourier transform decomposition. The application allows you to load black and white images and visualize them as a sum of sinusoidal components, with an interactive slider to control the number of frequency components used in the reconstruction.

## Features

- Load black and white images using CImg library
- Perform 2D Fourier Transform using FFTW3
- Interactive frequency slider to control reconstruction quality
- Real-time visualization of Fourier reconstruction
- Side-by-side comparison of original and reconstructed images
- Animation mode to see progressive frequency addition
- Display frequency circles and phase information
- Modern C++23 implementation
- Cross-platform support (Linux, Windows, macOS)

## Requirements

### System Dependencies

- C++23 compatible compiler (GCC 12+, Clang 15+, or MSVC 2022+)
- CMake 3.20 or higher
- OpenGL 3.3+
- FFTW3 library
- PNG and JPEG libraries (optional, for image format support)

### Ubuntu/Debian Installation

```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
sudo apt-get install libfftw3-dev
sudo apt-get install libpng-dev libjpeg-dev
sudo apt-get install libgtest-dev
```

### Fedora Installation

```bash
sudo dnf install gcc-c++ cmake git
sudo dnf install fftw-devel
sudo dnf install libpng-devel libjpeg-devel
sudo dnf install gtest-devel
```

### Windows (WSL2)

For WSL2 users, follow the Ubuntu/Debian instructions above. Make sure you have WSLg enabled for GUI support.

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/CppFourier.git
cd CppFourier

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Run tests (optional)
ctest
```

## Usage

```bash
./fourier_viewer
```

### Controls

1. **File Menu**
   - Open Image: Load a black and white image (PNG, JPEG, BMP supported)
   - Exit: Close the application

2. **Frequency Slider**
   - Adjust the number of frequency components used in reconstruction
   - Range: 1 to maximum frequencies (depends on image size)

3. **Display Options**
   - Show Original: Toggle original image display
   - Show Fourier Reconstruction: Toggle reconstructed image display
   - Show Frequency Circles: Visualize frequency components as circles
   - Show Phase Information: Display phase data

4. **Animation**
   - Enable animation to see progressive frequency addition
   - Adjust animation speed

## How It Works

The application performs a 2D Discrete Fourier Transform (DFT) on the input image, converting it from the spatial domain to the frequency domain. The image is then reconstructed by summing sinusoidal components (frequencies) back together. 

By limiting the number of frequencies used in reconstruction, you can see how images can be approximated using fewer components, demonstrating the principle of frequency-based compression.

## Architecture

- **ImageLoader**: Loads images using CImg library
- **ComplexImage**: Represents images in complex number format
- **FourierTransform**: Performs forward and inverse FFT using FFTW3
- **FourierVisualizer**: Generates visualization data for rendering
- **Renderer**: OpenGL-based rendering of images and visualizations
- **UIManager**: ImGui-based user interface

## Testing

The project includes comprehensive unit tests using Google Test framework:

```bash
cd build
ctest --verbose
```

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [FFTW3](http://www.fftw.org/) - Fast Fourier Transform library
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI library
- [CImg](http://cimg.eu/) - C++ image processing library
- [GLFW](https://www.glfw.org/) - OpenGL window management

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.