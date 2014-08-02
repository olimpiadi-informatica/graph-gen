#include "graph_gen.h"

int main(){
    UndirectedGraph<RandomIntLabels> g(100000);
    g.add_random_edges(100000);
    g.connect();
    cout << g << endl;
}
