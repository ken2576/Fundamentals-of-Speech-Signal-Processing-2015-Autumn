#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include "./srilm-1.5.10/lm/src/Ngram.h"
#include "./srilm-1.5.10/lm/src/Vocab.h"

using namespace std;

typedef map<unsigned short, vector<unsigned short> > Table;

////////Static Variables/////////
static Vocab voc;


static void
usage()
{
    cout << "Usage: modCalc [ <text.txt>, <ZhuyingBig5.map>, <LM>, <order> ]" << endl;
}

static void
myexit()
{
    usage();
    exit(-1);
}


///////Inline Functions/////////
inline unsigned short
big5toShort(const char* str)
{
    unsigned short tmp;
    tmp = ((unsigned short)str[0] << 8 | (unsigned short)str[1]);
    return tmp;
}

inline const char*
shorttoBig5(unsigned short s)
{
    unsigned char str[2] = {0};
    str[0] = (unsigned char)(s >> 8);
    str[1] = (unsigned char)(s & 0xFF);
    return (const char*)(str);
}

inline VocabIndex
getShortIndex(unsigned short word)
{
    VocabIndex wid = voc.getIndex(shorttoBig5(word));
    return wid;
}

inline VocabIndex
getCharIndex(const char* word)
{
    VocabIndex wid = voc.getIndex(word);
    return wid;
}

///////Forward Declaration/////////
size_t strGetTok(const string&, string&, size_t pos = 0, const char delim = ' ');



int main(int argc, char** argv) {
    const char *text, *zymap, *lm, *order;
    int orderNum;
    if (argc == 5) {
        text = argv[1];
        zymap = argv[2];
        lm = argv[3];
        order = argv[4];
    }
    else
        myexit();

    //read order
    stringstream ss;
    ss << order;
    ss >> orderNum;

    //create Ngram Language Model
    Ngram ngramlm(voc, orderNum);

    File lmFile(lm, "r");
    ngramlm.read(lmFile);
    lmFile.close();

    //create ZhuyingBig5 map
    fstream fs;
    string str, tok;
    size_t pos;
    Table tb;
    fs.open(zymap, ios::in);
    while (getline(fs, str)) {
        tok.clear();
        pos = 0;
        pos = strGetTok(str, tok, pos);
        while (pos != string::npos) {
            pos = strGetTok(str, tok, pos);
            cout << tok << " ";
        }
        cout << endl;
    }

    return 0;
}



size_t
strGetTok(const string& str, string& tok, size_t pos, const char del)
{
    size_t start = str.find_first_not_of(del, pos);
    if (start == string::npos)
    {
        tok = "";
        return start;
    }
    size_t end = str.find_first_of(del, start);
    tok = str.substr(start, end - start);
    return end;
}
