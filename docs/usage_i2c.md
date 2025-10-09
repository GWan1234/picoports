# Usage I2C

For I2C access from the command line you can use the tools provided by the package `i2c-tools`

```bash
sudo apt install i2c-tools
```

Example: Find your I2C interface device; subsequently, only the ID is required (look for "dln2-i2c"
in the third column; here the I2C interface device is `i2c-1`: ID `1`)

```bash
i2cdetect -l
# Example output:
# i2c-0   smbus   SMBus I801 adapter at efa0   SMBus adapter
# i2c-1   i2c     dln2-i2c-3-1.1:1.0-0         I2C adapter
```

Example: Scan for device on I2C bus using read operation* (using I2C interface device `i2c-1`)

```bash
i2cdetect -r 1
```

*I2C quick write is not supported, so detection has to use the read command (`-r` option). This may
create issues, depending on the attached devices, see also `man i2cdetect`.

Example: Write `0x40 0xff` to the device at I2C address `0x48` (using I2C interface device `i2c-1`)

```bash
i2cset 1 0x48 0x40 0xff
```

Example: Read data from the device at I2C address `0x48` (using I2C interface device `i2c-1`)

```bash
i2cget 1 0x48
```

Example: Attach kernel driver `pcf8591` for a Philips PCF8591 ADC/DAC at I2C address `0x48` (using
I2C interface device `i2c-1`)

```bash
echo pcf8591 0x48 | sudo tee /sys/class/i2c-adapter/i2c-1/new_device
# Example: Read value from first ADC channel
cat /sys/class/i2c-adapter/i2c-1/1-0048/in0_input
```

I2C pin assignment:

| Pico Pin | GP16 | GP17 |
|----------|------|------|
| I2C line |  SDA |  SCL |
