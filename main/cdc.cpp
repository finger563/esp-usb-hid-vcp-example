#include "cdc.hpp"

static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read((tinyusb_cdcacm_itf_t)itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
      fmt::print("Data from channel {}:\n", itf);
      std::vector<uint8_t> data(buf, buf + rx_size);
      fmt::print("  {::02x}\n", data);
    } else {
      fmt::print("read error\n");
    }

    /* write back */
    tinyusb_cdcacm_write_queue((tinyusb_cdcacm_itf_t)itf, buf, rx_size);
    tinyusb_cdcacm_write_flush((tinyusb_cdcacm_itf_t)itf, 0);
}

void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    fmt::print("Line state changed on channel {}: DTR:{}, RTS:{}\n", itf, dtr, rts);
}
