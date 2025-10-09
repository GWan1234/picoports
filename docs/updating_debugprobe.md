# Updating Debugprobe

- Check that the Picoports specific patches on top of upstream Debugprobe can be applied.
- Diff the changes in `usb_descriptors.c` between the previous and the new Debugprobe version,
  because we're replacing that with Picoports' `usb_descriptors.c`.
- Verify the license is still GPL compatible.
