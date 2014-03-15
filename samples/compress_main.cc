#include <cstdlib>
#include <iostream>
#include <fstream>
#include "backlinks_compression.h"

using namespace std;

bool Input(const char *filename, vector<pair<int, int> > *edges);

int main(int argc, char **argv) {
  if (argc != 3) {
    cerr << "usage: compress GRAPH OUTPUT" << endl;
    exit(EXIT_FAILURE);
  }
  
  vector<pair<int, int> > edges;
  if (!Input(argv[1], &edges)) {
    cerr << "error: Load failed" << endl;
    exit(EXIT_FAILURE);
  }
  
  BacklinksCompression bl;
  BitString result;
  bl.Compress(edges, &result);
  
  if (!result.Output(argv[2])) {
    cerr << "error: Output failed" << endl;
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}

bool Input(const char *filename, vector<pair<int, int> > *edges) {
  ifstream ifs(filename);
  if (!ifs) return false;
  for (int v, w; ifs >> v >> w; ) edges->push_back(make_pair(v, w));
  return !ifs.bad();
}
