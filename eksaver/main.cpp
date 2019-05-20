#include "saver.h"

#include "base/context.h"

#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 4) {
        // For example: ./eksaver /home/user/expack/eksaver.ini /home/user/expack/hist RTS-6.19 Si-6.19 BR-6.19 SBRF-6.19
        std::cerr << "Usage: eksaver [path_to_ini_file] [path_to_hist_folder] [symbol] [symbol] ..." << std::endl;
        return 1;
    }

    std::string ini = argv[1];
    std::string path = argv[2];
    std::vector<std::string> symbols;
    for (int i = 3; i < argc; i++)
        symbols.push_back(argv[i]);

    Config config;
    config.main.name = "eksaver";
    config.main.mode = Mode::Paper;
    config.plaza.ini = ini;
    config.plaza.key = "Your key";

    Saver saver(path, symbols);
    Context ctx(config, &saver);
    ctx.Run();

    return 0;
}
