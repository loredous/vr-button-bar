import struct
import time

from adafruit_hid import find_device
import usb_hid

# Gamepad Descriptor
GAMEPAD_REPORT_DESCRIPTOR = bytes((
    0x05, 0x01,  
    0x09, 0x05,  
    0xA1, 0x01,  
    0x85, 0x04,  
    0x05, 0x09,  
    0x19, 0x01,  
    0x29, 0x0C,  
    0x15, 0x00,  
    0x25, 0x01,  
    0x75, 0x01,  
    0x95, 0x0C,  
    0x81, 0x02,  
    0x95, 0x01,  
    0x75, 0x04,  
    0x81, 0x03,  
    0xC0,        
))

gamepad = usb_hid.Device(
    report_descriptor=GAMEPAD_REPORT_DESCRIPTOR,
    usage_page=0x01,           # Generic Desktop Control
    usage=0x05,                # Gamepad
    report_ids=(4,),           # Descriptor uses report ID 4.
    in_report_lengths=(2,),    # This gamepad sends 2 bytes in its report.
    out_report_lengths=(0,),   # It does not receive any reports.
)


class Gamepad:
    """Emulate a generic gamepad controller with 12 buttons,
    numbered 1-12"""

    def __init__(self, devices):
        """Create a Gamepad object that will send USB gamepad HID reports.

        Devices can be a list of devices that includes a gamepad device or a gamepad device
        itself. A device is any object that implements ``send_report()``, ``usage_page`` and
        ``usage``.
        """
        self._gamepad_device = find_device(devices, usage_page=0x1, usage=0x05)

        # Reuse this bytearray to send button reports.
        # Typically controllers start numbering buttons at 1 rather than 0.
        # report[0] buttons 1-8 (LSB is button 1)
        # report[1] buttons 9-12
        self._report = bytearray(2)

        # Remember the last report as well, so we can avoid sending
        # duplicate reports.
        self._last_report = bytearray(2)

        # Store settings separately before putting into report. Saves code
        # especially for buttons.
        self._buttons_state = 0

        # Send an initial report to test if HID device is ready.
        # If not, wait a bit and try once more.
        try:
            self.reset_all()
        except OSError:
            time.sleep(1)
            self.reset_all()

    def press_buttons(self, *buttons):
        """Press and hold the given buttons."""
        for button in buttons:
            self._buttons_state |= 1 << self._validate_button_number(button) - 1
        self._send()

    def release_buttons(self, *buttons):
        """Release the given buttons."""
        for button in buttons:
            self._buttons_state &= ~(1 << self._validate_button_number(button) - 1)
        self._send()

    def release_all_buttons(self):
        """Release all the buttons."""
        self._buttons_state = 0
        self._send()

    def click_buttons(self, *buttons):
        """Press and release the given buttons."""
        self.press_buttons(*buttons)
        self.release_buttons(*buttons)

    def reset_all(self):
        """Release all buttons."""
        self._buttons_state = 0
        self._send(always=True)

    def _send(self, always=False):
        """Send a report with all the existing settings.
        If ``always`` is ``False`` (the default), send only if there have been changes.
        """
        struct.pack_into(
            "<BB0b",
            self._report,
            0,
            self._buttons_state
        )

        print(self._report)

        if always or self._last_report != self._report:
            self._gamepad_device.send_report(self._report)
            # Remember what we sent, without allocating new storage.
            self._last_report[:] = self._report

    @staticmethod
    def _validate_button_number(button):
        if not 1 <= button <= 12:
            raise ValueError("Button number must in range 1 to 16")
        return button