#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <locale>

using namespace std;


int
main(int argc, char** argv) {
    const char *from, *to;
    if (argc == 3) {
        from = argv[1];
        to = argv[2];
    }
    else {
        cout << "Usage: modCalc [ <mappingfrom.txt>, <mappingto.txt> ]" << endl;
        exit(-1);
    }

    cout.imbue(locale("cht"));

    wfstream fs;
    wstring str;
    fs.open(from, ios::in);
    fs.imbue(locale("cht"));
    while (getline(fs, str)) {
        wcout << str << endl;
    }

    return 0;
}
