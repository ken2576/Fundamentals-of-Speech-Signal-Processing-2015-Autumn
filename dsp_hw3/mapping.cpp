#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdint.h>

using namespace std;


int
main(int argc, char** argv) {
    const char *from;
    if (argc == 2) {
        from = argv[1];
    }
    else {
        exit(-1);
    }

    ifstream fs(from);
    string str;
    getline(fs, str);
    cout << (unsigned char) str[0] << (unsigned char) str[1] << endl;
    cout << (uint16_t) str[0] << "  " << (uint16_t) str[1] << endl;
    unsigned short utf;
    utf = ((unsigned char)str[0] << 8 | (unsigned char)str[1]);
    cout << utf << endl;
    unsigned char strtmp[10] = {0};
    strtmp[0] = (unsigned char)(utf >> 8);
    strtmp[1] = (unsigned char)(utf & 0x00FF);
    cout << strtmp << endl;
    return 0;
}
