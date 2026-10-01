# Australian UHF CB mode (experimental)

Open the menu and set **UHF CB** to ON. **CB CH** selects channels 1–80; up/down steps through them. Press * to start/stop scanning. **CB Dup** enables the +750 kHz repeater offset on channels 1–8 and 41–48. PTT transmits on the selected voice channel. The screen shows the CB channel, duplex indicator and operating frequency.

The profile forces narrow FM, disables scrambling, DTMF identification, companding, VOX and dual/cross-band operation. Channels 22, 23, 61, 62 and 63 remain selectable for reception but block transmission. Channels 5 and 35 are for emergency messages only.

Normal VFO state is restored when UHF CB is switched OFF. The CB profile does not write over stored channels. Mode, CB channel, duplex and CB power selection are temporary; power-on starts in normal Egzumer mode. Within CB mode only the UHF CB, CB CH, CB Dup, Sql and TxP menu settings are editable. Other features become available again after leaving CB mode. Side-button actions are limited to power, monitor, scan, flashlight and keypad lock.

Builds are produced by the Firmware GitHub Actions workflow, under the aus-uhf-cb-firmware artifact. This is an experimental firmware requiring hardware testing; a successful build cannot establish RF performance or legal equipment compliance.

Australian CB equipment must comply with ACMA technical standards. Programming these channels does not certify a UV-K6. RF power, deviation, occupied bandwidth and unwanted emissions require measurement before any on-air use. Use conducted tests into a suitable dummy load for initial validation.

References:
- https://www.acma.gov.au/licences/citizen-band-radio-stations-class-licence
- https://github.com/egzumer/uv-k5-firmware-custom

This build disables optional DTMF calling globally to fit the radio's 60 KB application flash. The spectrum analyser and FM broadcast receiver remain included. Squelch changes in CB mode are temporary and the original squelch setting is restored on exit. Keypad DTMF and 1750 Hz tones are disabled while transmitting in CB mode.
