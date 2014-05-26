Backlinks compression
========================

Backlinks compression is a graph compression scheme for real-world graphs.

### From CUI Interface

    $ make
    $ bin/compress samples/graph_example.tsv output_file.dat
    $ bin/develop input_file.dat output_file.tsv

* Execute `make` to build programs.
* Execute `bin/compress` to compress a graph.
* Execute `bin/develop` to develop a compressed graph.

### From Your Program

    BacklinksCompression bl;
    std::vector<std::pair<int, int> > edge_list;
    BitString compressed_graph;
    
    bl.Compress(edge_list, &compressed_graph);
    bl.Develop(compressed_graph, &edge_list);

* Call `Compress` to compress a graph.
* Call `Develop` to develop a compressed graph.
* Call `TransformToAdj` and `TransformToEdge` to transform a graph.

#Performance

Backlinks compression scheme has two steps: ordering vertexes and encoding the graph.
This implementation uses BFS ordering for ordering vertexes and δ-code as the integer encoding scheme in encoding the graph.

The followings are the results for real graphs in [Stanford Large Network Dataset Collection](http://snap.stanford.edu/data/).

    graph : soc-Slashdot0922
    edges      : 948464
    bit length : 9238812
    bits/edge  : 9.74081
    
    graph : wiki-Vote
    edges      : 26120
    bit length : 259175
    bits/edge  : 9.92247

#References

* Flavio Chierichetti, Ravi Kumar, Silvio Lattanzi, Michael
Mitzenmacher, Alessandro Panconesi, and Prabhakar Raghavan. On
compressing social networks. KDD'09.

* Paolo Boldi, Marco Rosa, Massimo Santini, and Sebastiano Vigna.
Layered label propagation: a multiresolution coordinate-free ordering
for compressing social networks. WWW'11.
