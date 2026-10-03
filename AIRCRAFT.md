# CB / Aircraft / Spectrum firmware

Menu > Mode selects CB, AIRCRAFT or SPECTRUM. Normal startup remains CB channel 1.

## Aircraft listening

Select Mode > AIRCRAFT, then State, Airport and Air Ch. Up/down steps services for the selected airport. The display shows the airport name/code, service abbreviation and frequency. Press * to scan the selected airport. PTT is blocked in aircraft mode, even if the modulation is changed internally. This is a listening receiver, not aviation communications equipment.

Mode > SPECTRUM opens the existing spectrum analyser around the current CB/aircraft frequency. EXIT returns through its existing spectrum screens; exiting the analyser restores the selected CB/aircraft channel. PTT in the analyser selects a signal for listening; it does not transmit.

The upper side button toggles monitor, and the lower toggles the flashlight. Normal VFO/memory menus, FM broadcast, VOX, DTMF calling, general scan ranges, channel copying and wide receive are removed from the default interface/build. AM reception correction, spectrum and programming-cable support remain. CB/aircraft selections and menu changes are temporary; restart restores CB channel 1. Stored memories and calibration are not overwritten.

## Australian catalogue

The included data/airports-au.csv has 3,601 entries for 1,440 non-closed Australian airports/airfields with listed frequencies in 118–137 MHz, sourced from the public-domain OurAirports download on 3 October 2026. It includes state, airport name/code, coordinates, service, description and frequency. All entries are marked unverified. This is nationwide source coverage, not a claim that every Australian frequency is present or current. Coordinates describe the airport, not reception coverage. En-route sectors are not comprehensively represented.

The default onboard selection has 25 airports and 126 frequency entries across ACT, NSW, NT, QLD, SA, TAS, VIC and WA. It is a subset, not the entire nationwide catalogue. Selection: Canberra, Sydney/Bankstown/Camden/Newcastle, Darwin/Alice Springs, Brisbane/Gold Coast/Cairns/Rockhampton/Sunshine Coast/Townsville, Adelaide/Parafield, Hobart/Launceston, Melbourne/Avalon/Essendon/Moorabbin, Perth/Jandakot/Broome/Port Hedland.

For a custom firmware list, edit data/airports-selected.txt with identifiers from the CSV, run `python tools/build_air_catalogue.py`, and rebuild. Keep at least one airport for each state/territory; the generator rejects unknown/empty airports and checks bounds. The linker enforces the radio's flash capacity. This is a build-time customisation, not an on-radio database upload.

Source: https://ourairports.com/data/ (public domain, no accuracy guarantee). Verify local listening frequencies against current authoritative information. No nationwide ERSA publication has been copied into this firmware. Do not use this catalogue for flight operations.
