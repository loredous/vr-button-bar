import usb_hid
from gamepad import gamepad

# These are the default devices, so you don't need to write
# this explicitly if the default is what you want.
usb_hid.enable(
    (gamepad,)
)