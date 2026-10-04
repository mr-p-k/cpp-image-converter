#include "bmp_image.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <vector>

#include "img_lib.h"
#include "pack_defines.h"

using namespace std;

namespace img_lib {

namespace {

const std::uint32_t kBMPPadding = 54;
const std::uint32_t kBMPInfoHeaderSize = 40;
const std::uint16_t kRGBPlanes = 1;
const std::uint16_t kBitInPixel = 24;
const std::uint32_t kCompress = 0;
const std::int32_t kDPM = 11811;  // ~300 DPI
const std::int32_t kColors = 0;
const std::int32_t kImpColors = 0x1000000;

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    std::array<char, 2> bm = {'B', 'M'};
    std::uint32_t size = 0;
    std::array<uint8_t, 4> reserve = {0};
    std::uint32_t padding = kBMPPadding;
}
PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    // поля заголовка Bitmap Info Header
    std::uint32_t size = kBMPInfoHeaderSize;
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::uint16_t planes = kRGBPlanes;
    std::uint16_t bits = kBitInPixel;
    std::uint32_t compress = kCompress;
    std::uint32_t data_size = 0;
    std::int32_t horizontal_dpm = kDPM;
    std::int32_t vertical_dpm = kDPM;
    std::int32_t colors = kColors;
    std::int32_t imp_colors = kImpColors;
}
PACKED_STRUCT_END;

// функция вычисления отступа по ширине
int GetBMPStride(int w) {
    return 4 * (((w * 3) + 3) / 4);
}

PACKED_STRUCT_BEGIN RawBGR {
    char b, g, r;
}
PACKED_STRUCT_END;

[[nodiscard]] bool ValidateFileHeader(const BitmapFileHeader& header) noexcept {
    return header.bm == std::array<char, 2>{'B', 'M'} &&
           header.padding == kBMPPadding;
}

[[nodiscard]] bool ValidateInfoHeader(const BitmapInfoHeader& header) noexcept {
    return header.size == kBMPInfoHeaderSize && header.planes == kRGBPlanes &&
           header.bits == kBitInPixel && header.compress == kCompress &&
           header.imp_colors == kImpColors && 0 < header.width &&
           0 < header.height;
}

}  // namespace

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    std::ofstream ofs(file, std::ios::binary);
    if (!ofs) {
        return false;
    }

    BitmapFileHeader bmp_file_header{};
    BitmapInfoHeader bmp_info_header{};

    const auto kWidth = image.GetWidth();
    const auto kHeigth = image.GetHeight();
    const auto kBufSize = static_cast<size_t>(GetBMPStride(kWidth));

    bmp_file_header.size =
        kBMPPadding + static_cast<uint32_t>(kBufSize * kHeigth);

    bmp_info_header.width = static_cast<int32_t>(kWidth);
    bmp_info_header.height = static_cast<int32_t>(kHeigth);
    bmp_info_header.data_size = static_cast<uint32_t>(kBufSize * kHeigth);

    ofs.write(reinterpret_cast<char*>(&bmp_file_header),
              sizeof(BitmapFileHeader));
    ofs.write(reinterpret_cast<char*>(&bmp_info_header),
              sizeof(BitmapInfoHeader));

    // fill data
    std::vector<char> buf(kBufSize, 0);

    for (int y = kHeigth - 1; 0 <= y; --y) {
        auto src_line = Span<const Color>(image.GetLine(y), kWidth);

        auto dst_line =
            Span<RawBGR>(reinterpret_cast<RawBGR*>(buf.data()), kWidth);

        const auto kTransformer = [](Color c) -> RawBGR {
            return RawBGR{
                .b = static_cast<char>(c.b),
                .g = static_cast<char>(c.g),
                .r = static_cast<char>(c.r),
            };
        };

        std::transform(src_line.cbegin(), src_line.cend(), dst_line.begin(),
                       kTransformer);

        ofs.write(buf.data(), static_cast<std::streamsize>(kBufSize));
    }

    ofs.close();

    return ofs.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    std::ifstream ifs(file, std::ios::binary);
    if (!ifs) {
        return {};
    }

    BitmapFileHeader bmp_file_header{};
    BitmapInfoHeader bmp_info_header{};

    if (!ifs.read(reinterpret_cast<char*>(&bmp_file_header),
                  sizeof(bmp_file_header)) ||
        !ValidateFileHeader(bmp_file_header)) {
        //
        return {};
    }

    if (!ifs.read(reinterpret_cast<char*>(&bmp_info_header),
                  sizeof(bmp_info_header)) ||
        !ValidateInfoHeader(bmp_info_header)) {
        //
        return {};
    }

    const auto kWidth = static_cast<int>(bmp_info_header.width);
    const auto kHeigth = static_cast<int>(bmp_info_header.height);
    const auto kBufSize = static_cast<size_t>(GetBMPStride(kWidth));

    Image result{kWidth, kHeigth, Color::Black()};

    std::vector<char> buf;
    buf.reserve(kBufSize);

    for (int y = 0; y < kHeigth; ++y) {
        ifs.read(buf.data(), static_cast<std::streamsize>(kBufSize));

        auto dst_line = Span<Color>(result.GetLine(kHeigth - 1 - y), kWidth);
        auto src_line = Span<const RawBGR>(
            reinterpret_cast<const RawBGR*>(buf.data()), kWidth);

        const auto kTransformer = [](RawBGR bgr) -> Color {
            return Color{
                .r = static_cast<std::byte>(bgr.r),
                .g = static_cast<std::byte>(bgr.g),
                .b = static_cast<std::byte>(bgr.b),
                .a = std::byte{0},
            };
        };

        std::transform(src_line.cbegin(), src_line.cend(), dst_line.begin(),
                       kTransformer);
    }

    return result;
}

}  // namespace img_lib