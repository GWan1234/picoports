# Development Setup

```shell
git clone --no-recurse-submodules https://github.com/sevenlab-de/picoports.git
cd picoports
git submodule update --init -- pico-sdk debugprobe
git -C pico-sdk submodule update --init -- lib/tinyusb
git -C debugprobe submodule update --init -- freertos
# Only required for Pico 2 build:
git -C debugprobe/freertos submodule update --init -- portable/ThirdParty/Community-Supported-Ports
```

## Build

```shell
cmake -B build -DPICO_BOARD=pico -DLOG_ON_GP01=no -DBOOTSEL_BUTTON=no
make -C build
# quick install (if your system uses udisks2 auto-mounting):
cp build/picoports.uf2 /media/$USER/RPI-RP2/
```

- `PICO_BOARD`: Choose between `pico` = Build for Raspberry Pi Pico (1) and
  `pico2` = Build for Raspberry Pi Pico 2. Firmware images are not compatible between
  Pico (1) and Pico 2. CMake cache needs to be cleaned before changing the board.
- `LOG_ON_GP01`: Enable debug logging on GP0/GP1, TX/RX resp. (GPIO lines will start at GP2)
- `BOOTSEL_BUTTON`: Pressing the button resets the pico into BOOTSEL mode (GPIO line for button will
  not be available)

## Theory of operation

PicoPorts works without a custom driver, because it's using a driver that already exists. The driver
is called `dln2` (`gpio-dln2`, `dln2-adc`, `i2c-dln2`) and was written for the Diolan DLN-2 USB
adapter. PicoPorts just implements the other side of the interface which the driver provides. It's
mainly a glue layer from this kernel interface to the Pico's SDK interface.

Many thanks to the contributors who upstreamed this driver!

Additionally, PicoPorts incorporates the [debugprobe](https://github.com/raspberrypi/debugprobe/)
project. Luckily OpenOCD does not use the vendor and product ID of the device, but scans the product
name and interface name for `"CMSIS-DAP"`, so we can keep using the vendor and product ID required
for detection by the `dln2` driver.

### What about SPI?

The SPI subsystem is troublesome in the kernel. While a `spi-dln2` driver exists, it's simply not
possible to attach a device driver to a hotpluggable SPI interface without major efforts from the
user. The only way to attach a SPI device to an interface is via Devicetree/ACPI, which is not made
to be used with hotpluggable devices like USB devices.

As a workaround, PicoPorts may in the future add a custom USB interface for SPI and add a tool which
works similarly to the `spidev` virtual device driver, but in user space using `libusb`.

## Further resources

- All one needs to know about USB: <https://www.beyondlogic.org/usbnutshell/usb1.shtml>
