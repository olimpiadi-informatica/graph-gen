#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>
#include <functional>
#include <sstream>
#include <numeric>
#include "cpp-btree/btree_set.h"

typedef size_t vertex_t;

class TooManyEdgesException: public std::exception
{
	virtual const char* what() const noexcept
	{
		return "You specified too many edges!";
	}
};

class TooFewEdgesException: public std::exception
{
	virtual const char* what() const noexcept
	{
		return "You specified too few edges!";
	}
};

class TooFewNodesException: public std::exception
{
	virtual const char* what() const noexcept
	{
		return "You specified too few nodes!";
	}
};

class TooManySamplesException: public std::exception
{
	virtual const char* what() const noexcept
	{
		return "You specified too many values to sample from the given range!";
	}
};

class InvalidWeightGenException: public std::exception
{
	virtual const char* what() const noexcept
	{
		return "You didn't call Graph.set_max_weight()!";
	}
};

class NotImplementedException: public std::exception
{
	virtual const char* what() const noexcept
	{
		return "This function is not implemented yet!";
	}
};

template<typename T1, typename T2>
auto randrange(T1 bottom, T2 top)
-> typename
std::enable_if<
  	!std::is_integral< decltype(bottom+top) >::value,
	decltype(bottom+top)
>::type
{
	return double(rand())/RAND_MAX * (top - bottom) + bottom;
}

template<typename T1, typename T2>
auto randrange(T1 bottom, T2 top)
-> typename
std::enable_if<
	std::is_integral< decltype(bottom+top) >::value,
	decltype(bottom+top)
>::type
{
	return rand() % (top - bottom) + bottom;
}

/**
Labeler is the abstract class that defines the interface for a
graph labeler, i.e. a class that assigns labels to vertices.
*/
template<typename T>
class Labeler
{
public:
	typedef T label_t;

	virtual ~Labeler() {}

	/**
	label takes as argument the index of the node, and
	returns a label of type T for it. It must be a deterministic
	injective function.
	*/
	virtual T label(const vertex_t i) = 0;

};

/**
IotaLabeler is the simplest labeler. The label of the i-th vetex
is simply the integer (i+start).
*/
class IotaLabeler: public virtual Labeler<int>
{
private:
	int start;

public:
	IotaLabeler(int start = 0): start(start) {}
	~IotaLabeler() {}

	int label(const vertex_t i) override
	{
		return start + i;
	}
};

/**
RandIntLabeler assigns random labels from a given range
*/
class RandIntLabeler: public virtual Labeler<int>
{
private:
	std::vector<int> labels;

public:
	/**
	Define the sample range [start, end]
	*/
	RandIntLabeler(int start, int end)
	{
		labels.resize(end - start + 1);
		std::iota(labels.begin(), labels.end(), start);
		std::random_shuffle(labels.begin(), labels.end());
	}
	~RandIntLabeler() {}

	int label(const vertex_t i)
	{
		return labels.at(i);
	}
};

/**
StaticLabeler assigns labels from a given vector
*/
template<typename T>
class StaticLabeler: public Labeler<T>
{
private:
	std::vector<T>& labels;

public:
	StaticLabeler(const std::vector<T>& labels): labels(labels) {}
	~StaticLabeler() {}

	T label(const vertex_t i)
	{
		return labels.at(i);
	}
};

/**
Weighter is the abstract class that defines the interface for a
graph weighter, i.e. a class that assigns weights to edges.
*/
template<typename T>
class Weighter
{
public:
	typedef T weight_t;

	virtual ~Weighter() {}

	/**
	weight takes as arguments two vertex_t, corresponding to the tail and
	the head vertices of the edge of interest
	, and returns a weight
	of type T for the edge. It must be a deterministic function.
	*/
	virtual T weight(const vertex_t tail, const vertex_t head) = 0;
};

// TODO (?) Euclidean weights generator

/**
RandomWeighter is the simplest weighter. It returns random weights taken from
a given range of values
*/
template<typename T>
class RandomWeighter: public Weighter<T>
{
private:
	T min, max; // Define the range

public:
	RandomWeighter(T min, T max): min(min), max(max) {};
	~RandomWeighter() {};

	T weight(const vertex_t tail, const vertex_t head)
	{
		return randrange(min, max);
	}
};

/**
NoWeighter is a dummy weighter. It throws if called
*/
class NoWeighter: public Weighter<void>
{
public:
	~NoWeighter() {};

