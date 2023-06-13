# vr-button-bar
An open-design VR controller designed for making simple actions easy when doing mouse/keyboard work in VR environments.

## Current State
Currently the code for the Lolin-S2 dev board is working in CircuitPython. Windows will detect the device and register as a HID, with all button presses being correctly registered. The OpenVR driver will compile, be detected by SteamVR, but unfortunately no button presses are registered.