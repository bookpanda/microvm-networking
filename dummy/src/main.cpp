#include "dummy_nic.hpp"
#include "stack_api.hpp"
#include <chrono>
#include <iostream>
#include <thread>

void processing_thread(DummyNIC &nic) {
  while (true) {
    Packet pkt;
    if (nic.recv_packet(pkt)) {
      // Minimal processing: echo back
      nic.send_packet(pkt);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

int main() {
  DummyNIC nic;
  StackAPI api(nic);

  // Launch processing thread
  std::thread proc(processing_thread, std::ref(nic));

  // Inject a packet
  std::vector<uint8_t> data = {0x01, 0x02, 0x03};
  nic.inject_packet({data});

  // Receive packet via API
  std::vector<uint8_t> recv_data;
  while (!api.recv_packet(recv_data)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  std::cout << "Received packet of size " << recv_data.size() << std::endl;

  proc.join();
  return 0;
}
