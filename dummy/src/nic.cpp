#include <mutex>
#include <queue>
#include <vector>

struct Packet {
  std::vector<uint8_t> data;
};

class DummyNIC {
  std::queue<Packet> rx_queue;
  std::queue<Packet> tx_queue;
  std::mutex rx_mtx, tx_mtx;

public:
  void send_packet(const Packet &pkt) {
    std::lock_guard<std::mutex> lock(tx_mtx);
    tx_queue.push(pkt);
  }

  bool recv_packet(Packet &pkt) {
    std::lock_guard<std::mutex> lock(rx_mtx);
    if (rx_queue.empty())
      return false;
    pkt = rx_queue.front();
    rx_queue.pop();
    return true;
  }

  void inject_packet(const Packet &pkt) {
    std::lock_guard<std::mutex> lock(rx_mtx);
    rx_queue.push(pkt);
  }

  bool get_tx_packet(Packet &pkt) {
    std::lock_guard<std::mutex> lock(tx_mtx);
    if (tx_queue.empty())
      return false;
    pkt = tx_queue.front();
    tx_queue.pop();
    return true;
  }
};
