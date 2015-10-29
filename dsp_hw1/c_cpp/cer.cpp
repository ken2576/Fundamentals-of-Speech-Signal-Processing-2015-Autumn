#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

void readAns( const char*, vector<string> &);
void readResult( const char*, vector<string> &);

static void
usage()
{
    cout << "Usage: modCalc [ <result.txt>, <answer.txt> ]" << endl;
}

static void
myexit()
{
    usage();
    exit(-1);
}


int
main (int argc, char** argv) {
    const char *result_filename, *answer_filename;

    if (argc == 3) {
        result_filename = argv[1];
        answer_filename = argv[2];
    }
    else
        myexit();

    vector<string> answer, result;
    readAns(answer_filename, answer);
    readResult(result_filename, result);

    double counter = 0.0;
    for (size_t n=0; n<answer.size(); n++)
    {
        if (answer[n] == result[n])
        {
            counter++;
        }
    }
    cout << "acc:  " << float(100*counter/answer.size()) << endl;
    return 0;
}

void readAns( const char* file_name, vector<string> &answer)
{
    fstream fs;
    fs.open( file_name, ios::in );
    string strtmp;
    getline(fs, strtmp);
    while (strtmp != "" && strtmp != "\n")
    {
        answer.push_back(strtmp);
        getline(fs, strtmp);
    }
}

void readResult( const char* file_name, vector<string> &result )
{
    fstream fs;
    fs.open( file_name, ios::in );
    string strtmp;
    size_t pos1, pos2;
    pos2 = 0;
    pos1 = pos2;
    getline(fs, strtmp);
    while (strtmp != "" && strtmp != "\n")
    {
        pos2 = strtmp.find_first_of(" ", pos1);
        result.push_back(strtmp.substr(pos1, pos2));
        getline(fs, strtmp);
    }
}
