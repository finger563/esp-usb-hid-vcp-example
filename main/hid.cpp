#include "hid.hpp"

uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_MOUSE()
  // TUD_HID_REPORT_DESC_KEYBOARD()
  // TUD_HID_REPORT_DESC_GAMEPAD()
};

size_t desc_hid_report_len = sizeof(desc_hid_report);
