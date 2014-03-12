// Copyright 2014, Kento Nikaido
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Kento Nikaido nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "backlinks_compression.h"
#include <stdint.h>
#include <algorithm>
#include <vector>
#include <cstdio>

//
// BitString
//
inline void BitString::Init(uint64_t length) {
  data_.clear();
  data_.resize((length + 63) >> 6, 0);
  length_ = length;
}

inline void BitString::AppendBit(uint64_t val) {
  uint64_t index = length_ >> 6;
  uint64_t bit_pos = length_ & kBITMASK63;
  if (!bit_pos) {
    data_.push_back(val);
  } else {
    data_[index] |= val << bit_pos;
  }
  ++length_;
}

inline void BitString::AppendBitString(const BitString &bitstr) {
  for (uint64_t i = 0; i < bitstr.get_length(); ++i) {
    AppendBit(bitstr.GetBit(i));
  }
}

inline void BitString::SetBit(uint64_t pos, uint64_t val) {
  uint64_t index = pos >> 6;
  uint64_t bit_pos = pos & kBITMASK63;
  data_[index] ^= ((data_[index] >> bit_pos & 1) ^ val) << bit_pos;
}

inline int BitString::GetBit(uint64_t pos) const {
  uint64_t index = pos >> 6;
  uint64_t bit_pos = pos & kBITMASK63;
  return (data_[index] >> bit_pos & 1);
}

inline void BitString::print() {
  for (uint64_t i = 0; i < length_; ++i) std::printf("%d", GetBit(i));
}

//
// DeltaCode
//
inline int DeltaCode::CountBitLength(int val) {
  int count = 0;
  while (val) {
    val >>= 1;
    ++count;
  }
  return count;
}

void DeltaCode::EncodeInt(int val, BitString *out) {
  out->Init(0);
  int length_val = CountBitLength(val + 1) - 1;
  int length_length_val = CountBitLength(length_val + 1) - 1;
  for (int i = 0; i < length_length_val; ++i) out->AppendBit(0);
  out->AppendBit(1);
  for (int i = length_length_val - 1; i >= 0; --i) {
    out->AppendBit((length_val + 1) >> i & 1);
  }
  for (int i = length_val - 1; i >= 0; --i) {
    out->AppendBit((val + 1) >> i & 1);
  }
}

void DeltaCode::Encode(const std::vector<int> &in, BitString *out) {
  out->Init(0);
  BitString tmp;
  for (uint64_t i = 0; i < in.size(); ++i) {
    EncodeInt(in[i], &tmp);
    out->AppendBitString(tmp);
  }
}

int DeltaCode::DecodeNextInt(const BitString &in, uint64_t *i) {
    int length_length_val = 0;
    while (!in.GetBit(*i)){
      ++length_length_val;
      ++(*i);
    }
    ++(*i);
    int length_val = 1;
    for(int j = 0; j < length_length_val; ++j) {
      length_val = (length_val << 1) | in.GetBit(*i);
      ++(*i);
    }
    --length_val;
    int val = 1;
    for(int j = 0; j < length_val; ++j) {
      val = (val << 1) | in.GetBit(*i);
      ++(*i);
    }
    return val - 1;
}

int DeltaCode::DecodeInt(const BitString &in) {
  uint64_t i = 0;
  return DecodeNextInt(in, &i);
}

void DeltaCode::Decode(const BitString &in, std::vector<int> *out) {
  out->clear();
  uint64_t i = 0;
  while (i < in.get_length()) {
    out->push_back(DecodeNextInt(in, &i));
  }
}

//
// BacklinksCompression
//
void BacklinksCompression
::TransformToAdj(const std::vector<std::pair<int, int> > &edges,
                 std::vector<std::vector<int> > *adj) {
  int num_v = 0;
  for (uint64_t i = 0; i < edges.size(); ++i) {
    num_v = std::max(num_v, std::max(edges[i].first, edges[i].second) + 1);
  }
  adj->resize(num_v);
  for (uint64_t i = 0; i < edges.size(); ++i) {
    adj->at(edges[i].first).push_back(edges[i].second);
  }
  for (int i = 0; i < num_v; ++i) {
    std::sort(adj->at(i).begin(), adj->at(i).end());
  }
}

void BacklinksCompression
::TransformToEdge(const std::vector<std::vector<int> > &adj,
                   std::vector<std::pair<int, int> > *edges) {
  for (int i = 0; i < adj.size(); ++i) {
    for (int j = 0; j < adj[i].size(); ++j) {
      edges->push_back(std::make_pair(i, adj[i][j]));
    }
  }
}

