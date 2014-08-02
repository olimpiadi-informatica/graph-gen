#include <vector>
#include <unordered_set>
#include <algorithm>
#include <exception>
#include <iostream>
using namespace std;

class too_many_edges: public exception {
    virtual const char* what() const throw() {
        return "You specified too many edges!";
    }
};

class too_few_edges: public exception {
    virtual const char* what() const throw() {
        return "You specified too few edges!";
    }
};

class too_few_nodes: public exception {
    virtual const char* what() const throw() {
        return "You specified too few nodes!";
    }
};

class too_many_samples: public exception {
    virtual const char* what() const throw() {
        return "You specified too many values to sample from the given range!";
    }
};

int randint(int limit) {
    return rand() % limit;
}

class IntLabels {
private:
    vector<int> labels;
public:
    IntLabels(int num, int start = 0) {
        labels.resize(num);
        for(int i=0; i<num; i++)
            labels[i] = i+start;
    }
    unsigned size() {
        return labels.size();
    }
    int get(int i) {
        return labels[i];
    }
};

class RandomIntLabels {
private:
    vector<int> labels;
public:
    RandomIntLabels(int num, int start = 0) {
        labels.resize(num);
        for(int i=0; i<num; i++)
            labels[i] = i+start;
        random_shuffle(labels.begin(), labels.end());
    }
    unsigned size() {
        return labels.size();
    }
    int get(int i) {
        return labels[i];
    }
};

template<typename T>
class VectorLabels {
private:
    vector<T>& vec;
public:
    VectorLabels(vector<T>& v): vec(v) {}
    unsigned size() {
        return vec.size();
    }
    T get(int i) {
        return vec[i];
    }
};

class linear_sample {
private:
    vector<int> sval;
public:
    // we assume -1 < excl[0] < excl[1] < ... < excl[N-1] < max
    linear_sample(int num, int max, const vector<int>& excl = vector<int>()) {
        if(max - num - excl.size() < 1) throw too_many_samples();
        sval.resize(num);
        for(int i=0; i<num; i++)
            sval[i] = randint(max - num - excl.size());
        sort(sval.begin(), sval.end());
        int j = 0;
        for(int i=0; i<num; i++) {
            if(j < (int) excl.size() && excl[j] == sval[i] + i + j) j++;
            sval[i] += i + j;
        }

    }

    vector<int>::iterator begin() {
        return sval.begin();
    }

    vector<int>::iterator end() {
        return sval.end();
    }
};

template<typename T>
class Graph {
private:
    T labelGen;
    unordered_set<int>* adjList;
public:
    template<typename... Args>
    Graph(Args... params): labelGen(params...) {
        if(labelGen.size() == 0) throw too_few_nodes();
        adjList = new unordered_set<int>[labelGen.size()];
    }

    ~Graph() {
        delete[] adjList;
    }

    void add_edge(int a, int b) {
        adjList[a].insert(b);
    }

    void build_forest(int M) {
        if(M > labelGen.size()-1) throw too_many_edges();
        for(auto v: linear_sample(M, labelGen.size())) {
            int s = randint(v+1);
            add_edge(s, v+1);
            add_edge(v+1, s);
        }
    }

    void print_undirected() {
        int nedg = 0;
        for(unsigned i=0; i<labelGen.size(); i++)
            for(auto oth: adjList[i])
                if(i > oth)
                    nedg++;

        cout << labelGen.size() << " " << nedg << endl;
        for(unsigned i=0; i<labelGen.size(); i++)
            for(auto oth: adjList[i]) {
                if(i <= oth) continue;
                cout << labelGen.get(i) << " " << labelGen.get(oth) << endl;
            }
    }
};
