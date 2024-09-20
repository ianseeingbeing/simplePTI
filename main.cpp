#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <future>

using std::string;

int pdf_to_img(const string pdf, const string arguments);
void dir_to_img(const string dir, const string arguments, const bool inParallel);
string makeExportDirectory(const string pdf);

int main(int argc, char *argv[]) {

    string help =
        "pti (pdf to image) - a custom wraper for the pdftoppm cli interface.\n"
        "Usage: pti [flags] [path to PDF or directory]\n"
        "   -h                              : help\n"
        "   -p                              : convert directory of pdfs in parallel\n"
        "   -r <int>                        : resolution, in DPI (default is 150)\n"
        "   -png                            : convert to .png\n"
        "   -jpeg                           : convert to .jpg\n";

    string path = "";
    string imageType = "";
    int dpi = 150;
    bool inParallel = false;
    bool isDirectory = false;

    // checks for correct arguments
    if (argc == 1) {
        std::cerr << help << std::endl;
        return 1;
    }

    // apply properties of the arguemnts
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-h") {
            std::cout << help << std::endl;
        }
        else if (arg == "-p") {
            inParallel = true;
        }
        else if (arg == "-r") {
            i++;
            dpi = std::stoi(argv[i]);
        }
        else if (arg == "-png" || arg == "-jpeg") {
            imageType = arg;
        }
        else if (arg.find('/') != -1) {
            path = arg;
            if (path.back() == '/') {
                isDirectory = true;
            }
        }
    }

    std::cout << inParallel << "\n" << isDirectory << "\n" << dpi << "\n" << imageType << "\n" << path << std::endl;

    // checks path exists
    if (!std::filesystem::exists(path)) {
        std::cerr << "Error - path does not exist" << path << std::endl;
        return 0;
    }

    // checks image format
    if (imageType != "-png" && imageType != "-jpeg") {
        std::cerr << imageType << "Error - image format is not valid. use: -png or -jpeg" << std::endl;
        return 0;
    }

    // checks dpi
    if (dpi < 1) {
        std::cerr << "dpi has to be at least 1" << std::endl;
        return 0;
    }

    string arguments =
        "-progress "
        "-aa yes "
        "-aaVector yes "
        "-r " + std::to_string(dpi) +
        " " + imageType;

    if (isDirectory) {
        dir_to_img(path, arguments, inParallel);
    } else {
        pdf_to_img(path, arguments);
    }

    return 0;
}

int pdf_to_img(const string pdf, const string arguments) {

    // create export directory
    string exportDir = makeExportDirectory(pdf);
    std::cout << exportDir << std::endl;

    // create export dir
    string exportPath = exportDir + "/img";
    std::cout << exportPath << std::endl;

    // call command
    string cmd = "pdftoppm " + arguments + " " + pdf + " " + exportPath;
    int ptiResult = std::system(cmd.c_str());

    if (ptiResult != 0) {
        std::cerr << "Error converting " << pdf << std::endl;
        return 1;
    }

    return 0;
}

// convert an entire dir of pdfs to images
void dir_to_img(const string dir, string arguments, const bool inParallel) {
    std::vector<string> pdfs = {};

    // finds all .pdf files in dir and push into pdfs vector
    for (auto entry : std::filesystem::directory_iterator(dir)) {
        const auto path = entry.path();
        if (std::filesystem::is_regular_file(path)) {
            string filePath = path.string();
            if (filePath.substr(filePath.length() - 4) == ".pdf") {
                pdfs.push_back(filePath);
            }
        }
    }

    if (inParallel) {
        // converts all the pdfs at the same time using async.
        std::vector<std::future<int>> futures;
        for (const auto &pdf : pdfs) {
            futures.push_back(std::async(std::launch::async, pdf_to_img, pdf, arguments));
        }
        // checks if they have errored
        for (auto &future : futures) {
            int result = future.get();
            if (result != 0) {
                std::cerr << "Error finishing process" << std::endl;
            }
        }
    } else {
        for (string pdf : pdfs) {
            pdf_to_img(pdf, arguments);
        }
    }
}

// create export directory next to the location of the pdf file
string makeExportDirectory(const string pdf) {
    string dir = pdf;

    // removes the .pdf file extension
    for (int i = 0; i < 4; i++) {
        dir.pop_back();
    }

    // creates an export directory for the images
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directory(dir);
        std::cout << "created export directory at: " << dir << std::endl;
    } else {
        std::cout << "export directory already exists\n";
    }

    return dir;
}
