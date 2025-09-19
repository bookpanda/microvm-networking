#pragma once
#include "dummy_nic.hpp"
#include <cstdint>
#include <vector>

class StackAPI {
  DummyNIC &nic;

public:
  StackAPI(DummyNIC &n);
  void send_packet(const std::vector<uint8_t> &data);
  bool recv_packet(std::vector<uint8_t> &data);
};
