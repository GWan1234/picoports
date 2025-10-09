# Usage ADC

The ADC is registered as an IIO (industrial I/O) device.

Example: Analog read of GP26/ADC0 (in volt; using IIO device `iio:device0`)

```bash
echo "$(cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw) * $(cat /sys/bus/iio/devices/iio:device0/scale)" | bc
```

Special case: Analog read of Pico VSYS (in volt; using IIO device `iio:device0`)

```bash
echo "$(cat /sys/bus/iio/devices/iio:device0/in_voltage3_raw) * $(cat /sys/bus/iio/devices/iio:device0/scale) * 3" | bc
```

Special case: Analog read of Pico onboard temperature sensor (in degree Celsius; formula according
to Pico datasheet; using IIO device `iio:device0`)

```bash
echo "27.0 - ($(cat /sys/bus/iio/devices/iio:device0/in_voltage4_raw) * $(cat /sys/bus/iio/devices/iio:device0/scale) - 0.706) / 0.001721" | bc
```

ADC pin assignment (voltage index is the `X` in `in_voltageX_raw`):

| Pico Pin                  | GP26/ADC0 | GP27/ADC1 | GP28/ADC2 | (VSYS/3) | (temp. raw) |
|---------------------------|-----------|-----------|-----------|----------|-------------|
| voltage index             |         0 |         1 |         2 |        3 |           4 |

Note: The buffer functionality is not implemented yet.
