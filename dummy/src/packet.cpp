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
