#pragma once

#include <string>
#include <fstream>
#include <vector>

namespace ImageWriter
{
    uint8_t rbgdoubleToInt(double f);

    void _writeRgbString(unsigned char f, bool &newLine, int &charsInLine,
                         std::ofstream *streamPtr);

    void writeToPPM(const std::string &fileName, std::vector<unsigned char> &image, uint32_t width, uint32_t height);
}; // namespace ImageWriter
