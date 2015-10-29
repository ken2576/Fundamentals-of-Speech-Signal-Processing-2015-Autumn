#include "hmm.h"
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

#include <iomanip>

using namespace std;


static void
usage()
{
    cout << "Usage: modCalc [ <iteration> <model_init.txt>"   ;
    cout <<                  "<seq_model_txt> <output_model>]";
}

static void
myexit()
{
    usage();
    exit(-1);
}

//***read sequence
void readSeqs(const char *filename, vector<vector<int> > &trainingSeqs);

//***Baum Welch
void baumWelch(vector<vector<int> > &trainingSeqs, HMM &hmm_output, HMM hmm_input);

//***calculate various parameters
vector<vector<double> > calcalpha(vector<vector<int> > &trainingSeqs, HMM &hmm_input, int datacount);
vector<vector<double> > calcbeta(vector<vector<int> > &trainingSeqs, HMM &hmm_input, int datacount);
vector<vector<double> > calcgamma(vector<vector<double> > &, vector<vector<double> > &);
vector<vector<vector<double> > > calcepsilon(vector<vector<double> > &, vector<vector<double> > &, HMM &hmm, vector<vector<int> > &, int);

//***print functions
void printMatrix(vector<vector<double> >);
void printMatrices(vector<vector<vector<double> > >);
void printVector(vector<double>);

//***various summation function
vector<double> matrixSum(vector<vector<double> > , size_t);
vector<vector<double> > matricesSum(vector<vector<vector<double> > > , size_t);

//***various operator
vector<double> vectorAdd(vector<double> &, vector<double> &);
vector<vector<double> > matrixAdd(vector<vector<double> > &, vector<vector<double> > &);
vector<double> vectorDivision(vector<double> &, double );

//***used when needs to calculate new aij and bij
vector<vector<double> > aijDivision(vector<vector<double> > & , vector<double> &);
vector<vector<double> > bijSum(vector<vector<double> > &, vector<vector<int> > &, int);

//***initialize matrix
void matrixInit(vector<vector<double> > &, int , int );

//***setter functions
void setInit(vector<double>, HMM &);
void setObsv(vector<vector<double> >, HMM &);
void setTrans(vector<vector<double> >, HMM &);



int
main (int argc, char** argv)
{
    const char *iter_str, *init_filename, *seq_filename, *output_filename;
    HMM hmm_initial, hmm_output;
    size_t iterations;

    if (argc == 5) {
        iter_str = argv[1];
        init_filename = argv[2];
        seq_filename = argv[3];
        output_filename = argv[4];
    }
    else
        myexit();

    stringstream str2num;
    str2num << iter_str;
    str2num >> iterations;

    vector<vector<int> > trainingSeqs;
    loadHMM( &hmm_initial, init_filename);
    readSeqs(seq_filename, trainingSeqs);
    baumWelch(trainingSeqs, hmm_output, hmm_initial);

    if (iterations > 1)
    {
        for (size_t i=1; i<iterations; i++)
        {
            baumWelch(trainingSeqs, hmm_output, hmm_output);
        }
    }

    FILE *fp = open_or_die(output_filename, "w");
    dumpHMM(fp, &hmm_output);
    return 0;
}

