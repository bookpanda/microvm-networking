#pragma once
#include "packet.hpp"
#include <mutex>
#include <queue>

class DummyNIC {
  std::queue<Packet> rx_queue;
  std::queue<Packet> tx_queue;
  std::mutex rx_mtx, tx_mtx;

public:
  void send_packet(const Packet &pkt);
  bool recv_packet(Packet &pkt);
  void inject_packet(const Packet &pkt);
  bool get_tx_packet(Packet &pkt);
};
