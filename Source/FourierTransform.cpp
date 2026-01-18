#include <cmath>
#include <algorithm>
#include <numeric>
#include <execution>
#include <ranges>
#include <thread>
#include <future>
#include <utility>

#include "FourierTransform.hpp"
#include "RgbComplexImage.hpp"

FourierTransform::FourierTransform() = default;
FourierTransform::~FourierTransform() = default;

ComplexImage FourierTransform::transform2D(const ComplexImage& input, Direction direction) {
    if (input.getWidth() == 0 || input.getHeight() == 0) {
        return ComplexImage(0, 0);
    }

    ComplexImage transformed = input;
    fft2D(transformed, direction);
    return transformed;
}

void FourierTransform::fft2D(ComplexImage& image, Direction direction) {
    const size_t width = image.getWidth();
    const size_t height = image.getHeight();
    
    const auto process_axis = [&](size_t line_count, size_t line_length, auto coord_mapper) {
        auto line_indices = std::views::iota(0uz, line_count);
        std::for_each(line_indices.begin(), line_indices.end(),
            [&](size_t line_idx) {
                std::vector<ComplexImage::Complex> buffer(line_length);
                auto positions = std::views::iota(0uz, line_length);

                std::ranges::transform(positions, buffer.begin(),
                    [&](size_t pos) {
                        const auto [x, y] = coord_mapper(line_idx, pos);
                        return image.at(x, y);
                    });

                fft1D(buffer, direction);

                std::ranges::for_each(positions,
                    [&](size_t pos) {
                        const auto [x, y] = coord_mapper(line_idx, pos);
                        image.at(x, y) = buffer[pos];
                    });
            });
    };

    process_axis(height, width, [](size_t line, size_t pos) {
        return std::pair<size_t, size_t>{pos, line};
    });

    process_axis(width, height, [](size_t line, size_t pos) {
        return std::pair<size_t, size_t>{line, pos};
    });
}

void FourierTransform::fft1D(std::span<ComplexImage::Complex> data, Direction direction) {
    size_t n = data.size();
    if (n <= 1) return;
    
    if ((n & (n - 1)) == 0) {
        cooleyTukeyFFT(data, direction);
    } else {
        dft(data, direction);
    }
}

void FourierTransform::cooleyTukeyFFT(std::span<ComplexImage::Complex> data, Direction direction) {
    size_t n = data.size();
    if (n <= 1) return;
    
    for (size_t i = 0, j = 0; i < n; ++i) {
        if (i < j) std::swap(data[i], data[j]);
        size_t bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
    }
    
    Scalar angle_sign = (direction == Direction::Forward) ? -2.0 * PI : 2.0 * PI;
    
    for (size_t len = 2; len <= n; len <<= 1) {
        Scalar angle = angle_sign / len;
        ComplexImage::Complex wlen(std::cos(angle), std::sin(angle));
        
        for (size_t i = 0; i < n; i += len) {
            ComplexImage::Complex w(1, 0);
            size_t half_len = len >> 1;
            
            for (size_t j = 0; j < half_len; ++j) {
                ComplexImage::Complex u = data[i + j];
                ComplexImage::Complex v = data[i + j + half_len] * w;
                data[i + j] = u + v;
                data[i + j + half_len] = u - v;
                w *= wlen;
            }
        }
    }
    
    if (direction == Direction::Inverse) {
        Scalar factor = 1.0 / n;
        for (auto& c : data) {
            c *= factor;
        }
    }
}

void FourierTransform::dft(std::span<ComplexImage::Complex> data, Direction direction) {
    size_t n = data.size();
    std::vector<ComplexImage::Complex> result(n);
    
    Scalar angle_sign = (direction == Direction::Forward) ? -2.0 * PI : 2.0 * PI;
    
    for (size_t k = 0; k < n; ++k) {
        ComplexImage::Complex sum(0, 0);
        for (size_t j = 0; j < n; ++j) {
            Scalar angle = angle_sign * k * j / n;
            sum += data[j] * ComplexImage::Complex(std::cos(angle), std::sin(angle));
        }
        result[k] = sum;
    }
    
    if (direction == Direction::Inverse) {
        Scalar factor = 1.0 / n;
        for (auto& c : result) {
            c *= factor;
        }
    }
    
    std::copy(result.begin(), result.end(), data.begin());
}

ComplexImage FourierTransform::applyFrequencyMask(const ComplexImage& frequency_domain, Scalar frequency_cutoff, bool low_pass) {
    ComplexImage result = frequency_domain;
    const Scalar width = result.getWidth();
    const Scalar height = result.getHeight();
    
    // Use C++23 ranges for coordinate generation and filtering  
    auto coordinates = std::views::cartesian_product(
        std::views::iota(0uz, static_cast<size_t>(height)),
        std::views::iota(0uz, static_cast<size_t>(width))
    );
    
    // Filter and process coordinates that need masking
    auto mask_coords = coordinates | std::views::filter([=](const auto& coord) {
        auto [y, x] = coord;
        Scalar fx = (x < width / 2.0) ? static_cast<Scalar>(x) : static_cast<Scalar>(x) - width;
        Scalar fy = (y < height / 2.0) ? static_cast<Scalar>(y) : static_cast<Scalar>(y) - height;
        Scalar freq = std::sqrt(fx * fx + fy * fy);
        
        return (low_pass && freq > frequency_cutoff) || (!low_pass && freq < frequency_cutoff);
    });
    
    // Apply masking using ranges
    std::for_each(mask_coords.begin(), mask_coords.end(),
        [&result](const auto& coord) {
            auto [y, x] = coord;
            result.at(x, y) = ComplexImage::Complex(0, 0);
        });
    
    return result;
}

