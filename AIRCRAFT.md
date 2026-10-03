# CB / Aircraft firmware

Menu > Mode selects CB or AIRCRAFT. Normal startup remains CB channel 1.

## Aircraft listening

Select Mode > AIRCRAFT, then State, Airport and Air Ch. Up/down steps services for the selected airport. The display shows the airport name/code, service abbreviation and frequency. Press * to scan the selected airport. PTT is blocked in aircraft mode, even if the modulation is changed internally. This is a listening receiver, not aviation communications equipment.


The upper side button toggles monitor, and the lower toggles the flashlight. Normal VFO/memory menus, FM broadcast, VOX, DTMF calling, general scan ranges, channel copying and wide receive are removed from the default interface/build. AM reception correction and programming-cable support remain. CB/aircraft selections and menu changes are temporary; restart restores CB channel 1. Stored memories and calibration are not overwritten.

## Australian catalogue

The included data/airports-au.csv has 3,601 entries for 1,440 non-closed Australian airports/airfields with listed frequencies in 118–137 MHz, sourced from the public-domain OurAirports download on 3 October 2026. It includes state, airport name/code, coordinates, service, description and frequency. All entries are marked unverified. This is nationwide source coverage, not a claim that every Australian frequency is present or current. Coordinates describe the airport, not reception coverage. En-route sectors are not comprehensively represented.

The onboard selection has **340 airports and 895 frequency/service entries**: all **196 NSW** and **127 VIC** airports/airfields with usable 118–137 MHz entries in the bundled catalogue, plus **17 major airports elsewhere**. Airports absent from this source, marked closed, or without usable listed frequencies are not included. This is catalogue coverage, not a guarantee of every operating airfield or current frequency.

Other-state/territory airports: Canberra; Darwin and Alice Springs; Brisbane, Gold Coast, Cairns, Rockhampton, Sunshine Coast and Townsville; Adelaide and Parafield; Hobart and Launceston; Perth, Jandakot, Broome and Port Hedland.

Airports are sorted alphabetically by name within each state. The Airprt menu shows both halves of the 16-character airport name and its identifier. Long names are abbreviated to 16 characters; the identifier distinguishes similarly named airports.

The firmware stores airport names in a six-bit character format and shares 271 unique frequency/service pairs encoded on an exact 500 Hz grid across the 895 entries. All entries remain individually selectable per airport. The generator emits uncompressed host-test expectations; tests check every decoded name, identifier, frequency and service. The unused legacy VFO display/keypad paths were removed to make room; CB and Aircraft operation are retained. The spectrum analyser is disabled in this build and removed from the mode menu.

For a custom firmware list, edit data/airports-selected.txt with identifiers from the CSV, run `python tools/build_air_catalogue.py`, and rebuild. Keep at least one airport for each state/territory; the generator rejects unknown/empty airports and checks bounds. The generator rejects frequencies that cannot be represented exactly on the 500 Hz storage grid; it never rounds them. The linker enforces the radio's flash capacity. This is a build-time customisation, not an on-radio database upload.

Source: https://ourairports.com/data/ (public domain, no accuracy guarantee). Verify local listening frequencies against current authoritative information. No nationwide ERSA publication has been copied into this firmware. Do not use this catalogue for flight operations.
