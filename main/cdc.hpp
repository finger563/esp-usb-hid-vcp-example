#pragma once

#include <vector>
#include <cstdint>
#include <stdint.h>

#include <tinyusb.h>
#include <tusb_cdc_acm.h>

#include "format.hpp"

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event);
void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event);
