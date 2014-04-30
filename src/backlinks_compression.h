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

#ifndef BACKLINKS_COMPRESSION_H_
#define BACKLINKS_COMPRESSION_H_

#include <stdint.h>
#include <algorithm>
#include <vector>
#include <cstdio>

const int kWINDOW_WIDTH = 10;
enum Ordering { BFS, NONE };
const Ordering kORDERING = BFS;

//
// BitString
//
class BitString {
 public:
  BitString() : data_(0), length_(0) {}
  inline void Init(uint64_t length);
  inline void AppendBit(uint64_t val);
  inline void AppendBitString(const BitString &bitstr);
  inline void SetBit(uint64_t pos, uint64_t val);
  inline int GetBit(uint64_t pos) const;
  inline uint64_t get_length() const { return length_;}
  inline void Print() const;
  bool Input(const char *filename);
  bool Output(const char *filename);
 private:
  static const int kBITMASK63 = (1 << 6) - 1;
  std::vector<uint64_t> data_;
  uint64_t length_;
};

//
// delta-code (integer encoding)
//
class DeltaCode {
 public:
  inline int CountBitLength(int val);
  void EncodeInt(int val, BitString *out);
  inline void Encode(const std::vector<int> &in, BitString *out);
  int DecodeNextInt(const BitString &in, uint64_t *i);
  inline int DecodeInt(const BitString &in);
  inline void Decode(const BitString &in, std::vector<int> *out);
};

//
// BacklinksCompression
//
class BacklinksCompression {
 public:
  void TransformToAdj(const std::vector<std::pair<int, int> > &edges,
                      bool directed, std::vector<std::vector<int> > *adj);
  void TransformToEdge(const std::vector<std::vector<int> > &adj,
                        std::vector<std::pair<int, int> > *edges);
  void Compress(std::vector<std::pair<int, int> > edges,
                BitString *result);
  void Develop(const BitString &code,
               std::vector<std::pair<int, int> > *edges);
 private:
  void BFSOrder(const std::vector<std::vector<int> > &adj,
                std::vector<int> *order);
  void Order(const std::vector<std::pair<int, int> > &edges,
             std::vector<int> *order);
  inline bool Exist(const std::vector<std::vector<int> > &adj,
                    int from, int to);
  inline void ProceedCopying(const std::vector<std::vector<int> > &adj, int val,
                             int now, int *cur, std::vector<int> *residual);
  void CompressVertexes(const std::vector<std::vector<int> > &adj,
                        BitString *result);
};


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

inline void BitString::Print() const {
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

inline void DeltaCode::Encode(const std::vector<int> &in, BitString *out) {
  out->Init(0);
  BitString tmp;
  for (uint64_t i = 0; i < in.size(); ++i) {
    EncodeInt(in[i], &tmp);
    out->AppendBitString(tmp);
  }
}

inline int DeltaCode::DecodeInt(const BitString &in) {
  uint64_t i = 0;
  return DecodeNextInt(in, &i);
}

inline void DeltaCode::Decode(const BitString &in, std::vector<int> *out) {
  out->clear();
  uint64_t i = 0;
  while (i < in.get_length()) {
    out->push_back(DecodeNextInt(in, &i));
  }
}

//
// BacklinksCompression
//
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

#endif  // BACKLINKS_COMPRESSION_H_
