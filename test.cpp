#include "graph_gen.h"

int main(){
    Graph<IntLabels, RandomWeights<int>> g(100);
    g.build_forest(95);
    g.set_weight_range(1, 20);
    g.print_undirected();
}
