#include "graph_gen.h"

int main(){
    UndirectedGraph<IntLabels, RandomWeights<int>> g(100);
    g.build_path();
    g.set_weight_range(1, 20);
    g.print();
}
