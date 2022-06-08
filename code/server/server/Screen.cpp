#include "Server/Screen.hpp"
#include <direct.h>
#include <cstdlib>
#include <chrono>
#include <fstream>

namespace NRenderer {
Screen::Screen() : width(500), height(500), updated(false), mtx() {
    pixels = new RGBA[height * width];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            pixels[i * width + j] = {0, 0, 0, 1};
        }
    }
}
bool Screen::isUpdated() const {
    return updated;
}
Screen::~Screen() {
    if (pixels != nullptr)
        delete[] pixels;
}
void Screen::release() {
    mtx.lock();
    if (pixels == nullptr) {
        mtx.unlock();
        return;
    }
    delete[] pixels;
    pixels = nullptr;
    mtx.unlock();
}
unsigned int Screen::getWidth() const {
    mtx.lock();
    auto w = width;
    mtx.unlock();
    return w;
}
unsigned int Screen::getHeight() const {
    mtx.lock();
    int h = height;
    mtx.unlock();
    return h;
}
const RGBA* Screen::getPixels() const {
    mtx.lock();
    updated = false;
    mtx.unlock();
    return pixels;
}
void Screen::set(RGBA* pixels, int width, int height) {
    mtx.lock();
    updated = true;
    this->width = width;
    this->height = height;
    if (this->pixels != nullptr)
        delete[] this->pixels;
    this->pixels = new RGBA[width * height];
    for (int i = 0; i < width * height; i++) {
        this->pixels[i] = clamp(pixels[i]);
    }
    saveBMP();
    mtx.unlock();
}

std::string current_working_directory() {
    char buff[250];
    _getcwd(buff, 250);
    std::string current_working_directory(buff);
    return current_working_directory;
}

void Screen::saveBMP() {
    int l = (width * 3 + 3) / 4 * 4;
    char head[] = "BM";
    int bmi[] = {width * height * 4 + 54,
                 0,
                 54,
                 40,
                 width,
                 -height,
                 1 | 4 * 8 << 16,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0};
    char* img = new char[4 * width * height];
    for (int i = 0; i < width * height; i++) {
        img[4 * i] = (unsigned char)(this->pixels[i].z * 255);
        img[4 * i + 1] = (unsigned char)(this->pixels[i].y * 255);
        img[4 * i + 2] = (unsigned char)(this->pixels[i].x * 255);
        img[4 * i + 3] = (unsigned char)(this->pixels[i].w * 255);
    }
    std::chrono::zoned_time now{"Asia/Shanghai",
                                std::chrono::system_clock::now()};
    auto today = std::chrono::floor<std::chrono::days>(now.get_local_time());
    std::string filename = std::format(
        "E:/projects/nrenderer/code/pic/{:%Y_%m_%d}_{:%H_%M_%S}.bmp",
        std::chrono::year_month_day{today},
        std::chrono::hh_mm_ss{now.get_local_time() - today});
    std::ofstream fp(filename, ios::out | ios::binary);
    fp.write((char*)head, 2);
    fp.write((char*)bmi, sizeof(bmi));
    fp.write(img, 4 * height * width);
    fp.close();
    delete[] img;
}
}  // namespace NRenderer
