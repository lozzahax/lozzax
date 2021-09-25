// Copyright (c) 2018-2021, The Loki Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <array>

#include "hardfork.h"

namespace cryptonote {

// version 7 from the start of the blockchain, inhereted from Monero mainnet
static constexpr std::array mainnet_hard_forks =
{
  hard_fork{7,    0,       0, 1503046577 },
  hard_fork{8,    0,      10, 1632426903 },
  hard_fork{9,    0,     120, 1632428703 },
  hard_fork{10,   0,     140, 1632428943 },
  hard_fork{11,   0,     150, 1632429243 },
  hard_fork{12,   0,     160, 1632429363 },
  hard_fork{13,   0,     170, 1632429603 },
  hard_fork{14,   0,     180, 1632429843 },
  hard_fork{15,   0,     200, 1632430203 },
  hard_fork{16,   0,     250, 1632430683 },
  hard_fork{17,   0,     300, 1632431103 },
  hard_fork{18,   1,     500, 1632432603 },
};

static constexpr std::array testnet_hard_forks =
{
  hard_fork{7,   0,      0, 1503046577 },
  hard_fork{8,   0,   1000, 1632041429 },
  hard_fork{13,  0,   1200, 1632045029 },
  hard_fork{15,  0,   1300, 1632048629 },
  hard_fork{18,  1,   1400, 1632048629 },
};

static constexpr std::array devnet_hard_forks =
{
  hard_fork{7,   0,      0, 1503046577 },
  hard_fork{8,   0,   1000, 1632041429 },
  hard_fork{13,  0,   1200, 1632045029 },
  hard_fork{15,  0,   1300, 1632048629 },
  hard_fork{18,  1,   1400, 1632048629 },
};


template <size_t N>
static constexpr bool is_ordered(const std::array<hard_fork, N>& forks) {
  if (N == 0 || forks[0].version < 7)
    return false;
  for (size_t i = 1; i < N; i++) {
    auto& hf = forks[i];
    auto& prev = forks[i-1];
    if ( // [major,snoderevision] pair must be strictly increasing (lexicographically)
        std::make_pair(hf.version, hf.snode_revision) <= std::make_pair(prev.version, prev.snode_revision)
        // height must be strictly increasing; time must be weakly increasing
        || hf.height <= prev.height || hf.time < prev.time)
      return false;
  }
  return true;
}

static_assert(is_ordered(mainnet_hard_forks),
    "Invalid mainnet hard forks: version must start at 7, major versions and heights must be strictly increasing, and timestamps must be non-decreasing");
static_assert(is_ordered(testnet_hard_forks),
    "Invalid testnet hard forks: version must start at 7, versions and heights must be strictly increasing, and timestamps must be non-decreasing");
static_assert(is_ordered(devnet_hard_forks),
    "Invalid devnet hard forks: version must start at 7, versions and heights must be strictly increasing, and timestamps must be non-decreasing");

std::vector<hard_fork> fakechain_hardforks;

std::pair<const hard_fork*, const hard_fork*> get_hard_forks(network_type type)
{
  if (type == network_type::MAINNET) return {&mainnet_hard_forks[0], &mainnet_hard_forks[mainnet_hard_forks.size()]};
  if (type == network_type::TESTNET) return {&testnet_hard_forks[0], &testnet_hard_forks[testnet_hard_forks.size()]};
  if (type == network_type::DEVNET) return {&devnet_hard_forks[0], &devnet_hard_forks[devnet_hard_forks.size()]};
  if (type == network_type::FAKECHAIN) return {fakechain_hardforks.data(), fakechain_hardforks.data() + fakechain_hardforks.size()};
  return {nullptr, nullptr};
}


std::pair<std::optional<uint64_t>, std::optional<uint64_t>>
get_hard_fork_heights(network_type nettype, uint8_t version) {
  std::pair<std::optional<uint64_t>, std::optional<uint64_t>> found;
  for (auto [it, end] = get_hard_forks(nettype); it != end; it++) {
    if (it->version > version) { // This (and anything else) are in the future
      if (found.first) // Found something suitable in the previous iteration, so one before this hf is the max
        found.second = it->height - 1;
      break;
    } else if (it->version == version) {
      found.first = it->height;
    }
  }
  return found;
}

uint8_t hard_fork_ceil(network_type nettype, uint8_t version) {
  auto [it, end] = get_hard_forks(nettype);
  for (; it != end; it++)
    if (it->version >= version)
      return it->version;

  return version;
}

std::pair<uint8_t, uint8_t>
get_network_version_revision(network_type nettype, uint64_t height) {
  std::pair<uint8_t, uint8_t> result;
  for (auto [it, end] = get_hard_forks(nettype); it != end; it++) {
    if (it->height <= height)
      result = {it->version, it->snode_revision};
    else
      break;
  }
  return result;
}

bool is_hard_fork_at_least(network_type type, uint8_t version, uint64_t height) {
  return get_network_version(type, height) >= version;
}

std::pair<uint8_t, uint8_t>
get_ideal_block_version(network_type nettype, uint64_t height)
{
  std::pair<uint8_t, uint8_t> result;
  for (auto [it, end] = get_hard_forks(nettype); it != end; it++) {
    if (it->height <= height)
      result.first = it->version;
    result.second = it->version;
  }
  return result;
}


}

