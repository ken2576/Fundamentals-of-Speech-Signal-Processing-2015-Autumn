#include "hmm.h"
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <string>

using namespace std;

static void
usage()
{
    cout << "Usage: modCalc [ <modellist.txt>, <testing_data.txt>, <result.txt>, [answer.txt] ]" << endl;
}

static void
myexit()
{
    usage();
    exit(-1);
}


void readSeqs(const char *filename, vector<vector<int> > &trainingSeqs);
void viterbi(vector<vector<int> > &trainingSeqs, HMM hmms[5], vector<string> &, vector<double> &);

void outputAnswer(const char *, vector<string>, vector<double>);
void readAns( const char* file_name, vector<string> &answer);
void calcAcc( vector<string> &answer, vector<string> &result );

void printMatrix(vector<vector<double> >);
void printMatrices(vector<vector<vector<double> > >);
void printVector(vector<double>);


int
main (int argc, char** argv){
    const char *model_filename, *test_filename, *result_filename, *answer_filename;

    if (argc == 5) {
        model_filename = argv[1];
        test_filename = argv[2];
        result_filename = argv[3];
        answer_filename = argv[4];
    }
    else if (argc == 4) {
        model_filename = argv[1];
        test_filename = argv[2];
        result_filename = argv[3];
        answer_filename = 0;
    }
    else
        myexit();

    HMM hmms[5];
    load_models(model_filename, hmms, 5);

    vector<vector<int> > trainingSeqs;
    vector<string> result, answer;
    vector<double> answerP;
    readSeqs(test_filename, trainingSeqs);
    viterbi(trainingSeqs, hmms, result, answerP);

    outputAnswer(result_filename, result, answerP);
    if (answer_filename) {
        readAns(answer_filename, answer);
        calcAcc(answer, result);
    }
    return 0;
}



void
readSeqs(const char *filename, vector<vector<int> > &trainingSeqs) {
    fstream fs;
    string strtmp;
    stringstream str2num;
    fs.open(filename, ios::in);
    while (getline(fs, strtmp)){
        vector<int> tmp;
        tmp.clear();
        for (size_t i=0; i<strtmp.length(); i++)
        {
            switch(strtmp[i]){
                case 'A':
                    tmp.push_back(0);
                    break;
                case 'B':
                    tmp.push_back(1);
                    break;
                case 'C':
                    tmp.push_back(2);
                    break;
                case 'D':
                    tmp.push_back(3);
                    break;
                case 'E':
                    tmp.push_back(4);
                    break;
                case 'F':
                    tmp.push_back(5);
                    break;
            }
        }
        trainingSeqs.push_back(tmp);
    }
}

void
viterbi(vector<vector<int> > &trainingSeqs, HMM hmms[5], vector<string> &answers, vector<double> &answerP) {
    vector<vector<vector<double> > > deltas;
    vector<vector<double> > tmpmatrix;
    vector<double> tmpvector;
    vector<double> maxP;
    double tmpresult, ansP;
    vector<double> tmpdelta;
    int state, state_num;
    int datacount = 0;
    int t = 0;
    int T = trainingSeqs[0].size();
    state_num = hmms[0].state_num;


    while (datacount < 2500) {
        t = 0;
        state = trainingSeqs[datacount][t];
        deltas.clear();

        ///initialization
        for (int k=0; k<5; k++)
        {
            tmpvector.clear();
            tmpmatrix.clear();
            for (int n=0; n<state_num; n++)
            {
                tmpvector.push_back(hmms[k].initial[n] * hmms[k].observation[n][state]);
            }
            tmpmatrix.push_back(tmpvector);
            deltas.push_back(tmpmatrix);
        }

        ///recursion
        for (t = 1; t < T; t++)
        {
            state = trainingSeqs[datacount][t];
            for (int k=0; k<5; k++)
            {
                tmpdelta.clear();
                for (int j=0; j<state_num; j++)
                {
                    tmpvector.clear();
                    tmpresult = 0;
                    for (int i=0; i<state_num; i++)
                    {
                        tmpresult = deltas[k][t-1][i] * hmms[k].transition[j][i];
                        tmpvector.push_back(tmpresult);
                    }
                    tmpresult = *max_element(tmpvector.begin(), tmpvector.end());
                    tmpresult = tmpresult * hmms[k].observation[state][j];
                    tmpdelta.push_back(tmpresult);
                }
                deltas[k].push_back(tmpdelta);
            }
        }

        //printMatrices(deltas);
        maxP.clear();
        maxP.resize(5);
        for (int k=0; k<5; k++)
        {
            maxP[k] = *max_element(deltas[k][T-1].begin(), deltas[k][T-1].end());
            //cout << "k  " << k << "  maxP:  " << maxP[k] << endl;
        }

        int model_number = max_element(maxP.begin(), maxP.end()) - maxP.begin();

        switch(model_number) {
            case 0:
                answers.push_back("model_01.txt");
                break;
            case 1:
                answers.push_back("model_02.txt");
                break;
            case 2:
                answers.push_back("model_03.txt");
                break;
            case 3:
                answers.push_back("model_04.txt");
                break;
            case 4:
                answers.push_back("model_05.txt");
                break;
        }
        ansP = *max_element(maxP.begin(), maxP.end());
        answerP.push_back(ansP);

        datacount++;
    }
}


void
printMatrix(vector<vector<double> > a)
{
    for (size_t i=0; i<a.size(); i++)
    {
        for (size_t j=0; j<a[0].size(); j++)
        {
            cout << setw(16) << left << a[i][j];
        }
        cout << endl;
    }
}

void
printMatrices(vector<vector<vector<double> > > a)
{
    for (size_t k=0; k<a.size(); k++)
    {
        cout << "Matrix: " << k << endl;
        for (size_t i=0; i<a[0].size(); i++)
        {
            for (size_t j=0; j<a[0][0].size(); j++)
            {
                cout << setw(16) << left << a[k][i][j];
            }
            cout << endl;
        }
        cout << endl << endl;
    }
}

void
printVector(vector<double> a)
{
    for (size_t n=0; n<a.size(); n++)
    {
        cout << setw(16) << left << a[n];
    }
    cout << endl;
}




void outputAnswer(const char *result_filename, vector<string> answers, vector<double> answerP)
{
    fstream fs;
    fs.open(result_filename, ios::out);
    for (size_t n=0; n<answers.size(); n++)
    {
        fs << answers[n] << " " << answerP[n] << endl;
    }
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



void calcAcc( vector<string> &answer, vector<string> &result ) {
    double counter = 0.0;
    fstream fs;
    fs.open("acc.txt", ios::out);
    for (size_t n=0; n<answer.size(); n++)
    {
        if (answer[n] == result[n])
        {
            counter++;
        }
    }
    cout << "acc:  " << double(counter/answer.size()) << endl;
    fs << "acc: " << double(counter/answer.size()) << endl;
}
