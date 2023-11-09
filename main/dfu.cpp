#include "dfu.hpp"

extern "C" uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state) {
  return 0;
}

extern "C" void tud_dfu_download_cb(uint8_t alt, uint16_t block_num, uint8_t const *data, uint16_t length) {

}

extern "C" void tud_dfu_manifest_cb(uint8_t alt) {

}