void BacklinksCompression
::Compress(const std::vector<std::pair<int, int> > &edges, BitString *result) {
  std::vector<std::vector<int> > adj;
  TransformToAdj(edges, &adj);
  int num_v = adj.size();
  CompressVertexes(adj, num_v, result);
}

void BacklinksCompression
::Develop(const BitString &code, std::vector<std::pair<int, int> > *edges) {
  DeltaCode delta;
  uint64_t cur = 0;
  int num_v = delta.DecodeNextInt(code, &cur);
  std::vector<std::vector<int> > adj(num_v);
  
  for (int i = 0; i < num_v; ++i) {
    int j = i - delta.DecodeNextInt(code, &cur);
    if (code.GetBit(cur++)) adj[i].push_back(i);
    
    // copying
    if (i != j) {
      for (int k = 0; k < adj[j].size(); ++k) {
        if (Exist(adj, adj[j][k], j) && j >= adj[j][k]) continue;
        if (code.GetBit(cur++)) {
          adj[i].push_back(adj[j][k]);
        }
      }
    }
    
    // residual
    int num_residual = delta.DecodeNextInt(code, &cur);
    if (num_residual != 0) {
      int sign = (code.GetBit(cur++) ? 1 : -1);
      int now = i + delta.DecodeNextInt(code, &cur) * sign;
      adj[i].push_back(now);
      for (int j = 1; j < num_residual; ++j) {
        now += delta.DecodeNextInt(code, &cur);
        adj[i].push_back(now);
      }
    }
    
    sort(adj[i].begin(), adj[i].end());
    
    // reciprocal
    for (int k = 0; k < adj[i].size(); ++k) {
      if (i >= adj[i][k]) continue;
      if (code.GetBit(cur++)) {
        adj[adj[i][k]].push_back(i);
      }
    }
  }
  
  TransformToEdge(adj, edges);
}

// followings are private
inline bool BacklinksCompression
::Exist(const std::vector<std::vector<int> > &adj, int from, int to) {
  return std::binary_search(adj[from].begin(), adj[from].end(), to);
}

inline void BacklinksCompression
::ProceedCopying(const std::vector<std::vector<int> > &adj, int val,
                 int now, int *cur, std::vector<int> *residual) {
  while ((*cur) < adj[now].size() && adj[now][*cur] < val) {
    if (!Exist(adj, adj[now][*cur], now) || now < adj[now][*cur]) {
      residual->push_back(adj[now][*cur]);
    }
    ++(*cur);
  }
}

void BacklinksCompression
::CompressVertexes(const std::vector<std::vector<int> > &adj,
                   int num_v, BitString *result) {
  DeltaCode delta;
  BitString tmp;
  delta.EncodeInt(num_v, &tmp);
  result->AppendBitString(tmp);
  
  for (int i = 0; i < num_v; ++i) {
    BitString best;
    best.Init(0);
    for (int j = i; j > i - kWINDOW_WIDTH && j >= 0; --j) {
      BitString now;
      now.Init(0);
      delta.EncodeInt(i - j, &tmp);
      now.AppendBitString(tmp);
      if (Exist(adj, i, i)) { // self-loop
        now.AppendBit(1);
      } else {
        now.AppendBit(0);
      }
      std::vector<int> residual;
      
      // copying
      int cur = 0;
      if (i != j) {
        for (int k = 0; k < adj[j].size(); ++k) {
          if (Exist(adj, adj[j][k], j) && j >= adj[j][k]) continue;
          ProceedCopying(adj, adj[j][k], i, &cur, &residual);
          if (cur < adj[i].size() && adj[i][cur] == adj[j][k]) {
            now.AppendBit(1);
          } else {
            now.AppendBit(0);
          }
        }
      }
      ProceedCopying(adj, num_v, i, &cur, &residual);
      
      // residual
      delta.EncodeInt(residual.size(), &tmp);
      now.AppendBitString(tmp);
      if (residual.size() != 0) {
        if (residual[0] > i) {
          now.AppendBit(1);
        } else {
          now.AppendBit(0);
        }
        delta.EncodeInt(std::abs(residual[0] - i), &tmp);
        now.AppendBitString(tmp);
        for (int k = 1; k < residual.size(); ++k) {
          delta.EncodeInt(residual[k] - residual[k - 1], &tmp);
          now.AppendBitString(tmp);
        }
      }
      
      // reciprocal
      for (int k = 0; k < adj[i].size(); ++k) {
        if (i >= adj[i][k]) continue;
        if (Exist(adj, adj[i][k], i)) {
          now.AppendBit(1);
        } else {
          now.AppendBit(0);
        }
      }
      
      // update best
      if (i == j || best.get_length() > now.get_length()) best = now;
    }
    
    result->AppendBitString(best);
  }
}
