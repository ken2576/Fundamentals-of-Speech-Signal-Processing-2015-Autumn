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

struct deltaTri {
    unsigned short curr;
    unsigned short mid;
    unsigned short prev;
    double prob;
};

struct state {
}

//Static Variables
static Vocab voc;
static vector< vector<string> > seq;
static vector< vector<double> > delta;
static vector< vector<deltaTri> > deltaTrigram;
static vector< vector<unsigned short> > psi;
static Table tb;
static Ngram* lm = NULL;
static bool trigram = false;

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



//Forward Declaration
unsigned short big5toShort(const char*);
unsigned char* shorttoBig5(unsigned short);
VocabIndex getCharIndex(const char*);

double getUnigramProb(const char*);
double getBigramProb(const char*, const char*);
double getTrigramProb(const char*, const char*, const char*);

void viterbi(size_t);
void viterbiTri(size_t);
vector<string> backTrack();
vector<string> backTrackTri();

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

    //determine trigram or not
    if (orderNum == 3)
        trigram = true;

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
            if (tok != "")
                vs.push_back(tok);
        }
        seq.push_back(vs);
    }
    fs.close();

    //printSeq();

    //viterbi
    vector<string> sentence;
    for (size_t sample = 0; sample < seq.size(); sample++) {
        if (trigram == false)
            viterbi(sample);
        else
            viterbiTri(sample);

        if (trigram == false)
            sentence = backTrack();
        else
            sentence = backTrackTri();
        cout << "<s> ";
        for (size_t w = 0; w < sentence.size(); w++) {
            cout << sentence[w] << " ";
        }
        cout << "</s>\n";
    }
    return 0;
}

unsigned short
big5toShort(const char* str)
{
    //bitmasking the character
    unsigned short tmp;
    tmp = ((unsigned char)str[0] << 8 | (unsigned char)str[1]);
    return tmp;
}

unsigned char*
shorttoBig5(unsigned short s)
{
    //need to use char[3] instead of string or will result in no output
    unsigned char* str = new unsigned char [3];
    str[0] = (unsigned char)(s >> 8);
    str[1] = (unsigned char)(s & 0x00FF);
    str[2] = NULL;
    return str;
}

VocabIndex
getCharIndex(const char* word)
{
    VocabIndex wid = voc.getIndex(word);
    return wid;
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
double
getUnigramProb(const char *w1)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex("<s>");

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid2, Vocab_None };
    return lm->wordProb( wid1, context);
}


// Get P(W2 | W1) -- bigram
double
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
double
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

    delta.resize(seq[k].size() + 1);
    psi.resize(seq[k].size() + 1);

    //initialization delta_0 = 1 (0.0 in log10)
    Table::iterator it;
    string str;
    vector<unsigned short> vi, vj, vk;
    unsigned short key;
    str = seq[k][0];
    key = big5toShort(str.c_str());
    it = tb.find(key);
    vi = (*it).second;
    for (size_t i = 0; i < vi.size(); i++) {
        string ch;
        ch.assign((const char*)(shorttoBig5(vi[i])));
        double p = getUnigramProb(ch.c_str());
        delta[0].push_back(p);
    }

    //iterate bigram case
    double currProb = 0.0;
    unsigned char *w1, *w2;
    for (size_t t = 1; t < seq[k].size(); t++) {
        str = seq[k][t];
        key = big5toShort(str.c_str());
        it = tb.find(key);
        vj = (*it).second;

        str = seq[k][t-1];
        key = big5toShort(str.c_str());
        it = tb.find(key);
        vi = (*it).second;

        double maxProb;
        unsigned short maxStateWord;
        for (size_t j = 0; j < vj.size(); j++) {
            maxProb = -INFINITY;
            maxStateWord = 0;
            w2 = shorttoBig5(vj[j]);
            //double bj = getUnigramProb((const char*)(w2));
            for (size_t i = 0; i < vi.size(); i++) {
                w1 = shorttoBig5(vi[i]);
                //cout << w1 << " " << w2 << endl;
                double aij = getBigramProb((const char*)(w1), (const char*)(w2));
                currProb = delta[t-1][i] + aij;
                if (currProb > maxProb) {
                    maxProb = currProb;
                    maxStateWord = vi[i];
                }
            }
            //maxProb += bj;
            delta[t].push_back(maxProb);
            psi[t].push_back(maxStateWord);
        }
    }
    size_t end = seq[k].size()-1;
    str = seq[k][end];
    key = big5toShort(str.c_str());
    it = tb.find(key);
    vi = (*it).second;
    double maxProb = -INFINITY;
    unsigned short maxStateWord = 0;
    for (size_t i = 0; i < delta[end].size(); i++) {
        if (delta[end][i] > maxProb) {
            maxProb = delta[end][i];
            maxStateWord = vi[i];
        }
    }
    delta[end+1].push_back(maxProb);
    psi[end+1].push_back(maxStateWord);
}

