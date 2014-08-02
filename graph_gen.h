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

class invalid_weightGen: public exception {
    virtual const char* what() const throw() {
        return "You didn't call Graph.set_max_weight()!";
    }
};

template<typename T1, typename T2>
auto randrange(T1 bottom, T2 top) -> decltype(bottom+top) {
    return double(rand())/RAND_MAX * (top - bottom) + bottom;
}

template<>
int randrange(int bottom, int top) {
    return rand() % (top - bottom) + bottom;
}

template<>
long unsigned int randrange(int bottom, long unsigned int top) {
    return rand() % (top - bottom) + bottom;
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

template<typename T>
class RandomWeights {
private:
    T max_weight;
    T min_weight;
public:
    typedef T weight_type;
    RandomWeights(): max_weight(0), min_weight(0) {}
    bool is_sane() {return max_weight > 0;}
    void set_weight_range(T min, T max) {
        max_weight = max;
        min_weight = min;
    }
    void print(int a, int b) {
        cout << " " << randrange(min_weight, max_weight);
    }
};

class NoWeights {
public:
    typedef int weight_type;
    bool is_sane() {return true;}
    void set_weight_range(int, int) {}
    void print(int a, int b) {}
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
            sval[i] = randrange(0, max - num - excl.size());
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

template<typename T1, typename T2 = NoWeights>
class Graph {
private:
    T1 labelGen;
    T2 weightGen;
    unordered_set<int>* adjList;
    typedef typename T2::weight_type weight_type;
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
            int s = randrange(0, v+1);
            add_edge(s, v+1);
            add_edge(v+1, s);
        }
    }

    void set_weight_range(weight_type bottom, weight_type top) {
        weightGen.set_weight_range(bottom, top);
    }

    void print_undirected() {
        if(!weightGen.is_sane()) throw invalid_weightGen();
        int nedg = 0;
        for(unsigned i=0; i<labelGen.size(); i++)
            for(auto oth: adjList[i])
                if(i > oth)
                    nedg++;

        cout << labelGen.size() << " " << nedg << endl;
        for(unsigned i=0; i<labelGen.size(); i++)
            for(auto oth: adjList[i]) {
                if(i <= oth) continue;
                cout << labelGen.get(i) << " " << labelGen.get(oth);
                weightGen.print(i, oth);
                cout << endl;
            }
    }
};
