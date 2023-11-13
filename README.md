# ESP32s3 USB HID & VCP Example

This is an example project for the ESP32s3 that uses the USB peripheral to
implement both a USB HID device and a USB VCP device.

![CleanShot 2023-11-09 at 14 43 03](https://github.com/finger563/esp-usb-hid-vcp-example/assets/213467/82710d71-57eb-43fc-a1c0-4e3a5b5f189e)

![CleanShot 2023-11-09 at 16 10 48](https://github.com/finger563/esp-usb-hid-vcp-example/assets/213467/fba5b18a-72b3-4946-9d91-b09e3c440293)

## Cloning

Since this repo contains a submodule, you need to make sure you clone it
recursively, e.g. with:

``` sh
git clone --recurse-submodules git@github.com:finger563/esp-usb-hid-vcp-example
```

Alternatively, you can always ensure the submodules are up to date after cloning
(or if you forgot to clone recursively) by running:

``` sh
git submodule update --init --recursive
```

## Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Output

![CleanShot 2023-11-09 at 14 43 03](https://github.com/finger563/esp-usb-hid-vcp-example/assets/213467/82710d71-57eb-43fc-a1c0-4e3a5b5f189e)
