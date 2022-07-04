//
// Created by JW on 03/07/2022.
//

std::vector<float> vertices = {
        0, 0, 0,
        1, 0, 0,
        0, 0, 0,
        0, 1, 0,
        0, 0, 0,
        0, 0, 1
};

std::vector<unsigned char> colors = {
        255, 0, 0, 255,
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        0, 0, 255, 255
};

std::vector<unsigned short> indices = {
        0, 1, // x axis - red
        2, 3, // y axis - green
        4, 5 // z axis - blue
};