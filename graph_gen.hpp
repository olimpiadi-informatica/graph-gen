#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>
#include <functional>
#include <sstream>
#include "cpp-btree/btree_set.h"
using namespace std;
using namespace btree;

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

class not_implemented: public exception {
    virtual const char* what() const throw() {
        return "This function is not implemented yet!";
    }
};

template<typename T1, typename T2>
auto randrange(T1 bottom, T2 top)
  -> typename enable_if<!is_integral<decltype(bottom+top)>::value,
                        decltype(bottom+top)>::type {
    return double(rand())/RAND_MAX * (top - bottom) + bottom;
}

template<typename T1, typename T2>
auto randrange(T1 bottom, T2 top)
  -> typename enable_if<is_integral<decltype(bottom+top)>::value,
                        decltype(bottom+top)>::type {
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
    size_t size() const {
        return labels.size();
    }
    int get(int i) const {
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
    size_t size() const {
        return labels.size();
    }
    int get(int i) const {
        return labels[i];
    }
};

template<typename T>
class VectorLabels {
private:
    vector<T>& vec;
public:
    VectorLabels(vector<T>& v): vec(v) {}
    size_t size() const {
        return vec.size();
    }
    T get(int i) const {
        return vec[i];
    }
};

// TODO (?) Euclidean weights generator

template<typename T>
class RandomWeights {
private:
    T max_weight;
    T min_weight;
public:
    typedef T weight_type;
    static const bool has_weight = true;
    RandomWeights(): max_weight(0), min_weight(0) {}
    bool is_sane() const {return max_weight > 0;}
    void set_weight_range(T min, T max) {
        max_weight = max;
        min_weight = min;
    }
    T print(int a, int b) const {
        return randrange(min_weight, max_weight);
    }
};

class NoWeights {
public:
    typedef int weight_type;
    static const bool has_weight = false;
    bool is_sane() const {return true;}
    void set_weight_range(int, int) {}
    int print(int, int) const {return -1;}
};

class linear_sample {
private:
    vector<int> sval;
public:
    // we assume -1 < excl[0] < excl[1] < ... < excl[N-1] < max
    linear_sample(int num, size_t max, const vector<int>& excl = vector<int>()) {
        if(max < num + excl.size()) throw too_many_samples();
        sval.resize(num);
        for(int i=0; i<num; i++)
            sval[i] = randrange(0, max - num - excl.size() + 1);
        sort(sval.begin(), sval.end());
        int j = 0;
        for(int i=0; i<num; i++) {
            while(j < (int) excl.size() && excl[j] <= sval[i] + i + j) j++;
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
protected:
    T1 labelGen;
    T2 weightGen;
    btree_set<unsigned>* adjList;
    typedef typename T2::weight_type weight_type;

    void add_random_edges(int M,
                          function<int(int)> naval,
                          function<int(int, int)> encd,
                          function<int(int, int)> dest) {
        vector<int> limits = {0};
        for(size_t i=0; i<labelGen.size(); i++)
            limits.push_back(naval(i) + *limits.rbegin());
        vector<int> excl;
        for(size_t i=0; i<labelGen.size(); i++)
            for(auto d: adjList[i]) {
                int cod = encd(i, d) + limits[i];
                if(cod < limits[i+1])
                    excl.push_back(cod);
            }
        int curn = 0;
        for(auto l: linear_sample(M, *limits.rbegin(), excl)) {
            while(l >= limits[curn+1]) curn++;
            add_edge(curn, dest(curn, l - limits[curn]));
        }
    }

public:
    template<typename... Args>
    Graph(Args... params): labelGen(params...) {
        if(labelGen.size() == 0) throw too_few_nodes();
        adjList = new btree_set<unsigned>[labelGen.size()];
    }

    ~Graph() {
        delete[] adjList;
    }

    virtual void add_edge(int a, int b) = 0;

    void build_forest(int M) {
        if(M > labelGen.size()-1) throw too_many_edges();
        for(auto v: linear_sample(M, labelGen.size()-1)) {
            add_edge(randrange(0, v+1), v+1);
        }
    }

    void build_path() {
        for(size_t i=0; i<labelGen.size()-1; i++)
            add_edge(i, i+1);
    }

    void build_cycle() {
        for(size_t i=0; i<labelGen.size()-1; i++)
            add_edge(i, i+1);
        add_edge(labelGen.size()-1, 0);
    }

    void build_tree() {
        build_forest(labelGen.size()-1);
    }

    void build_star() {
        for(size_t i=1; i<labelGen.size(); i++)
            add_edge(0, i);
    }

    void build_wheel() {
        for(size_t i=1; i<labelGen.size(); i++) {
            add_edge(i-1, i);
            add_edge(0, i);
        }
        add_edge(labelGen.size(), 0);
    }

    void build_clique() {
        for(size_t i=0; i<labelGen.size(); i++)
            for(size_t j=i+1; j<labelGen.size(); j++)
                add_edge(i, j);
    }

    void set_weight_range(weight_type bottom, weight_type top) {
        weightGen.set_weight_range(bottom, top);
    }

    friend ostream& operator<<(ostream& os, const Graph<T1, T2>& g) {
        return os << g.to_string();
    }

    virtual string to_string() const = 0;

    virtual void connect() = 0;

    virtual void add_random_edges(int M) = 0;
};

class DSU {
private:
    int* parent;
    int* rank;
    int N;
public:
    DSU(int N): N(N) {
        parent = new int[N];
        rank = new int[N];
        for(int i=0; i<N; i++) parent[i] = i;
    }

    ~DSU() {
        delete[] parent;
        delete[] rank;
    }

    int find(int a) {
        if(parent[a] == a) return a;
        return parent[a] = find(parent[a]);
    }

    void merge(int a, int b) {
        int va = find(a);
        int vb = find(b);
        if(rank[va] < rank[vb]) swap(va, vb);
        parent[vb] = va;
        rank[va] += rank[vb];
    }
};

template<typename T1, typename T2 = NoWeights>
class UndirectedGraph: public Graph<T1, T2> {
private:
    using Graph<T1, T2>::adjList;
    using Graph<T1, T2>::labelGen;
    using Graph<T1, T2>::weightGen;
    using Graph<T1, T2>::add_random_edges;
public:
    using Graph<T1, T2>::Graph;

    virtual void add_edge(int a, int b) {
        adjList[a].insert(b);
        adjList[b].insert(a);
    }

    virtual string to_string() const {
        if(!weightGen.is_sane()) throw invalid_weightGen();
        int nedg = 0;
        ostringstream oss;
        for(size_t i=0; i<labelGen.size(); i++)
            for(auto oth: adjList[i])
                if(i > oth)
                    nedg++;
        oss << labelGen.size() << " " << nedg << "\n";
        for(size_t i=0; i<labelGen.size(); i++)
            for(auto oth: adjList[i]) {
                if(i <= oth) break;
                oss << labelGen.get(i) << " " << labelGen.get(oth);
                if(T2::has_weight) oss << " " << weightGen.print(i, oth);
                oss << "\n";
            }
        return oss.str();
    }

    virtual void connect() {
        int N = labelGen.size();
        DSU d(N);
        for(int i=0; i<N; i++)
            for(auto x: adjList[i])
                d.merge(i, x);
        // TODO (?): randomize the order of the vertexes
        vector<int> conncomp = {0};
        for(int i=0; i<N; i++)
            if(d.find(0) != d.find(i)) {
                conncomp.push_back(i);
                d.merge(0, i);
            }
        random_shuffle(conncomp.begin(), conncomp.end());
        for(size_t i=1; i<conncomp.size(); i++)
            add_edge(conncomp[randrange(0, i)], conncomp[i]);
    }

    virtual void add_random_edges(int M) {
        add_random_edges(M,
                         [](int n) {return n;},
                         [](int, int dst) {return dst;},
                         [](int, int cod) {return cod;});
    }
};

template<typename T1, typename T2 = NoWeights>
class DirectedGraph: public Graph<T1, T2> {
private:
    using Graph<T1, T2>::adjList;
    using Graph<T1, T2>::labelGen;
    using Graph<T1, T2>::weightGen;
    using Graph<T1, T2>::add_random_edges;
public:
    using Graph<T1, T2>::Graph;

    virtual void add_edge(int a, int b) {
        adjList[a].insert(b);
    }

    virtual string to_string() const {
        if(!weightGen.is_sane()) throw invalid_weightGen();
        int nedg = 0;
        ostringstream oss;
        for(size_t i=0; i<labelGen.size(); i++)
            nedg += adjList[i].size();
        oss << labelGen.size() << " " << nedg << "\n";
        for(size_t i=0; i<labelGen.size(); i++)
            for(auto oth: adjList[i]) {
                oss << labelGen.get(i) << " " << labelGen.get(oth);
                if(T2::has_weight) oss << " " << weightGen.print(i, oth);
                oss << "\n";
            }
        return oss.str();
    }

    virtual void add_random_edges(int M) {
        add_random_edges(M,
                         [&](int n) {return labelGen.size()-1;},
                         [](int n, int dst) {return dst - (dst>n);},
                         [](int n, int cod) {return cod + (cod>=n);});
    }

    virtual void connect() {
        // TODO: implement this. (tarjan?)
        throw not_implemented();
    }
};