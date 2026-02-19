# Usage GPIO

For GPIO access from the command line you can use the tools provided by the package `gpiod`

```bash
sudo apt install gpiod
```

Example: Find your gpiochip device name (look for "dln2" in the second column; here the gpiochip
device is `gpiochip1`)

```bash
gpiodetect
# Example output:
# gpiochip0 [INT34C6:00] (463 lines)
# gpiochip1 [dln2] (20 lines)
```

Example: Read GP0 (using gpiochip device `gpiochip1`)

```bash
gpioget gpiochip1 0
```

Example: Set GP8 high for 1 second (using gpiochip device `gpiochip1`)

```bash
# Note that PicoPorts' GPIO states persist after gpioset command exits.
gpioset gpiochip1 8=1
sleep 1
gpioset gpiochip1 8=0
```

Special case: Switch on Pico LED (connected to GP25; using gpiochip device `gpiochip1`)

```bash
gpioset gpiochip1 19=1
```

## Using multiple devices

> [!NOTE]
> The method described below [was merged into Systemd](https://github.com/systemd/systemd/pull/39625)
> and thus will not be necessary for Systemd v259 and later.

When using multiple PicoPorts devices, it's not easy to determine the exact gpiochip device using
`gpiodetect`. Instead, we can implement a `udev` rule to assign unique symlinks to the devices based
on their device ID or their USB bus path. This rule is provided in `60-gpiochip.rules`. You can
install it with:

```bash
sudo cp 60-gpiochip.rules /etc/udev/rules.d/
sudo udevadm control --reload
```

Then unplug and plug in the USB connection.

After installation, `udev` will create symlinks of the form:

- `/dev/gpio/by-id/usb-PicoPorts_GPIO_Expander_{{DEVICE_ID}}-if00`
- `/dev/gpio/by-path/{{BUS_PATH}}-platform-dln2-gpio.*.auto`

The `gpiod` tools accept device paths instead of `gpiochip*` identifiers.

Example: Read GP0 (using gpiochip device `/dev/gpio/by-id/usb-PicoPorts_GPIO_Expander_E660012345678901-if00`)

```bash
gpioget /dev/gpio/by-id/usb-PicoPorts_GPIO_Expander_E660012345678901-if00 0
```
