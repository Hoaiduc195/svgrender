#pragma once
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

class FileReader {
public:
    static string ReadFileToString(const string& filename) {
        ifstream file(filename);
        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};
