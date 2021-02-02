#include "ImageWriter.h"

// uint8_t ImageWriter::rbgdoubleToInt(double f)
// {
//     uint8_t c;
//     if (f < 0.0)
//     {
//         c = (uint8_t)0;
//     }
//     else if (f > 1.0)
//     {
//         c = (uint8_t)255;
//     }
//     else
//     {
//         c = (uint8_t)(f * 255);
//     }

//     return c;
// }

void ImageWriter::_writeRgbString(unsigned char f, bool &newLine, int &charsInLine,
                                  std::ofstream *streamPtr)
{
    // std::string c = std::to_string(rbgdoubleToInt(f));
    std::string c = std::to_string(f);

    charsInLine += c.length() + 1;
    if (charsInLine >= 70)
    {
        *(streamPtr) << "\n";
        charsInLine = 0;
        newLine = true;
    }

    if (!newLine)
        *(streamPtr) << " ";
    *(streamPtr) << c;
    newLine = false;
}

void ImageWriter::writeToPPM(const std::string &fileName, std::vector<unsigned char> &image, uint32_t width, uint32_t height)
{
    std::ofstream out(fileName);

    if (out.fail())
    {
        throw std::runtime_error("Failed to open file.");
        return;
    }

    out << "P3\n"
        << std::to_string(width) << " " << std::to_string(height) << "\n255";

    int charsInCurrentLine = 0;
    bool newLine = true;

    // for (int i = height - 1; i >= 0; i--)
    // {
    //     out << "\n";
    //     charsInCurrentLine = 0;
    //     newLine = true;

    //     for (int j = 0; j < width; j += 4)
    //     {
    //         _writeRgbString(image.at((i * width) + j), newLine, charsInCurrentLine, &out);
    //         _writeRgbString(image.at((i * width) + j + 1), newLine, charsInCurrentLine, &out);
    //         _writeRgbString(image.at((i * width) + j + 2), newLine, charsInCurrentLine, &out);
    //     }
    // }

    for (int i = 0; i < height; i++)
    {
        out << "\n";
        charsInCurrentLine = 0;
        newLine = true;

        for (int j = 0; j < width; j++)
        {
            _writeRgbString(image.at((i * width * 4) + (j * 4)), newLine, charsInCurrentLine, &out);
            _writeRgbString(image.at((i * width * 4) + (j * 4) + 1), newLine, charsInCurrentLine, &out);
            _writeRgbString(image.at((i * width * 4) + (j * 4) + 2), newLine, charsInCurrentLine, &out);
        }
    }
    out << "\n";
    out.close();
}