
#include "modules/drivers/rfid/rfid_component.h"

#include "modules/common/adapters/adapter_gflags.h"

namespace apollo {
namespace drivers {
namespace rfid {

std::string RfidComponent::Name() const { return "rfid"; }

RfidComponent::RfidComponent() {}

bool RfidComponent::Init() {
  // Uart device set option
  device_.SetOpt(9600, 8, 'N', 1);

  // Publish rfid station data
  rfid_writer_ = node_->CreateWriter<RFID>(FLAGS_rfid_topic);

  // Async read
  async_action_ = cyber::Async(&RfidComponent::Action, this);
  return true;
}

// TODO()CHECK
bool RfidComponent::Check() {
  // sum check for rfid result
  int check_value = 0;// buffer[11];

  // char new_hex;
  // auto transfered_size = stringToHex(&buffer + 1, &new_hex);
  // AINFO << "transfered size : " << transfered_size
  //       << " retrun transfered: " << new_hex;

  int value = 0;
  // for (int i = 0; i < 11; i + 2) {
    // value | = new_hex[i];
  // }

  return value == check_value;
}

void RfidComponent::Action() {
  int count = 0;
  static char buffer[13];
  static char buf;
  while (!apollo::cyber::IsShutdown()) {
    // count = 1;
    std::memset(buffer, 0, 13);
    while (1) {
      int ret = device_.Read(&buf, 1);
      AINFO << "RFID Device return: " << ret;
      if (ret == 1) {
        AINFO << "RFID Device buf: " << buf;
        // 0x02 Head
        // 0x03 End
        if (buf == 0x02) {
          count = 0;
          break;
        }
        buffer[count] = buf;
        count++;
      }
      AINFO << "count: " << count;
      if (buf == 0x03 && count == 13) {
        AINFO << "origin id from buffer[10]: " << buffer[10];
        uint32_t station_id = buffer[10] - '0';
        AINFO << "TRANSFER ID :" << station_id;

        apollo::drivers::RFID rfid;
        auto header = rfid.mutable_header();
        header->set_timestamp_sec(apollo::cyber::Time::Now().ToSecond());
        header->set_frame_id("rfid");

        rfid.set_id(station_id);

        rfid_writer_->Write(rfid);
      }
    }
  }
}

RfidComponent::~RfidComponent() { AINFO << "RfidComponent::~RfidComponent()"; }

}  // namespace rfid
}  // namespace drivers
}  // namespace apollo
