#include <iostream>
#include <cstring>
#include <cmath>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include "./srilm-1.5.10/lm/src/Ngram.h"
#include "./srilm-1.5.10/lm/src/Vocab.h"

using namespace std;

typedef map<unsigned short, vector<unsigned short> > Table;

//Static Variables
static Vocab voc;
static vector< vector<string> > seq;
static vector< vector<double> > delta;
static vector< vector<unsigned short> > psi;
static Table tb;
static Ngram* lm = NULL;

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
big5toShort(const char* str)
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

void viterbi(size_t);
vector<string> backTrack();

//Printer Functions
void printVector(const vector<string>&);
void printSeq();
void printDeltaMatrix(const vector<vector<double> >&);
void printPsiMatrix(const vector<vector<unsigned short> >&);

//Utility Functions
size_t strGetTok(const string&, string&, size_t pos = 0, const char delim = ' ');


int
main(int argc, char** argv) {
    const char *text, *zymap, *lm_name, *order;
    int orderNum;
    if (argc == 5) {
        text = argv[1];
        zymap = argv[2];
        lm_name = argv[3];
        order = argv[4];
    }
    else
        myexit();

    //read order
    stringstream ss;
    ss << order;
    ss >> orderNum;

    //create Ngram Language Model
    lm = new Ngram(voc, orderNum);
    File lmFile(lm_name, "r");
    lm->read(lmFile);
    lmFile.close();

    //create ZhuyingBig5 map
    fstream fs;
    string str, tok;
    vector<unsigned short> v;
    size_t pos;
    unsigned short key;
    fs.open(zymap, ios::in);
    while (getline(fs, str)) {
        v.clear();
        tok.clear();
        pos = 0;
        pos = strGetTok(str, tok, pos);
        key = big5toShort(tok.c_str());
        while (pos != string::npos) {
            pos = strGetTok(str, tok, pos);
            v.push_back(big5toShort(tok.c_str()));
        }
        tb[key] = v;
    }
    fs.close();

    //read text into seq
    fs.open(text, ios::in);
    vector<string> vs;
    while (getline(fs, str)) {
        vs.clear();
        tok.clear();
        pos = 0;
        while (pos != string::npos) {
            pos = strGetTok(str, tok, pos);
            if (strcmp(tok.c_str(), "\n") != 0)
                vs.push_back(tok);
        }
        seq.push_back(vs);
    }
    fs.close();

//    printSeq();

    //viterbi
    vector<string> sentence;
//    for (size_t sample = 0; sample < seq.size(); sample++) {
//        viterbi(sample);

        viterbi(0);
        printDeltaMatrix(delta);
        printPsiMatrix(psi);
        sentence = backTrack();
        cout << "<s> ";
        for (size_t w = 0; w < sentence.size(); w++) {
            cout << sentence[w] << " ";
        }
        cout << "</s>\n";

//    }
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
    return lm->wordProb( wid1, context);
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
    return lm->wordProb( wid2, context);
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
    return lm->wordProb( wid3, context );
}


void
viterbi(size_t k)
{
    //clear delta vector
    if (!delta.empty()) {
        for (size_t i = 0; i < delta.size(); i++) {
            delta[i].clear();
        }
        delta.clear();
    }

    //clear psi vector
    if (!psi.empty()) {
        for (size_t i = 0; i < psi.size(); i++) {
            psi[i].clear();
        }
        psi.clear();
    }

    delta.resize(seq[k].size());
    psi.resize(seq[k].size());

    //initialization delta_0 = 1 (0.0 in log10)
    Table::iterator it;
    string str;
    vector<unsigned short> vi, vj;
    unsigned short key;
    str = seq[k][0];
    key = big5toShort(str.c_str());
    it = tb.find(key);
    vi = (*it).second;
    for (size_t i = 0; i < vi.size(); i++) {
        delta[0].push_back(0.0);
    }

    //iterate
    double currProb = 0.0;
    for (size_t t = 1; t < seq[k].size()-1; t++) {
        str = seq[k][t];
        key = big5toShort(str.c_str());
        it = tb.find(key);
        vj = (*it).second;

        str = seq[k][t-1];
        key = big5toShort(str.c_str());
        it = tb.find(key);
        vi = (*it).second;


        for (size_t j = 0; j < vj.size(); j++) {
            double maxProb = -INFINITY;
            unsigned short maxStateWord = 0;
            for (size_t i = 0; i < vi.size(); i++) {
                double aij = getBigramProb(shorttoBig5(vi[i]), shorttoBig5(vj[j]));
                currProb = delta[t-1][i] + aij;
                if (currProb > maxProb) {
                    maxProb = currProb;
                    maxStateWord = vj[j];
                }
            }
            delta[t].push_back(maxProb);
            psi[t].push_back(maxStateWord);
        }
    }
}

vector<string>
backTrack()
{
    vector<string> ret;
    for (size_t t = delta.size()-1; t > 0; --t) {
        cout << t << endl;
        double maxProb = -INFINITY;
        unsigned short maxStateWord = 0;
        for (size_t i = 0; i < delta[t].size(); i++) {
            double currProb = delta[t][i];
            if (currProb > maxProb) {
                maxProb = currProb;
                maxStateWord = psi[t][i];
            }
        }
        string str(shorttoBig5(maxStateWord));
        ret.push_back(str);
    }
    reverse(ret.begin(), ret.end());
    return ret;
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

void
printDeltaMatrix(const vector<vector<double> >& d)
{
    cout << "======Delta======" << endl;
    int count = 0;
    for (size_t i = 0; i < d.size(); i++) {
        for (size_t j = 0; j < d[i].size(); j++) {
            count++;
            cout << d[i][j] << " ";
            if ((count % 15) == 0)
                cout << endl;
        }
        cout << endl << "---------------------------------------" << endl;
    }
}

void
printPsiMatrix(const vector<vector<unsigned short> >& s)
{
    cout << "======Psi======" << endl;
    for (size_t i = 0; i < s.size(); i++) {
        for (size_t j = 0; j < s[i].size(); j++) {
            cout << s[i][j] << " ";
        }
        cout << endl;
    }
}