ComplexImage FourierTransform::applyFrequencyMaskCircular(const ComplexImage& frequency_domain, Scalar radius_ratio) {
    ComplexImage result = frequency_domain;
    const size_t width = result.getWidth();
    const size_t height = result.getHeight();
    
    const Scalar cx = width / 2.0;
    const Scalar cy = height / 2.0;
    const Scalar max_radius = std::min(cx, cy) * radius_ratio;
    
    // Use C++23 ranges for coordinate generation and filtering  
    auto coordinates = std::views::cartesian_product(
        std::views::iota(0uz, static_cast<size_t>(height)),
        std::views::iota(0uz, static_cast<size_t>(width))
    );
    
    // Filter coordinates outside the circle and apply masking in parallel
    auto outside_circle = coordinates | std::views::filter([=](const auto& coord) {
        auto [y, x] = coord;
        Scalar dx = static_cast<Scalar>(x) - cx;
        Scalar dy = static_cast<Scalar>(y) - cy;
        Scalar dist = std::sqrt(dx * dx + dy * dy);
        return dist > max_radius;
    });
    
    std::for_each(outside_circle.begin(), outside_circle.end(),
        [&result](const auto& coord) {
            auto [y, x] = coord;
            result.at(x, y) = ComplexImage::Complex(0, 0);
        });
    
    return result;
}

std::vector<std::pair<int, int>> FourierTransform::getTopFrequencyIndices(const ComplexImage& frequency_domain, size_t num_frequencies) {
    const size_t width = frequency_domain.getWidth();
    const size_t height = frequency_domain.getHeight();
    
    // Use C++23 ranges to generate coordinates and transform to magnitude tuples
    auto coordinates = std::views::cartesian_product(
        std::views::iota(0uz, height),
        std::views::iota(0uz, width)
    );
    
    std::vector<std::tuple<Scalar, int, int>> magnitude_indices;
    magnitude_indices.reserve(width * height);
    
    for (const auto& coord : coordinates) {
        auto [y, x] = coord;
        Scalar mag = std::abs(frequency_domain.at(x, y));
        magnitude_indices.emplace_back(mag, static_cast<int>(x), static_cast<int>(y));
    }
    
    // Sort by magnitude for better performance
    const auto limit = std::min(num_frequencies, magnitude_indices.size());
    std::partial_sort(magnitude_indices.begin(), 
                      magnitude_indices.begin() + limit,
                      magnitude_indices.end(),
                      std::greater<>{});
    
    // Extract coordinates from top magnitude tuples
    std::vector<std::pair<int, int>> result;
    result.reserve(limit);
    
    for (size_t i = 0; i < limit; ++i) {
        result.emplace_back(std::get<1>(magnitude_indices[i]), std::get<2>(magnitude_indices[i]));
    }
    
    return result;
}

ComplexImage FourierTransform::keepTopFrequencies(const ComplexImage& frequency_domain, size_t num_frequencies) {
    ComplexImage result(frequency_domain.getWidth(), frequency_domain.getHeight());
    auto top_indices = getTopFrequencyIndices(frequency_domain, num_frequencies);
    
    for (const auto& [x, y] : top_indices) {
        result.at(x, y) = frequency_domain.at(x, y);
    }
    
    return result;
}

RGBComplexImage FourierTransform::transformRGB2D(const RGBComplexImage& input, Direction direction) {
    RGBComplexImage result(input.getWidth(), input.getHeight());
    
    // Transform each channel in parallel using C++23 ranges and futures
    auto channel_range = std::views::iota(0, 3);
    std::vector<std::future<ComplexImage>> channel_futures;
    
    // Launch parallel transforms for each RGB channel
    std::ranges::transform(channel_range, std::back_inserter(channel_futures),
        [&input, direction](int channel) {
            return std::async(std::launch::async, [&input, direction, channel]() {
                return FourierTransform{}.transform2D(input.getChannel(channel), direction);
            });
        });
    
    // Collect results from futures
    for (auto [channel, future] : std::views::zip(channel_range, channel_futures)) {
        result.getChannel(channel) = future.get();
    }
    
    return result;
}

RGBComplexImage FourierTransform::keepTopFrequenciesRGB(const RGBComplexImage& frequency_domain, size_t num_frequencies) {
    RGBComplexImage result(frequency_domain.getWidth(), frequency_domain.getHeight());
    
    // Apply frequency filtering to each channel
    for (int channel = 0; channel < 3; ++channel) {
        result.getChannel(channel) = keepTopFrequencies(frequency_domain.getChannel(channel), num_frequencies);
    }
    
    return result;
}
