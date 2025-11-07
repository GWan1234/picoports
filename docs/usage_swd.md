# Usage SWD

SWD debugging can be used in the same way the
[Raspberry Pi Debug Probe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html#about-the-debug-probe)
is used.

Example: Debug a Respberry Pi Pico (`rp2040`) using `openocd` and `gdb`.

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000"
# Usually opens the port 3333 and 3334
```

```bash
gdb-multiarch PATH_TO_FIRMWARE_BINARY.elf -ex 'target extended-remote :3333'
```

SWD pin assignment:

| Pico Pin | GP18  | GP19  |
|----------|-------|-------|
| SWD line | SWCLK | SWDIO |
