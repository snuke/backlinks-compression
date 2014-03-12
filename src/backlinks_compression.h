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
#include <vector>

const int kWINDOW_WIDTH = 10;

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
  inline void print();
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
  void Encode(const std::vector<int> &in, BitString *out);
  int DecodeNextInt(const BitString &in, uint64_t *i);
  int DecodeInt(const BitString &in);
  void Decode(const BitString &in, std::vector<int> *out);
};

//
// BacklinksCompression
//
class BacklinksCompression {
 public:
  void TransformToAdj(const std::vector<std::pair<int, int> > &edges,
                      std::vector<std::vector<int> > *adj);
  void TransformToEdge(const std::vector<std::vector<int> > &adj,
                        std::vector<std::pair<int, int> > *edges);
  void Compress(const std::vector<std::pair<int, int> > &edges,
                BitString *result);
  void Develop(const BitString &code,
               std::vector<std::pair<int, int> > *edges);
 private:
  inline bool Exist(const std::vector<std::vector<int> > &adj,
                    int from, int to);
  inline void ProceedCopying(const std::vector<std::vector<int> > &adj, int val,
                             int now, int *cur, std::vector<int> *residual);
  void CompressVertexes(const std::vector<std::vector<int> > &adj,
                        int num_v, BitString *result);
};

#endif  // BACKLINKS_COMPRESSION_H_