void
readSeqs(const char *filename, vector<vector<int> > &trainingSeqs)
{
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
baumWelch(vector<vector<int> > &trainingSeqs, HMM &hmm_output, HMM hmm_input)
{
    vector<vector<double> > alpha, beta, gamma, epssum, bijsum, newaij, newbij, tmpmatrix;
    vector<double> newpi, gammasum, gammasumT, tmpvector;
    vector<vector<vector<double> > > eps;
    int datacount = 10000;///????
    int T = trainingSeqs[0].size();


    newpi.resize(hmm_input.state_num);

    matrixInit(epssum, hmm_input.state_num, hmm_input.state_num);
    matrixInit(bijsum, hmm_input.state_num, hmm_input.state_num);

    gammasum.resize(hmm_input.state_num);
    gammasumT.resize(hmm_input.state_num);
    for (int data=0; data<datacount; data++)
    {
        alpha = calcalpha(trainingSeqs, hmm_input, data);
        beta = calcbeta(trainingSeqs, hmm_input, data);
        gamma = calcgamma(alpha, beta);
        eps = calcepsilon(alpha, beta, hmm_input, trainingSeqs, data);
        newpi = vectorAdd(newpi, gamma[0]);

        tmpmatrix = matricesSum(eps, T-1);
        epssum = matrixAdd(epssum, tmpmatrix);

        tmpvector = matrixSum(gamma, T-1);
        gammasum = vectorAdd(gammasum, tmpvector);

        tmpmatrix = bijSum(gamma, trainingSeqs, data);
        bijsum = matrixAdd(bijsum, tmpmatrix);

        tmpvector = matrixSum(gamma, T);
        gammasumT = vectorAdd(gammasum, tmpvector);
    }
    newpi = vectorDivision(newpi, 10000.0);
    setInit(newpi, hmm_output);

    newaij = aijDivision(epssum, gammasum);
    setTrans(newaij, hmm_output);

    newbij = aijDivision(bijsum, gammasumT);
    setObsv(newbij, hmm_output);

}


vector<vector<double> >
calcalpha(vector<vector<int> > &trainingSeqs, HMM  &hmm_input, int datacount)
{
    vector<vector<double> > alpha;
    vector<double> tmp;
    double tmpresult;
    int t = 0;
    int state;
    double sum;
    ///////alpha 1
    state = trainingSeqs[datacount][t];
    for (int i=0; i<hmm_input.state_num; i++)
    {
        tmpresult = hmm_input.initial[i] * hmm_input.observation[state][i];
        tmp.push_back(tmpresult);
    }
    alpha.push_back(tmp);
    //////
    for (t = 0; t<trainingSeqs[0].size() - 1; t++)
    {
        tmp.clear();
        tmpresult = 0;
        sum = 0;
        state = trainingSeqs[datacount][t+1];
        for (int j=0; j<hmm_input.state_num; j++)
        {
            sum = 0;
            for (int i=0; i<hmm_input.state_num; i++)
            {
                sum += alpha[t][i] * hmm_input.transition[j][i];////????
            }
            tmpresult = hmm_input.observation[state][j] * sum;
            tmp.push_back(tmpresult);
        }
        alpha.push_back(tmp);
    }
    return alpha;
}

vector<vector<double> >
calcbeta(vector<vector<int> > &trainingSeqs, HMM &hmm_input, int datacount)
{
    vector<vector<double> > beta;
    vector<double> tmp;
    double tmpresult, sum;
    int state;
    int t;
    tmpresult = 0;
    t = 49;
    beta.resize(t+1);
    ///////beta 1
    for (int i=0; i<hmm_input.state_num; i++)
    {
        tmpresult = 1;
        tmp.push_back(tmpresult);
    }
    beta[t] = tmp;

    for (t = 49; t > 0; t--)
    {
        tmp.clear();
        tmpresult = 0;
        sum = 0;
        state = trainingSeqs[datacount][t];
        for (int i=0; i<hmm_input.state_num; i++)
        {
            sum = 0;
            for (int j=0; j<hmm_input.state_num; j++)
            {
                sum += beta[t][j] * hmm_input.transition[j][i] * hmm_input.observation[state][j]; ///?????
            }
            tmp.push_back(sum);
        }
        beta[t-1] = tmp;
    }
    return beta;
}

//returns a T*N matrix
vector<vector<double> >
calcgamma(vector<vector<double> > &a, vector<vector<double> > &b)
{
    vector<vector<double> > gamma;
    vector<double> tmp;
    double tmpresult = 0; //store calculation result
    double sum = 0;
    for (size_t t=0; t<a.size(); t++)
    {
        sum = 0;
        tmp.clear();
        for (size_t i=0; i<a[0].size(); i++)
        {
            sum += a[t][i] * b[t][i];
        }
        for (size_t j=0; j<a[0].size(); j++)
        {

            tmpresult = a[t][j] * b[t][j] / sum;
            tmp.push_back(tmpresult);
        }
        gamma.push_back(tmp);
    }
    return gamma;
}


//returns T-1 N*N Matrices
vector<vector<vector<double> > >
calcepsilon(vector<vector<double> > &a, vector<vector<double> > &b, HMM &hmm_input, vector<vector<int> > &trainingSeqs, int datacount)
{
    vector<vector<vector<double> > > epsmatrix;
    vector<vector<double> > eps;
    vector<double> tmp;
    double tmpresult = 0;
    double sum = 0;
    int state = 0;


    for (size_t t=1; t<a.size(); t++)
    {
        sum = 0;
        state = trainingSeqs[datacount][t];
        for (size_t j=0; j<a[0].size(); j++)
        {
            for (size_t i=0; i<a[0].size(); i++)
            {
                sum += a[t-1][i] * hmm_input.transition[j][i] * hmm_input.observation[state][j] * b[t][j];///???
            }
        }

        eps.clear();
        for (size_t j=0; j<a[0].size(); j++)
        {
            tmp.clear();
            for (size_t i=0; i<a[0].size(); i++)
            {
                tmpresult = a[t-1][i] * hmm_input.transition[j][i] * hmm_input.observation[state][j] * b[t][j] / sum;///????
                tmp.push_back(tmpresult);
            }
            eps.push_back(tmp);
        }
        epsmatrix.push_back(eps);
    }
    return epsmatrix;
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

vector<double>
matrixSum(vector<vector<double> > a, size_t T)
{
    vector<double> sum;
    sum.resize(a[0].size());
    if (T>a.size())
        T=a.size();
    for (size_t j=0; j<a[0].size(); j++)
    {
        for (size_t i=0; i<T; i++)
        {
            sum[j] += a[i][j];
        }
    }
    return sum;
}

vector<vector<double> >
matrixAdd(vector<vector<double> > &a, vector<vector<double> > &b)
{
    vector<vector<double> > result;
    result.resize(a.size());
    for(size_t n=0; n<result.size(); n++)
    {
        result[n].resize(a[0].size());
    }
    for(size_t i=0; i<result.size(); i++)
    {
        for(size_t j=0; j<result[0].size(); j++)
        {
            result[i][j] = a[i][j] + b[i][j];
        }
    }
    return result;
}

vector<vector<double> >
matricesSum(vector<vector<vector<double> > > a, size_t T)
{
    vector<vector<double> > sum = a[0];
    for (size_t n=1; n<T; n++)
    {
        sum = matrixAdd(sum, a[n]);
    }
    return sum;
}

vector<double>
vectorAdd(vector<double> &a, vector<double> &b)
{
    vector <double> result;
    result.resize(a.size());
    for (size_t n=0; n<a.size(); n++)
    {
        result[n] = a[n] + b[n];
    }
    return result;
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

vector<vector<double> >
aijDivision(vector<vector<double> > &a, vector<double> &b)
{
    vector<vector<double> > result;
    result.clear();
    result.resize(a.size());
    for(size_t n=0; n<result.size(); n++)
    {
        result[n].resize(a[0].size());
    }
    for(size_t j=0; j<result[0].size(); j++)
    {
        for(size_t i=0; i<result.size(); i++)
        {
            result[i][j] = a[i][j] / b[j];
        }
    }
    return result;
}


vector<vector<double> >
bijSum(vector<vector<double> > &a, vector<vector<int> > &trainingSeqs, int datacount)
{
    vector<vector<double> > result;
    int state;
    result.resize(a[0].size());
    for(size_t n=0; n<result.size(); n++)
    {
        result[n].resize(a[0].size());
    }
    for (size_t n=0; n<trainingSeqs[0].size(); n++)
    {
        state = trainingSeqs[datacount][n];
        result[state] = vectorAdd(result[state], a[n]);
    }
    return result;
}



vector<double>
vectorDivision(vector<double> &a, double t)
{
    vector<double> result;
    result.resize(a.size());
    for (size_t n=0; n<a.size(); n++)
    {
        result[n] = a[n] / t;
    }
    return result;
}


void
matrixInit(vector<vector<double> > &a, int i, int j)
{
    a.resize(i);
    for(size_t n=0; n<a.size(); n++)
    {
        a[n].resize(j);
    }
}



void
setInit(vector<double> v, HMM &hmm)
{
    for (size_t n=0; n<v.size(); n++)
    {
        hmm.initial[n] = v[n];
    }
    hmm.state_num = 6;
}


void
setObsv(vector<vector<double> > v, HMM &hmm)
{
    for (size_t j=0; j<v.size(); j++)
    {
        for (size_t i=0; i<v[0].size(); i++)
        {
            hmm.observation[j][i] = v[j][i];
        }
    }
    hmm.observ_num = 6;
}

void
setTrans(vector<vector<double> > v, HMM &hmm)
{
    for (size_t j=0; j<v.size(); j++)
    {
        for (size_t i=0; i<v[0].size(); i++)
        {
            hmm.transition[j][i] = v[j][i];
        }
    }
    hmm.state_num = 6;
}
