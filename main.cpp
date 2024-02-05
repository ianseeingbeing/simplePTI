#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

void pdf_to_img(std::string, std::string, int);
std::string makeExportPath(const std::string pdfPath);

int main(int argc, char *argv[]) {
    
    std::string pdfPath;
    int dpi;

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_pdf_file> <dpi>" << std::endl;
        return 1;
    }
    else {
        pdfPath = argv[1];
        dpi = std::stoi(argv[2]);
    }

    pdf_to_img(pdfPath, makeExportPath(pdfPath), dpi);

    return 0;
}

void pdf_to_img(std::string pdfPath, std::string exportPath, int dpi) {
    std::string arguments = "-r " + std::to_string(dpi) + " -progress -aa yes -aaVector yes";
    std::string cmd = "pdftoppm " + arguments + " " + pdfPath + " " + exportPath + " -png";

    int result = std::system(cmd.c_str());

    std::cout << (result == 0 ? "finished successfully" : "ran into error") << std::endl;
}

std::string makeExportPath(const std::string pdfPath) {
    std::string path = pdfPath;

    for (int i = 0; i < 4; i++) {
        path.pop_back(); 
    }

    if (std::filesystem::exists(pdfPath)) {
        if (!std::filesystem::exists(path)) {  
            std::filesystem::create_directory(path); 
            std::cout << "created export directory at: " << path << std::endl;  
        }
        else {
            std::cout << "export directory already exists\n"; 
        }
    }
    else {
        std::cout << "cannot find pdf at: " << pdfPath << std::endl;
    }

    path += "/img";

    return path;
}