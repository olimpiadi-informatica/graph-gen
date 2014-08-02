#include "graph_gen.h"

int main(){
    UndirectedGraph<IntLabels, RandomWeights<int>> g(100);
    g.build_forest(95);
    g.connect();
    g.set_weight_range(1, 20);
    g.print();
}