void
viterbiTri(size_t k)
{
    //clear delta vector
    if (!deltaTrigram.empty()) {
        for (size_t i = 0; i < deltaTrigram.size(); i++) {
            deltaTrigram[i].clear();
        }
        deltaTrigram.clear();
    }

    //clear psi vector
    if (!psi.empty()) {
        for (size_t i = 0; i < psi.size(); i++) {
            psi[i].clear();
        }
        psi.clear();
    }

    deltaTrigram.resize(seq[k].size() + 1);
    psi.resize(seq[k].size() + 1);

    //initialization
    Table::iterator it;
    string str;
    vector<unsigned short> vi, vj, vk;
    unsigned short key;
    str = seq[k][0];
    key = big5toShort(str.c_str());
    it = tb.find(key);
    vi = (*it).second;
    for (size_t i = 0; i < vi.size(); i++) {
        string ch;
        ch.assign((const char*)(shorttoBig5(vi[i])));
        deltaTri d;
        d.curr = vi[i];
        d.prob = getUnigramProb(ch.c_str());
        deltaTrigram[0].push_back(d);
    }

    double currProb = 0.0;
    unsigned char *w1, *w2, *w3;

    str = seq[k][0];
    key = big5toShort(str.c_str());
    it = tb.find(key);
    vi = (*it).second;

    str = seq[k][1];
    key = big5toShort(str.c_str());
    it = tb.find(key);
    vj = (*it).second;

    double maxProb;
    unsigned short maxStateWord;
    for (size_t j = 0; j < vj.size(); j++) {
        maxProb = -INFINITY;
        maxStateWord = 0;
        w2 = shorttoBig5(vj[j]);
        for (size_t i = 0; i < vi.size(); i++) {
            w1 = shorttoBig5(vi[i]);
            double aij = getTrigramProb("<s>", (const char*)(w1), (const char*)(w2));
            currProb = deltaTrigram[0][i].prob + aij;
            if (currProb > maxProb) {
                maxProb = currProb;
                maxStateWord = vi[i];
            }
        }
        deltaTri d;
        d.prob = maxProb;
        d.curr = vj[j];
        d.mid = maxStateWord;
        d.prev = maxStateWord;
        deltaTrigram[1].push_back(d);
    }

    //iterate trigram case
    for (size_t t = 2; t < seq[k].size(); t++) {
        str = seq[k][t];
        key = big5toShort(str.c_str());
        it = tb.find(key);
        vk = (*it).second;

        maxProb = -INFINITY;
        maxStateWord = 0;
        w3 = shorttoBig5(vk[k]);
        for (size_t i = 0; i < deltaTrigram[t-1].size(); i++) {
            w2 = shorttoBig5(deltaTrigram[t-1][i].curr);
            w1 = shorttoBig5(deltaTrigram[t-1][i].mid);
            double aij = getTrigramProb((const char*)(w1), (const char*)(w2),
                    (const char*)(w3));
            currProb = deltaTrigram[t-1][i].prob + aij;
            if (currProb > maxProb) {
                maxProb = currProb;
                maxStateWord = deltaTrigram[t-1][i].prev;
            }
        }
        deltaTri d;
        d.prob = maxProb;
        d.curr = vk[k];
        d.mid = big5toShort((const char*)(w2));
        d.prev = big5toShort((const char*)(w1));
        deltaTrigram[t].push_back(d);
        psi[t].push_back(maxStateWord);
    }
}

vector<string>
backTrack()
{
    vector<string> ret;
    size_t end = 0;
    for (size_t t = delta.size()-1; t > end; --t) {
        double maxProb = -INFINITY;
        unsigned short maxStateWord = 0;
        for (size_t i = 0; i < delta[t].size(); i++) {
            if (delta[t][i] > maxProb) {
                maxProb = delta[t][i];
                maxStateWord = psi[t][i];
            }
        }
        string str;
        str.assign((const char*)(shorttoBig5(maxStateWord)));
        ret.push_back(str);
    }
    reverse(ret.begin(), ret.end());
    return ret;
}

vector<string>
backTrackTri()
{
    vector<string> ret;
    size_t end = 1;
    if (trigram)
        end = 1;
    for (size_t t = deltaTrigram.size()-1; t > end; --t) {
        double maxProb = -INFINITY;
        unsigned short maxStateWord = 0;
        for (size_t i = 0; i < deltaTrigram[t].size(); i++) {
            if (deltaTrigram[t][i].prob > maxProb) {
                maxProb = deltaTrigram[t][i].prob;
                maxStateWord = psi[t][i];
            }
        }
        string str;
        str.assign((const char*)(shorttoBig5(maxStateWord)));
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
    int count = 0;
    for (size_t i = 0; i < s.size(); i++) {
        for (size_t j = 0; j < s[i].size(); j++) {
            count++;
            cout << s[i][j] << " ";
            if ((count % 15) == 0)
                cout << endl;
        }
        cout << endl << "---------------------------------------" << endl;
    }
}