	void weight(const vertex_t, const vertex_t)
	{
		// TODO: Define a proper exception
		throw NotImplementedException();
	}
};

/** 
RangeSampler provides iterators for ranging over sampled integers
in a given range.
*/
class RangeSampler
{
private:
	std::vector<int> samples;

public:
	/**
	The constructor generates the samples from the range [min, max].

	@param sample_size the number of samples
	@param min the min of the range
	@param max the max of the range
	@param excl an optional vector of undesired values
	*/
	RangeSampler(
		const size_t sample_size,
		const int min,
		const int max,
		std::vector<int> excl = std::vector<int>()
	) {
		if (!std::is_sorted(excl.begin(), excl.end()))
			std::sort(excl.begin(), excl.end());

		// If the user requests more samples than the dimension of the range, throw
		if (max - min + 1 < int(sample_size + excl.size()))
			throw TooManySamplesException();
		
		samples.resize(sample_size);
		for (size_t i = 0; i < sample_size; i++)
			samples[i] = randrange(min, max - sample_size - excl.size() + 1);

		// TODO: Is counting sort better than std::sort here?
		std::sort(samples.begin(), samples.end());
		size_t excl_idx = 0;
		for(size_t i = 0; i < sample_size; i++)
		{
			while(excl_idx < excl.size() && excl[excl_idx] <= samples[i] + int(i + excl_idx))
				excl_idx++;
			samples[i] += i + excl_idx;
		}
	}

	std::vector<int>::iterator begin()
	{
		return samples.begin();
	}

	std::vector<int>::iterator end()
	{
		return samples.end();
	}
};

/**
Graph is an abstract class
*/
template<typename label_t, typename weight_t = void>
class Graph {
protected:
	size_t vertices_no; // Number of vertices
	Labeler<label_t>& labeler;
	Weighter<weight_t>& weighter;

	btree::btree_set<unsigned>* adj_list;

	/**
	Creates random edges

	@param edges_no number of edges to be created
	@param max_outdegree this lambda specifies the maximum (out)degree for the vertex passed as argument
	@param neighbour_to_rank this lambda takes two vertices and returns the rank of the second vertex among the neighbours of the first one
	@param rank_to_neighbour this lambda takes a vertex and a rank, and returns, among the neighbours of the given vertex, the one whose rank is the given rank
	*/
	void add_random_edges(
		const size_t edges_no,
		const std::function< size_t(const vertex_t) > max_outdegree,
		const std::function< size_t(const vertex_t, const vertex_t) > neighbour_to_rank,
		const std::function< vertex_t(const vertex_t, const size_t) > rank_to_neighbour
	) {
		// Use RangeSampler to pick edges_no random edges. To do that, we first
		// "lay out" all the possible edges, giving them indices 0, 1, ... 
		//
		// The range [edge_range[i] .. edge_range[i+1]) is exactly
		// the range of edges going out from vertex i. Basically, we are just
		// computing the partial sums of max_outdegree.
		std::vector<size_t> edge_range(vertices_no + 1, 0);
		for (vertex_t i = 0; i < vertices_no; i++)
			edge_range[i+1] = edge_range[i] + max_outdegree(i);

		// We now remove the existing edges from the range of edges that
		// RangeSampler will choose from.
		std::vector<int> excluded_edges;
		for (vertex_t v = 0; v < vertices_no; v++)
		{
			for (vertex_t neighbour: adj_list[v])
			{
				size_t edge_rank = neighbour_to_rank(v, neighbour) + edge_range[v];
				if (edge_rank < edge_range[v+1])
					excluded_edges.push_back(edge_rank);
			}
		}

		// We now call RangeSampler and scan the sorted samples, adding edges as we go
		vertex_t current_vertex = 0;
		for (size_t edge_rank:
			RangeSampler(
				edges_no,
				0,
				edge_range[vertices_no],
				excluded_edges
		)) {
			while (edge_rank >= edge_range[current_vertex + 1])
				current_vertex++;
			add_edge(
				current_vertex,
				rank_to_neighbour(current_vertex, edge_rank - edge_range[current_vertex])
			);
		}
	}

public:
	/**
	Initialize the graph

	@param vertices_no number of vertices of the graph
	@param edges_no number of edges of the graph
	*/
	Graph(const size_t vertices_no, Labeler<label_t>& labeler, Weighter<weight_t>& weighter):
		vertices_no(vertices_no), labeler(labeler), weighter(weighter)
	{
		adj_list = new btree::btree_set<unsigned>[vertices_no];
	}

