#include <gtest/gtest.h>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include "backlinks_compression.h"
using namespace std;

TEST(TestCase1, DeltaCode) {
  std::vector<int> original, result;
  BitString code;
  srand((unsigned)time(NULL));
  for (int i = 0; i < 10000; ++i)
    original.push_back(i*i);
  DeltaCode delta;
  delta.Encode(original, &code);
  delta.Decode(code, &result);
	EXPECT_EQ(result, original);
}

TEST(Random, Compression) {
  srand(2014);
  vector<pair<int, int> > original, result;
  BitString code;
  
  int vs = 200, es = 1000;
  vector<pair<int, int> > box;
  for(int i = 0; i < vs; i++)for(int j = 0; j < vs; j++){
    box.push_back(make_pair(i, j));
  }
  random_shuffle(box.begin(),box.end());
  for(uint64_t i = 0; i < es; i++) original.push_back(box[i]);
  sort(original.begin(), original.end());
  BacklinksCompression BL;
  BL.Compress(original, &code);
	//code.print();
	BL.Develop(code, &result);
	EXPECT_EQ(result, original);
}

