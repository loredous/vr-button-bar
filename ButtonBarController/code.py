import board
import digitalio
from adafruit_debouncer import Debouncer
from gamepad import Gamepad
import usb_hid

BUTTON_DICT = {
    board.IO21: {"name":"side_2", "button": None, "gpid": 1},
    board.IO33: {"name":"flat_1", "button": None, "gpid": 2},
    board.IO36: {"name":"flat_2", "button": None, "gpid": 3},
    board.IO37: {"name":"small_bottom_1", "button": None, "gpid": 4},
    board.IO40: {"name":"small_bottom_2", "button": None, "gpid": 5},
    board.IO11: {"name":"big_1", "button": None, "gpid": 6},
    board.IO8: {"name":"big_2", "button": None, "gpid": 7},
    board.IO7: {"name":"big_3", "button": None, "gpid": 8},
    board.IO4: {"name":"big_4", "button": None, "gpid": 9},
    board.IO3: {"name":"small_top_1", "button": None, "gpid": 10},
    board.IO1: {"name":"small_top_2", "button": None, "gpid": 11},
    board.IO34: {"name":"side_1", "button": None, "gpid": 12}
}

pad = Gamepad(usb_hid.devices)

led = digitalio.DigitalInOut(board.LED)
led.direction = digitalio.Direction.OUTPUT

for pin in BUTTON_DICT:
    btn = digitalio.DigitalInOut(pin)
    btn.pull = digitalio.Pull.UP
    BUTTON_DICT[pin]["button"] = Debouncer(btn)

while True:
    for pin in BUTTON_DICT:
        BUTTON_DICT[pin]['button'].update()
        if BUTTON_DICT[pin]['button'].rose:
            print(f'Button {BUTTON_DICT[pin]['name']} rose!')
            pad.release_buttons(BUTTON_DICT[pin]['gpid'])
        if BUTTON_DICT[pin]['button'].fell:
            print(f'Button {BUTTON_DICT[pin]['name']} fell!')
            pad.press_buttons(BUTTON_DICT[pin]['gpid'])