"""Generate a compact onboard airport subset from the nationwide CSV.
Edit data/airports-selected.txt (airport identifiers), run this, then rebuild.
Source is public-domain OurAirports data, not an operationally verified database.
"""
from pathlib import Path
import csv,json,unicodedata
from decimal import Decimal
root=Path(__file__).resolve().parents[1]
regions=['ACT','NSW','NT','QLD','SA','TAS','VIC','WA']
selected=set((root/'data/airports-selected.txt').read_text().split())
rows=list(csv.DictReader((root/'data/airports-au.csv').open(encoding='utf-8')))
services=[]; airports=[]; channels=[]
def ascii_text(s): return unicodedata.normalize('NFKD',s).encode('ascii','ignore').decode()
for region in regions:
 for ident in sorted(selected):
  rr=[r for r in rows if r['ident']==ident and r['state']==region]
  if not rr: continue
  name=ascii_text(rr[0]['airport']).upper()
  for word in [' INTERNATIONAL',' AIRPORT',' AERODROME',' KINGSFORD SMITH']: name=name.replace(word,'')
  first=len(channels); seen=set()
  for r in rr:
   frequency=int(Decimal(r['frequency_mhz'])*100000)
   assert 11800000<=frequency<13700000
   svc=ascii_text(r['service'])[:8]
   if (frequency,svc) in seen: continue
   seen.add((frequency,svc))
   if svc not in services: services.append(svc)
   channels.append((frequency,services.index(svc)))
  count=len(channels)-first
  assert 0<count<256
  airports.append((name[:16],ident,first,count,regions.index(region)))
assert selected==set(a[1] for a in airports), 'Unknown or empty airport selection'
assert all(any(a[4]==i for a in airports) for i in range(8)), 'Select at least one airport per state/territory'
assert len(channels)<65536 and len(services)<256
out='#include "app/aircraft.h"\n'
out+='const char *const gAirRegions[8] = {'+','.join(json.dumps(x) for x in regions)+'};\n'
out+='const char *const gAirServices[] = {'+','.join(json.dumps(x) for x in services)+'};\n'
out+='const AirAirport gAirports[] = {\n'+''.join('{%s,%s,%d,%d,%d},\n'%(json.dumps(n),json.dumps(i),f,c,r) for n,i,f,c,r in airports)+'};\n'
out+='const AirChannel gAirChannels[] = {\n'+''.join('{%du,%d},\n'%c for c in channels)+'};\n'
out+='const uint16_t gAirportsCount = sizeof(gAirports)/sizeof(gAirports[0]);\n'
(root/'app/aircraft_data.c').write_text(out)
print(f'{len(airports)} airports, {len(channels)} frequencies, {len(rows)} nationwide catalogue rows')
