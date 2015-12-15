#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include "./srilm-1.5.10/lm/src/Ngram.h"
#include "./srilm-1.5.10/lm/src/Vocab.h"

using namespace std;

typedef map<string, vector<string> > Table;

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
big5toShort(string str)
{
    unsigned short tmp;
    tmp = ((unsigned short)str[0] << 8 | (unsigned short)str[1]);
    return tmp;
}

inline const char*
shorttoBig5(unsigned short s)
{
    string str("");
    str[0] = (unsigned char)(s >> 8);
    str[1] = (unsigned char)(s & 0xFF);
    return (str.c_str());
}

inline VocabIndex
getShortIndex(unsigned short word)
{
    VocabIndex wid = voc.getIndex((const char*)shorttoBig5(word));
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
    string str, tok, key, tmp;
    vector<string> v;
    size_t pos;
    unsigned short sh;
    Table tb;
    fs.open(zymap, ios::in);
    getline(fs, str);
//    while (getline(fs, str)) {
        v.clear();
        tok.clear();
        pos = 0;
        pos = strGetTok(str, tok, pos);
        key = tok;
        while (pos != string::npos) {
            pos = strGetTok(str, tok, pos);
            v.push_back(tok);
        }
        tb[key] = v;
//    }

        str.clear();
        v.clear();
        Table::iterator it;
        str = "�t";
        it = tb.begin();
        v = (*it).second;
        for (size_t i = 0; i < v.size(); i++) {
            cout << v[i] << " ";
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
