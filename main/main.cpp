#include <sdkconfig.h>

#include <chrono>
#include <thread>

#include <driver/uart.h>

#include <tinyusb.h>

#include "logger.hpp"
#include "task.hpp"

#include "cdc.hpp"
#include "hid.hpp"

using namespace std::chrono_literals;

// Some of the code below was taken from the TinyUSB example for hid + cdc
// device at
// https://github.com/hathach/tinyusb/blob/master/examples/dual/host_hid_to_device_cdc/src/usb_descriptors.c

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
  .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x02
};

// String Descriptor Index
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_CDC_INTERFACE,
  STRID_HID_INTERFACE,
};

const char* string_descriptor[] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},        // 0: is supported language is English (0x0409)
    "Finger563",                 // 1: Manufacturer
    "Finger563 USB HID & CDC Device",  // 2: Product
    "123456",                    // 3: Serials, should use chip ID
    "Finger563 USB CDC",         // 4: Interface 1 string
    "USB HID interface",         // 5: Interface 2 string
};

enum
{
  ITF_NUM_CDC = 0,
  ITF_NUM_CDC_DATA,
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

//------------- USB Endpoint numbers -------------//
enum {
    // Available USB Endpoints: 5 IN/OUT EPs and 1 IN EP
    EP_EMPTY = 0,
    EPNUM_CDC_NOTIF,
    EPNUM_CDC,
    EPNUM_HID,
};

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + (1 * TUD_CDC_DESC_LEN) + (1 * TUD_HID_DESC_LEN))

static const uint8_t configuration_descriptor[] = {
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
  // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(0, STRID_CDC_INTERFACE, EPNUM_CDC_NOTIF, 8, EPNUM_CDC, 0x80 | EPNUM_CDC, CFG_TUD_CDC_EP_BUFSIZE),
  // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
  TUD_HID_DESCRIPTOR(2, STRID_HID_INTERFACE, HID_ITF_PROTOCOL_NONE, desc_hid_report_len, 0x80 | EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10),
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
extern "C" uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf)
{
  return desc_hid_report;
}

extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
  return 0;
}

extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
}

extern "C" void app_main(void) {
  static auto start = std::chrono::high_resolution_clock::now();
  static auto elapsed = [&]() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float>(now - start).count();
  };

  espp::Logger logger({.tag = "USB HID VCP", .level = espp::Logger::Verbosity::DEBUG});

  logger.info("Bootup");

  const tinyusb_config_t tusb_cfg = {
    .device_descriptor = &desc_device,
    .string_descriptor = string_descriptor,
    .string_descriptor_count = sizeof(string_descriptor) / sizeof(string_descriptor[0]),
    .external_phy = false,
    .configuration_descriptor = configuration_descriptor,
  };
  tinyusb_driver_install(&tusb_cfg);

  tinyusb_config_cdcacm_t acm_cfg = {
    .usb_dev = TINYUSB_USBDEV_0,
    .cdc_port = TINYUSB_CDC_ACM_0,
    .rx_unread_buf_sz = 64,
    .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
    .callback_rx_wanted_char = NULL,
    .callback_line_state_changed = NULL,
    .callback_line_coding_changed = NULL
  };

  ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
  ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                                                   TINYUSB_CDC_ACM_0,
                                                   CDC_EVENT_LINE_STATE_CHANGED,
                                                   &tinyusb_cdc_line_state_changed_callback));
  // make a simple task that prints "Hello World!" every second
  espp::Task task({
      .name = "example task",
        .callback = [&](auto &m, auto &cv) -> bool {
          auto log_str = fmt::format("[{:.3f}] Hello from the task!\r\n", elapsed());
          logger.debug(log_str);
          // also print it out to the USB CDC
          if (tud_cdc_n_connected(0)) {
            tud_cdc_n_write(0, (uint8_t *)log_str.c_str(), log_str.size());
            tud_cdc_n_write_flush(0);
          }
          // now send some data out on the game controller
          static uint8_t ifIdx = 0;
          static uint8_t report_id = 0;
          static uint8_t report[2];
          static uint8_t report_size = sizeof(report);

          static int8_t x = 0, y = 0, z = 0, rz = 0, rx = 0, ry = 0;
          static uint8_t hat = 0;
          static uint32_t buttons = 0;
          if (tud_hid_n_ready(ifIdx)) {
            // generic hid report:
            // tud_hid_n_report(ifIdx, report_id, report, report_size);

            // mouse HID report:
            // instance, report_id, buttons, x, y, vertical wheel, horizontal wheel
            tud_hid_n_mouse_report(ifIdx, report_id, (uint8_t)buttons, x, y, z, rz);

            switch (x) {
            case -127:
              x = 0;
              break;
            case 0:
              x = 127;
              break;
            case 127:
              x = -127;
              break;
            default:
              break;
            }

            // NOTE: tinyUSB has a GAMEPAD report descriptor and helper functions if we
            //       wanted to use their gamepad implementation:
            // use template layout report TUD_HID_REPORT_DESC_GAMEPAD
            // bool sent = tud_hid_n_gamepad_report(ifIdx, report_id,
            //                                      x, y, z, rz, rx, ry, hat, buttons);
          }
        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, 1s);
        // we don't want to stop the task, so return false
        return false;
      },
      .stack_size_bytes = 4096,
    });
  task.start();

  while (true) {
    std::this_thread::sleep_for(1s);
  }
}
