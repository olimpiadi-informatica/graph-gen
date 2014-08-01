#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

class IntLabels {
private:
    vector<int> labels;
public:
    IntLabels(int num, int start) {
        labels.resize(num);
        for(int i=0; i<num; i++)
            labels[i] = i+start;
    }
    IntLabels(int num) {
        IntLabels(num, 0);
    }
    inline unsigned size() {
        return labels.size();
    }
};

class RandomIntLabels {
private:
    vector<int> labels;
public:
    RandomIntLabels(int num, int start) {
        labels.resize(num);
        for(int i=0; i<num; i++)
            labels[i] = i+start;
        random_shuffle(labels.begin(), labels.end());
    }
    RandomIntLabels(int num) {
        IntLabels(num, 0);
    }
    inline unsigned size() {
        return labels.size();
    }
};

template<typename T>
class VectorLabels {
private:
    vector<T>& vec;
public:
    VectorLabels(vector<T>& v): vec(v) {}
    inline unsigned size() {
        return vec.size();
    }
};

template<typename T>
class Graph {
private:
    T labelGen;
public:
    template<typename... Args>
    Graph(Args... params): labelGen(params...) {}
};
