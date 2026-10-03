# Australian UHF CB mode (experimental)

Normal power-on starts in **UHF CB** mode on channel 1. Use **Mode** to choose CB, AIRCRAFT or SPECTRUM. See AIRCRAFT.md. **CB CH** selects channels 1â€“80; up/down steps through them. Press * to start/stop scanning. **CB Dup** enables the +750 kHz repeater offset on channels 1â€“8 and 41â€“48. PTT transmits on the selected voice channel. The screen shows one large CB channel label, without a frequency or duplicate VFO. TX/RX, power, narrow FM, duplex, scanning and transmit warnings remain visible.

The profile forces narrow FM, disables scrambling, DTMF identification, companding, VOX and dual/cross-band operation. Channels 22, 23, 61, 62 and 63 remain selectable for reception but block transmission. Channels 5 and 35 are for emergency messages only.

The simplified build does not expose normal VFO operation. The CB profile does not write over stored channels. Mode, CB channel, duplex and CB power selection are temporary; normal power-on always starts in CB mode on channel 1, with duplex off. Special maintenance boot modes remain unchanged. The simplified menu contains mode, CB/aircraft selection, squelch, power, scan resume, lighting, beep, lock, battery display and timeout controls. Settings are temporary. Upper side button: monitor; lower: flashlight.

Builds are produced by the Firmware GitHub Actions workflow, under the aus-uhf-cb-firmware artifact. This is an experimental firmware requiring hardware testing; a successful build cannot establish RF performance or legal equipment compliance.

Australian CB equipment must comply with ACMA technical standards. Programming these channels does not certify a UV-K6. RF power, deviation, occupied bandwidth and unwanted emissions require measurement before any on-air use. Use conducted tests into a suitable dummy load for initial validation.

References:
- https://www.acma.gov.au/licences/citizen-band-radio-stations-class-licence
- https://github.com/egzumer/uv-k5-firmware-custom

This build disables optional DTMF calling globally to fit the radio's 60 KB application flash. The spectrum analyser and AM aircraft receiver remain included; FM broadcast is removed. Squelch changes in CB mode are temporary and the original squelch setting is restored on exit. Keypad DTMF and 1750 Hz tones are disabled while transmitting in CB mode.

## Channel-use display

A short hint appears below the channel number. Power/NFM/duplex appear at the top; receive-only warnings and scanning remain on separate rows.

| Channels | Display |
| --- | --- |
| 5, 35 | EMERGENCY ONLY |
| 22, 23 | DATA ONLY (transmit remains blocked) |
| 61â€“63 | RESERVED (transmit remains blocked) |
| 1â€“8, 41â€“48 except 5 | REPEATER OUTPUT |
| 31â€“38, 71â€“78 except 35 | REPEATER INPUT |
| 10 | 4WD / CONVOY |
| 11 | CALLING |
| 18 | CARAVANS / CAMPERS |
| 29 | PACIFIC/BRUCE HWY |
| 40 | ROAD / TRUCKS |
| All others | GENERAL USE |

Road, 4WD and caravan descriptions are customary uses, not exclusive channel allocations. Listen before transmitting. Calling is for establishing contact, then move to a suitable free channel. Repeater labels describe the channel pair; enable CB Dup on the output channel when using a repeater. These display hints do not change any transmission restrictions.

Sources checked 3 October 2026:
- [ACMA CB class licence guidance](https://www.acma.gov.au/licences/citizen-band-radio-stations-class-licence)
- [Oricom channel-use chart](https://cdn.asp.events/CLIENT_Exhibiti_030C4FB2_97AC_2262_74166DECA5FD4BAB/sites/4x4-Outdoors-Show2021/media/libraries/brochures/12998-Oricom-UHF-Full-Range-Brochure-0524-Final-Proof.pdf)
- [GME channel restrictions](https://gme.net.au/app/uploads/XRS-390C_QSG.pdf)
