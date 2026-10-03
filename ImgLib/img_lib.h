#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

namespace img_lib {

// I think better place this class to separate file like utils.h
template <typename T>
class Span {
public:
    using iterator = T*;
    using const_iterator = const T*;

    explicit Span(T* ptr, size_t size) : ptr_(ptr), size_(size) {
    }

    iterator begin() {
        return const_cast<iterator>(static_cast<const Span&>(*this).begin());
    }

    const_iterator begin() const {
        return ptr_;
    }

    const_iterator cbegin() const {
        return begin();
    }

    iterator end() {
        return const_cast<iterator>(static_cast<const Span&>(*this).end());
    }

    const_iterator end() const {
        return ptr_ + size_;
    }

    const_iterator cend() const {
        return end();
    }

    size_t size() const {
        return size_;
    }

    T* data() {
        return const_cast<T*>(static_cast<const Span&>(*this).data());
    }

    const T* data() const {
        return ptr_;
    }

    T& operator[](size_t index) {
        return const_cast<T&>(static_cast<const Span&>(*this)[index]);
    }

    const T& operator[](size_t index) const {
        assert(index < size_);

        return *(ptr_ + index);
    }

private:
    T* ptr_;
    size_t size_;
};

struct Size {
    int width;
    int height;
};

struct Color {
    static Color Black() {
        return {std::byte{0}, std::byte{0}, std::byte{0}, std::byte{255}};
    }

    std::byte r, g, b, a;
};

class Image {
public:
    // создаёт пустое изображение
    Image() = default;

    // создаёт изображение заданного размера, заполняя его заданным цветом
    Image(int w, int h, Color fill);

    // геттеры для отдельного пикселя изображения
    Color GetPixel(int x, int y) const {
        return const_cast<Image*>(this)->GetPixel(x, y);
    }
    Color& GetPixel(int x, int y) {
        assert(x < GetWidth() && y < GetHeight() && x >= 0 && y >= 0);
        return GetLine(y)[x];
    }

    // геттер для заданной строки изображения
    Color* GetLine(int y);
    const Color* GetLine(int y) const;

    int GetWidth() const;
    int GetHeight() const;

    // шаг задаёт смещение соседних строк изображения
    // он обычно совпадает с шириной, но может быть больше неё
    int GetStep() const;

    // будем считать изображение корректным, если
    // его площадь положительна
    explicit operator bool() const {
        return GetWidth() > 0 && GetHeight() > 0;
    }

    bool operator!() const {
        return !operator bool();
    }

private:
    int width_ = 0;
    int height_ = 0;
    int step_;

    std::vector<Color> pixels_;
};

}  // namespace img_lib