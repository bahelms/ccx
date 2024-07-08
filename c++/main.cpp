#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    std::ifstream file(argv[1]);
    if (file) {
        std::vector<char> buffer;
        char ch;

        while (file.get(ch)) {
            std::cout << ch;
        }
    }
}