	~Graph() {
		delete[] adj_list;
	}

	// Interface methods
	virtual void add_edge(const vertex_t a, const vertex_t b) = 0;
	virtual std::string to_string() const = 0;
	virtual void connect() = 0;
	virtual void add_random_edges(const size_t edges_t) = 0;

	void build_forest(size_t edges_no)
	{
		if (edges_no > vertices_no - 1)
			throw TooManyEdgesException();
		for(vertex_t v: RangeSampler(edges_no, 0, vertices_no-1))
			add_edge(randrange(0, v+1), v+1);
	}

	void build_path()
	{
		for(vertex_t i = 0; i < vertices_no - 1; i++)
			add_edge(i, i+1);
	}

	void build_cycle()
	{
		for(vertex_t i = 0; i < vertices_no - 1; i++)
			add_edge(i, i+1);
		add_edge(vertices_no - 1, 0);
	}

	void build_tree()
	{
		build_forest(vertices_no - 1);
	}

	void build_star()
	{
		for(vertex_t i=1; i<vertices_no; i++)
			add_edge(0, i);
	}

	void build_wheel() {
		for(vertex_t i=1; i<vertices_no; i++) {
			add_edge(i-1, i);
			add_edge(0, i);
		}
		add_edge(vertices_no, 0);
	}

	void build_clique()
	{
		for(vertex_t i=0; i<vertices_no; i++)
			for(vertex_t j=i+1; j<vertices_no; j++)
				add_edge(i, j);
	}
 
	friend std::ostream& operator<<(std::ostream& os, const Graph<label_t, weight_t>& g)
	{
		return os << g.to_string();
	}
};

/**
Disjoint set data structure
*/
class DisjointSet
{
private:
	size_t* parent;
	size_t* rank;
	size_t N;

public:
	DisjointSet(const size_t N): N(N)
	{
		parent = new size_t[N];
		rank = new size_t[N];
		for (size_t i=0; i<N; i++)
			parent[i] = i, rank[i] = 1;
	}

	~DisjointSet()
	{
		delete[] parent;
		delete[] rank;
	}

	size_t size() const
	{
		return N;
	}

	size_t find(const size_t a)
	{
		if (parent[a] == a) return a;
		return parent[a] = find(parent[a]);
	}

	void merge(const size_t a, size_t b)
	{
		int va = find(a);
		int vb = find(b);
		if (rank[va] < rank[vb])
			std::swap(va, vb);
		parent[vb] = va;
		rank[va] += rank[vb];
	}
};


template<typename label_t, typename weight_t = void>
class UndirectedGraph: public Graph<label_t, weight_t>
{
private:
	using Graph<label_t, weight_t>::adj_list;
	using Graph<label_t, weight_t>::labeler;
	using Graph<label_t, weight_t>::weighter;
	using Graph<label_t, weight_t>::add_random_edges;
	using Graph<label_t, weight_t>::vertices_no;

	// For unweighted graphs
	std::string to_string(std::true_type) const
	{
		size_t edges_no = 0;
		std::ostringstream oss;
		for (vertex_t v = 0; v < vertices_no; v++)
			for (vertex_t neighbour: adj_list[v])
				if (v > neighbour)
					edges_no++;

		oss << vertices_no << " " << edges_no << "\n";
		for (vertex_t v = 0; v < vertices_no; v++)
		{
			for (vertex_t neighbour: adj_list[v])
			{
				if (v <= neighbour)
					break;
				oss << labeler.label(v) << " " << labeler.label(neighbour);
				oss << "\n";
			}
		}
		return oss.str();
	}

	// For weighted graphs
	std::string to_string(std::false_type) const
	{
		size_t edges_no = 0;
		std::ostringstream oss;
		for (vertex_t v = 0; v < vertices_no; v++)
			for (vertex_t neighbour: adj_list[v])
				if (v > neighbour)
					edges_no++;

		oss << vertices_no << " " << edges_no << "\n";
		for (vertex_t v = 0; v < vertices_no; v++)
		{
			for (vertex_t neighbour: adj_list[v])
			{
				if (v <= neighbour)
					break;
				oss << labeler.label(v) << " " << labeler.label(neighbour);
				oss << " " << weighter.weight(v, neighbour);
				oss << "\n";
			}
		}
		return oss.str();
	}

public:
	using Graph<label_t, weight_t>::Graph;

