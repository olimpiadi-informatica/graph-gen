#include "graph_gen.h"

int main(){
    Graph<IntLabels> g(100);
    g.build_forest(95);
    g.print_undirected();
}
