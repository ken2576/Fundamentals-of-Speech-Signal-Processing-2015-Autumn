#include <iostream>
#include <cstring>
#include <cmath>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include "./srilm-1.5.10/lm/src/Ngram.h"
#include "./srilm-1.5.10/lm/src/Vocab.h"

using namespace std;

typedef map<unsigned short, vector<string> > Table;

//Static Variables
static Vocab voc;
static vector< vector<string> > seq;
static vector< vector<double> > delta;

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


//Inline Functions
inline unsigned short
big5toShort(string str)
{
    unsigned short tmp;
    tmp = ((unsigned char)str[0] << 8 | (unsigned char)str[1]);
    return tmp;
}

inline const char*
shorttoBig5(unsigned short s)
{
    string str("");
    str[0] = (unsigned char)(s >> 8);
    str[1] = (unsigned char)(s & 0x00FF);
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

//Forward Declaration
static double getUnigramProb(const char*);
static double getBigramProb(const char*, const char*);
static double getTrigramProb(const char*, const char*, const char*);

void viterbi();

//Printer Functions
void printVector(const vector<string>&);
void printSeq();

//Utility Functions
size_t strGetTok(const string&, string&, size_t pos = 0, const char delim = ' ');


int
main(int argc, char** argv) {
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
    vector<string> v;
    size_t pos;
    unsigned short key;
    Table tb;
    fs.open(zymap, ios::in);
    while (getline(fs, str)) {
        v.clear();
        tok.clear();
        pos = 0;
        pos = strGetTok(str, tok, pos);
        key = big5toShort(tok.c_str());
        while (pos != string::npos) {
            pos = strGetTok(str, tok, pos);
            v.push_back(tok);
        }
        tb[key] = v;
    }
    fs.close();

    //read text into seq
    fs.open(text, ios::in);
    while (getline(fs, str)) {
        v.clear();
        tok.clear();
        pos = 0;
        while (pos != string::npos) {
            pos = strGetTok(str, tok, pos);
            v.push_back(tok);
        }
        seq.push_back(v);
    }
    fs.close();

    printSeq();
//    for (size_t i = 0; i < seq.size(); i++) {
        //viterbi
//    }
    Table::iterator it;
    //str = str.substr(1, 2);
    //key = big5toShort(str);
    //it = tb.find(key);

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

// Get P(W1) -- unigram
static double
getUnigramProb(const char *w1)
{
    VocabIndex wid1 = voc.getIndex(w1);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { Vocab_None };
    return lm.wordProb( wid1, context);
}


// Get P(W2 | W1) -- bigram
static double
getBigramProb(const char *w1, const char *w2)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

// Get P(W3 | W1, W2) -- trigram
static double
getTrigramProb(const char *w1, const char *w2, const char *w3)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);
    VocabIndex wid3 = voc.getIndex(w3);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);
    if(wid3 == Vocab_None)  //OOV
        wid3 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid2, wid1, Vocab_None };
    return lm.wordProb( wid3, context );
}


void
viterbi()
{
}


void
printVector(const vector<string>& v)
{
    for (size_t i = 0; i < v.size(); i++) {
        cout << v[i] << " ";
    }
}

void
printSeq()
{
    for (size_t i = 0; i < seq.size(); i++) {
        for (size_t j = 0; j < seq[i].size(); j++) {
            cout << seq[i][j] << " ";
        }
        cout << endl;
    }
}
