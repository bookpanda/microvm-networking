#include "stack_api.hpp"

StackAPI::StackAPI(DummyNIC &n) : nic(n) {}

void StackAPI::send_packet(const std::vector<uint8_t> &data) {
  Packet pkt{data};
  nic.send_packet(pkt);
}

bool StackAPI::recv_packet(std::vector<uint8_t> &data) {
  Packet pkt;
  if (!nic.recv_packet(pkt))
    return false;
  data = pkt.data;
  return true;
}
