#include "dummy_nic.hpp"

void DummyNIC::send_packet(const Packet &pkt) {
  std::lock_guard<std::mutex> lock(tx_mtx);
  tx_queue.push(pkt);
}

bool DummyNIC::recv_packet(Packet &pkt) {
  std::lock_guard<std::mutex> lock(rx_mtx);
  if (rx_queue.empty())
    return false;
  pkt = rx_queue.front();
  rx_queue.pop();
  return true;
}

void DummyNIC::inject_packet(const Packet &pkt) {
  std::lock_guard<std::mutex> lock(rx_mtx);
  rx_queue.push(pkt);
}

bool DummyNIC::get_tx_packet(Packet &pkt) {
  std::lock_guard<std::mutex> lock(tx_mtx);
  if (tx_queue.empty())
    return false;
  pkt = tx_queue.front();
  tx_queue.pop();
  return true;
}
