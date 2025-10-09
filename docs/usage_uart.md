# Usage UART

Note: Many systems provide a symlink of the form
`/dev/serial/by-id/usb-PicoPorts_GPIO_Expander_{{DEVICE_ID}}-if01` ([udev rule](https://github.com/systemd/systemd/blob/main/rules.d/60-serial.rules)),
which can be used instead of `/dev/ttyACM*`.

Example: Open a serial terminal to interact with the UART (using tty device `/dev/ttyACM0` and `tio`
tool with default setting 115200:8n1)

```bash
tio /dev/ttyACM0
```

Example: Use custom UART settings (using tty device `/dev/ttyACM0` and `tio` tool)

```bash
tio /dev/ttyACM0 --baudrate 9600 --databits 7 --parity odd --stopbits 2
```

(supported databits: 5, 6, 7, 8; supported parity: none, even, odd; supported stopbits: 1, 2)

UART pin assignment:

| Pico Pin  | GP20 | GP21 |
|-----------|------|------|
| UART line |   TX |   RX |
