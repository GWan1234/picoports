# Usage ADC

The ADC is registered as an IIO (industrial I/O) device.

Example: Analog read of GP26/ADC0 (in volt; using IIO device `iio:device0`)

```bash
echo "$(cat /sys/bus/iio/devices/iio\:device0/in_voltage0_raw) * $(cat /sys/bus/iio/devices/iio\:device0/scale)" | bc
```

Special case: Analog read of Pico VSYS (in volt; using IIO device `iio:device0`)

```bash
echo "$(cat /sys/bus/iio/devices/iio\:device0/in_voltage3_raw) * $(cat /sys/bus/iio/devices/iio\:device0/scale) * 3" | bc
```

Special case: Analog read of Pico onboard temperature sensor (in degree Celsius; formula according
to Pico datasheet; using IIO device `iio:device0`)

```bash
echo "27.0 - ($(cat /sys/bus/iio/devices/iio\:device0/in_voltage4_raw) * $(cat /sys/bus/iio/devices/iio\:device0/scale) - 0.706) / 0.001721" | bc
```

Example: Multiple analog reads of GP26/ADC0 and GP28/ADC2 (using IIO device `iio:device0` with 20 Hz
sampling frequency)

Note: The values are in binary format, described by "le:u10/16>>0", see Kernel documentation for
["Industrial IIO device buffers" > "Scan Elements" > "_type"](https://docs.kernel.org/iio/iio_devbuf.html#scan-elements),
i.e., they are unsigned 16 bit integers in little-endian format and only the lower 10 bit are used.

Option 1: Use the tools provided by the package `iiod` and `libiio-utils`

```bash
sudo apt install iiod libiio-utils
```

```bash
iio_attr --uri=ip:127.0.0.1 -d iio:device0 sampling_frequency 20
iio_readdev --uri=ip:127.0.0.1 --samples=10 --buffer-size=32 --timeout=0 iio:device0 voltage0 voltage2 | hexdump -v
# Example output:
#   0000000 0005 01fd 0005 01fd 0005 01fd 0005 01fd
#   0000010 0005 01fd 0005 01fd 0005 01fd 0005 01fd
#   0000020 0005 01fd 0005 01fd
#   0000028
```

Note: For application use, there are multiple other bindings for libiio, e.g., Python, Rust, C#...

Option 2: The same can be achieved just via `/sys` (requires root access):

```bash
echo 20 > /sys/bus/iio/devices/iio\:device0/sampling_frequency
# Enable GP26/ADC0 and GP28/ADC2
echo 1 > /sys/bus/iio/devices/iio\:device0/scan_elements/in_voltage0_en
echo 1 > /sys/bus/iio/devices/iio\:device0/scan_elements/in_voltage2_en
# Start sampling
echo 1 > /sys/bus/iio/devices/iio\:device0/buffer/enable
# Read values
hexdump -v /dev/iio\:device0
# Stop sampling
echo 0 > /sys/bus/iio/devices/iio\:device0/buffer/enable
```

ADC pin assignment (voltage index is the `X` in `in_voltageX_raw`):

| Pico Pin                  | GP26/ADC0 | GP27/ADC1 | GP28/ADC2 | (VSYS/3) | (temp. raw) |
|---------------------------|-----------|-----------|-----------|----------|-------------|
| voltage index             |         0 |         1 |         2 |        3 |           4 |