	void add_edge(const vertex_t tail, const vertex_t head) override
	{
		adj_list[tail].insert(head);
		adj_list[head].insert(tail);
	}

	std::string to_string() const override
	{
		return to_string(std::is_same<weight_t, void>());
	}

	void connect() override {
		DisjointSet connected_components(vertices_no);
		for (vertex_t v = 0; v < vertices_no; v++)
			for (vertex_t neighbour: adj_list[v])
				connected_components.merge(v, neighbour);

		// We are going to scan through the vertices in random order
		std::vector<size_t> shuffled_vertices(vertices_no);
		std::iota(shuffled_vertices.begin(), shuffled_vertices.end(), vertex_t(0));
		std::random_shuffle(shuffled_vertices.begin(), shuffled_vertices.end());

		// representatives contains K representative vertices, with K the number
		// of connected components in the graph. A representative is a randomly chosen
		// vertex among the vertices forming its connected component
		std::vector<vertex_t> representatives = { shuffled_vertices[0] };
		for(size_t i = 1; i < vertices_no; i++) {
			if (connected_components.find(shuffled_vertices[0]) != connected_components.find(shuffled_vertices[i]))
			{
				representatives.push_back(shuffled_vertices[i]);
				connected_components.merge(shuffled_vertices[0], shuffled_vertices[i]);
			}
		}

		// Build a random tree spanning the representative vertices
		for (size_t i = 1; i < representatives.size(); i++)
			add_edge(representatives[randrange(0, i)], representatives[i]);
	}

	void add_random_edges(const size_t edges_no)
	{
		add_random_edges(
			edges_no,
			[](vertex_t v)              -> size_t   { return v; },     // max_outdegree
			[](vertex_t, vertex_t head) -> size_t   { return head; },  // neighbour_to_rank
			[](vertex_t, size_t rank)   -> vertex_t { return rank; }   // rank_to_neighbour
		);
	}
};

template<typename label_t, typename weight_t = void>
class DirectedGraph: public Graph<label_t, weight_t>
{
private:
	using Graph<label_t, weight_t>::adj_list;
	using Graph<label_t, weight_t>::labeler;
	using Graph<label_t, weight_t>::weighter;
	using Graph<label_t, weight_t>::add_random_edges;
	using Graph<label_t, weight_t>::vertices_no;

	// For unweighted graphs
	std::string to_string(std::true_type) const
	{
		size_t edges_no = 0;
		std::ostringstream oss;
		for (vertex_t v = 0; v < vertices_no; v++)
			for (vertex_t neighbour: adj_list[v])
				if (v > neighbour)
					edges_no++;

		oss << vertices_no << " " << edges_no << "\n";
		for (vertex_t v = 0; v < vertices_no; v++)
		{
			for (vertex_t neighbour: adj_list[v])
			{
				oss << labeler.label(v) << " " << labeler.label(neighbour);
				oss << "\n";
			}
		}
		return oss.str();
	}

	// For weighted graphs
	std::string to_string(std::false_type) const
	{
		size_t edges_no = 0;
		std::ostringstream oss;
		for (vertex_t v = 0; v < vertices_no; v++)
			for (vertex_t neighbour: adj_list[v])
				if (v > neighbour)
					edges_no++;

		oss << vertices_no << " " << edges_no << "\n";
		for (vertex_t v = 0; v < vertices_no; v++)
		{
			for (vertex_t neighbour: adj_list[v])
			{
				oss << labeler.label(v) << " " << labeler.label(neighbour);
				oss << " " << weighter.weight(v, neighbour);
				oss << "\n";
			}
		}
		return oss.str();
	}

public:
	using Graph<label_t, weight_t>::Graph;


	void add_edge(const vertex_t tail, const vertex_t head) override
	{
		adj_list[tail].insert(head);
	}

	std::string to_string() const override
	{
		return to_string(std::is_same<weight_t, void>());
	}

	void add_random_edges(const size_t edges_no)
	{
		add_random_edges(
			edges_no,
			[&](vertex_t v) { return vertices_no - 1; },                        // max_outdegree
			[](vertex_t tail, vertex_t head) { return head - (head > tail); },  // neighbour_to_rank
			[](vertex_t v, size_t rank) { return rank + (rank >= v); }          // rank_to_neighbour
		);
	}

	/**
	Add the minimum number of edges so that the resulting digraph is
	STRONGLY connected
	*/
	virtual void connect()
	{
		// TODO: implement this. (tarjan?)
		throw NotImplementedException();
	}
};
