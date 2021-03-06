#include "modules/lcd/lcd_display.h"

#include "math.h"

namespace apollo {
namespace drivers {
namespace lcd {
bool LcdComponet::Init() {
  if (!GetProtoConfig(&device_conf_)) {
    AERROR << "Unable to load lcd conf file: " << ConfigFilePath();
    return false;
  }
  device_ = std::make_unique<Uart>(device_conf_.device_id().c_str());
  // Uart device set option
  device_->SetOpt(9600, 8, 'N', 1);

  // /diamond/canbus/chassis
  chassis_reader_ = node_->CreateReader<Chassis>(
      "/diamond/canbus/chassis",
      [this](const std::shared_ptr<Chassis>& chassis) {
        chassis_.CopyFrom(*chassis);
      });
  // AINFO << listener;
  async_action_ = cyber::Async(&LcdComponet::Proc, this);
  // async_action_=std::async(std::launch::async,&LcdComponet::Proc,this);
  return true;
}
bool LcdComponet::Proc() {
  int cout = 0;
  while (!apollo::cyber::IsShutdown()) {
    unsigned char vehicle_id[8] = {0x01, 0x06, 0x00, 0x00,
                                   0x00, 0x01, 0x48, 0x0A};
    int results = device_->Write(vehicle_id, 8);
    AINFO << "results->" << results;
    unsigned char table[11][8] = {
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0xD8, 0x0A},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x01, 0x19, 0xCA},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x02, 0x59, 0xCB},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x03, 0x98, 0x0B},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x04, 0xD9, 0xC9},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x05, 0x18, 0x09},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x06, 0x58, 0x08},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x07, 0x99, 0xC8},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x08, 0xD9, 0xCC},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x09, 0x18, 0x0C},
        {0x01, 0x06, 0x00, 0x01, 0x00, 0x0A, 0x58, 0x0D}};
    unsigned char eq[20][8] = {
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x00, 0xC8, 0x0B},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x01, 0x09, 0xCB},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x05, 0x08, 0x08},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x0A, 0x48, 0x0C},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x0F, 0x88, 0x0F},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x14, 0xC8, 0x04},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x19, 0x09, 0xC1},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x1E, 0x48, 0x03},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x23, 0x89, 0xD2},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x28, 0xC8, 0x15},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x29, 0x09, 0xD5},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x2A, 0x49, 0xD4},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x2B, 0x88, 0x14},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x2C, 0xC9, 0xD6},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x2D, 0x08, 0x16},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x2E, 0x48, 0x17},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x2F, 0x89, 0xD7},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x30, 0x09, 0xDF},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x31, 0x09, 0xDF},
        {0x01, 0x06, 0x00, 0x04, 0x00, 0x32, 0x49, 0xDE}};

    unsigned char mode0[8] = {0x01, 0x06, 0x00, 0x03, 0x00, 0x00, 0x79, 0xCA};
    unsigned char mode1[8] = {0x01, 0x06, 0x00, 0x03, 0x00, 0x01, 0xB8, 0x0A};
    while (1) {
      int speed = round(3.6 * chassis_.speed_mps());
      if (speed == 0) {
        int modes = device_->Write(mode0, 8);
        AINFO << "modes->" << modes;
      } else {
        int modes = device_->Write(mode1, 8);
        AINFO << "modese->" << modes;
      }
      for (int j = 0; j < 11; j++) {
        if (speed == table[j][5]) {
          int sd = device_->Write(table[speed], 8);

          AINFO << "sd->" << sd;
        }
      }
      AINFO << chassis_.bat_percentage();
      if (chassis_.bat_percentage() <= 40) {
        cout = round(chassis_.bat_percentage() * 0.1);
      } else {
        cout = round(chassis_.bat_percentage());
      }
      for (int i = 0; i < 20; i++) {
        // std::cout << i << std::endl;
        if (cout == eq[i][5]) {
          int percentage = device_->Write(eq[i], 8);
          AINFO << "eq->" << percentage;
          AINFO << i;
        }
      }
      if (chassis_.driving_mode() == 0 or chassis_.driving_mode() == 1) {
        unsigned char driving_mode[8] = {0x01, 0x06, 0x00, 0x02,
                                         0x00, 0x01, 0xE9, 0xCA};
        int driving = device_->Write(driving_mode, 8);
        AINFO << "0-1 driving->" << driving;
      } else if (chassis_.driving_mode() == 1 or chassis_.driving_mode() == 2 or
                 chassis_.driving_mode() == 3) {
        unsigned char driving_mode[8] = {0x01, 0x06, 0x00, 0x02,
                                         0x00, 0x00, 0x28, 0x0A};
        int driving = device_->Write(driving_mode, 8);
        AINFO << "1-2-3 driving->" << driving;
      }
    }
  }
  return true;
}
}  // namespace lcd
}  // namespace drivers
}  // namespace apollo
