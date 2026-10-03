#include <bmp_image.h>
#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

using namespace std;

using std::string_view_literals::operator""sv;

namespace {

enum class Format : char {
    BMP,
    JPEG,
    PPM,
    UNKNOWN,
};

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    return Format::UNKNOWN;
}

class ImageFormatInterface {
public:
    [[nodiscard]] virtual bool SaveImage(const img_lib::Path& file,
                                         const img_lib::Image& image) const = 0;

    [[nodiscard]] virtual img_lib::Image LoadImage(
        const img_lib::Path& file) const = 0;

    virtual ~ImageFormatInterface() = default;
};

class Jpeg final : public ImageFormatInterface {
public:
    [[nodiscard]] bool SaveImage(const img_lib::Path& file,
                                 const img_lib::Image& image) const override {
        //
        return img_lib::SaveJPEG(file, image);
    }

    [[nodiscard]] img_lib::Image LoadImage(
        const img_lib::Path& file) const override {
        //
        return img_lib::LoadJPEG(file);
    }
};

class Ppm final : public ImageFormatInterface {
public:
    [[nodiscard]] bool SaveImage(const img_lib::Path& file,
                                 const img_lib::Image& image) const override {
        //
        return img_lib::SavePPM(file, image);
    }

    [[nodiscard]] img_lib::Image LoadImage(
        const img_lib::Path& file) const override {
        //
        return img_lib::LoadPPM(file);
    }
};

class Bmp final : public ImageFormatInterface {
public:
    [[nodiscard]] bool SaveImage(const img_lib::Path& file,
                                 const img_lib::Image& image) const override {
        //
        return img_lib::SaveBMP(file, image);
    }

    [[nodiscard]] img_lib::Image LoadImage(
        const img_lib::Path& file) const override {
        //
        return img_lib::LoadBMP(file);
    }
};

const ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    switch (GetFormatByExtension(path)) {
        case Format::JPEG:
            static const Jpeg kJpegInterface;
            return &kJpegInterface;
        case Format::PPM:
            static const Ppm kPpmInterface;
            return &kPpmInterface;
        case Format::BMP:
            static const Bmp kBmpInterface;
            return &kBmpInterface;
        default:
            break;
    }

    return nullptr;
}

}  // namespace

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    const auto* in_loader = GetFormatInterface(in_path);

    if (nullptr == in_loader) {
        std::cerr << "Unknown format of the input file"sv << std::endl;

        return 2;
    }

    const auto* out_saver = GetFormatInterface(out_path);

    if (nullptr == out_saver) {
        std::cerr << "Unknown format of the output file"sv << std::endl;

        return 3;
    }

    auto img = in_loader->LoadImage(in_path);

    if (!img) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!out_saver->SaveImage(out_path, img)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
}