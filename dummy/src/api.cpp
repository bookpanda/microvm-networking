class StackAPI {
  DummyNIC &nic;

public:
  StackAPI(DummyNIC &n) : nic(n) {}

  void send_packet(const std::vector<uint8_t> &data) {
    Packet pkt{data};
    nic.send_packet(pkt);
  }

  bool recv_packet(std::vector<uint8_t> &data) {
    Packet pkt;
    if (!nic.recv_packet(pkt))
      return false;
    data = pkt.data;
    return true;
  }
};
