#include <iostream>
#include <fstream>
#include <string>
#include <preform/color.hpp>
#include <preform/image2.hpp>
#include <preform/image_filters.hpp>
#include <preform/simplex_noise3.hpp>
#include <preform/option_parser.hpp>

// Float type.
typedef float Float;

// 1-dimensional vector type.
typedef pr::vec1<Float> Vec1f;

// 2-dimensional vector type.
typedef pr::vec2<Float> Vec2f;

// 3-dimensional vector type.
typedef pr::vec2<Float> Vec3f;

// 2-dimensional vector type.
typedef pr::vec2<int> Vec2i;

// Image.
typedef pr::image2<Float, Float, 1> Image2x1;

// Image Mitchell filter.
typedef pr::mitchell_filter2<Float> MitchellFilter2;

// Simplex noise.
typedef pr::simplex_noise3<Float> SimplexNoise3;

int main(int argc, char** argv)
{
    // Configurable options.
    int seed = 0;
    Vec2i image_dim = {512, 512};
    Vec2f image_filter_rad = {2, 2};
    Vec3f noise_scale = {8, 8};
    Float noise_locz = 0;
    std::string ofs_name = "simplex_noise3.pgm";

    // Option parser.
    pr::option_parser opt_parser("[OPTIONS]");

    // Specify seed.
    opt_parser.on_option(
    "-s", "--seed", 1,
    [&](char** argv) {
        try {
            seed = std::stoi(argv[0]);
        }
        catch (const std::exception&) {
            throw 
                std::runtime_error(
                std::string("-s/--seed expects 1 integer ")
                    .append("(can't parse ").append(argv[0])
                    .append(")"));
        }
    })
    << "Specify seed. By default, 0.\n";

    // Specify image dimensions.
    opt_parser.on_option(
    nullptr, "--image-dim", 2,
    [&](char** argv) {
        try {
            image_dim[0] = std::stoi(argv[0]);
            image_dim[1] = std::stoi(argv[1]);
            if (!(image_dim > 0).all()) {
                throw std::exception();
            }
        }
        catch (const std::exception&) {
            throw 
                std::runtime_error(
                std::string("--image-dim expects 2 positive integers ")
                    .append("(can't parse ")
                    .append(argv[0]).append(" ")
                    .append(argv[1]).append(")"));
        }
    })
    << "Specify image dimensions. By default, 512x512.\n";

    // Specify image filter radii.
    opt_parser.on_option(
    nullptr, "--image-filter-rad", 2,
    [&](char** argv) {
        try {
            image_filter_rad[0] = std::stof(argv[0]);
            image_filter_rad[1] = std::stof(argv[1]);
            if (!(image_filter_rad > 0).all()) {
                throw std::exception();
            }
        }
        catch (const std::exception&) {
            throw 
                std::runtime_error(
                std::string("--image-filter-rad expects 2 positive floats ")
                    .append("(can't parse ")
                    .append(argv[0]).append(" ")
                    .append(argv[1]).append(")"));
        }
    })
    << "Specify image reconstruction filter radii. By default, 2x2.\n";

    // Specify noise scale.
    opt_parser.on_option(
    nullptr, "--noise-scale", 2,
    [&](char** argv) {
        try {
            noise_scale[0] = std::stof(argv[0]);
            noise_scale[1] = std::stof(argv[1]);
        }
        catch (const std::exception&) {
            throw 
                std::runtime_error(
                std::string("--noise-scale expects 2 floats ")
                    .append("(can't parse ")
                    .append(argv[0]).append(" ")
                    .append(argv[1]).append(")"));
        }
    })
    << "Specify noise scale. By default, 8x8.\n";

    // Specify noise location in z.
    opt_parser.on_option(
    nullptr, "--noise-locz", 1,
    [&](char** argv) {
        try {
            noise_locz = std::stof(argv[0]);
        }
        catch (const std::exception&) {
            throw 
                std::runtime_error(
                std::string("--noise-locz expects 1 float ")
                    .append("(can't parse ").append(argv[0])
                    .append(")"));
        }
    })
    << "Specify noise location in z. By default, 0.\n";

    // Specify output filename.
    opt_parser.on_option(
    "-o", "--output", 1,
    [&](char** argv) {
        ofs_name = argv[0];
    })
    << "Specify output filename. By default, simplex_noise3.pgm.\n";

    // Display help.
    opt_parser.on_option(
    "-h", "--help", 0,
    [&](char**) {
        std::cout << opt_parser << std::endl;
        std::exit(EXIT_SUCCESS);
    })
    << "Display this help and exit.\n";

    try {
        // Parse args.
        opt_parser.parse(argc, argv);
    }
    catch (const std::exception& exception) {
        std::cerr << "Unhandled exception!\n";
        std::cerr << "exception.what(): " << exception.what() << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Noise.
    SimplexNoise3 noise(seed);

    // Image.
    Image2x1 image;
    image.resize(image_dim);
    MitchellFilter2 image_filter;

    for (int i = 0; i < image_dim[0]; i++)
    for (int j = 0; j < image_dim[1]; j++) {
        // Supersample.
        for (int k = 0; k < 3; k++)
        for (int l = 0; l < 3; l++) {
            Vec2f image_loc = {
                Float(i) + Float(k) / 3,
                Float(j) + Float(l) / 3
            };
            Vec2f noise_loc = image_loc / image_dim * noise_scale;
            Vec1f noise_val = {noise.evaluate({
                noise_loc[0],
                noise_loc[1],
                noise_locz
            })};
            noise_val *= Float(0.5);
            noise_val += Float(0.5); // Map into [0, 1].
            image.reconstruct(
                    noise_val / 9, 
                    image_loc,
                    image_filter_rad,
                    image_filter);
        }
    }

    try {
        std::ofstream ofs(ofs_name);
        ofs << "P2\n";
        ofs << image_dim[0] << ' ';
        ofs << image_dim[1] << '\n';
        ofs << "255\n";
        for (int j = 0; j < image_dim[1]; j++) 
        for (int i = 0; i < image_dim[0]; i++) {
            ofs << int(pr::pack_uint8(
                       pr::srgbenc(image(i, j)[0]))) << ' ';
        }
    }
    catch (const std::exception& exception) {
        std::cerr << "Unhandled exception!\n";
        std::cerr << "exception.what(): " << exception.what() << "\n";
        std::exit(EXIT_FAILURE);
    }
    return 0;
}
