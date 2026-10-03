"""Build compact aircraft data and test vectors from the public-domain AU catalogue."""
from pathlib import Path
import csv,json,unicodedata
from decimal import Decimal
root=Path(__file__).resolve().parents[1]
regions=['ACT','NSW','NT','QLD','SA','TAS','VIC','WA']
selected=set((root/'data/airports-selected.txt').read_text().split())
rows=list(csv.DictReader((root/'data/airports-au.csv').open(encoding='utf-8')))
services=[]; airports=[]; channels=[]; unique=[]; indices=[]
def ascii_text(s): return unicodedata.normalize('NFKD',s).encode('ascii','ignore').decode()
def display_text(s): return ''.join(c if 32<=ord(c)<=95 else '-' for c in ascii_text(s).upper())
grouped={}
for row in rows:
 if row['ident'] in selected: grouped.setdefault(row['ident'],[]).append(row)
for region in regions:
 for ident,rr in sorted(grouped.items(),key=lambda pair: (pair[1][0]['airport'].upper(),pair[0])):
  if rr[0]['state']!=region: continue
  name=display_text(rr[0]['airport'])
  for word in [' INTERNATIONAL',' AIRPORT',' AERODROME',' KINGSFORD SMITH']: name=name.replace(word,'')
  name=name[:16].rstrip()
  assert len(ident)<=8 and display_text(ident)==ident
  first=len(channels);seen=set()
  for r in rr:
   value=Decimal(r['frequency_mhz'])*100000
   assert value==int(value), 'Frequency exceeds firmware precision'
   frequency=int(value); assert 11800000<=frequency<13700000
   svc=ascii_text(r['service']).strip()[:8]
   if (frequency,svc) in seen: continue
   seen.add((frequency,svc))
   if svc not in services: services.append(svc)
   entry=(frequency,services.index(svc));channels.append(entry)
   if entry not in unique: unique.append(entry)
   indices.append(unique.index(entry))
  count=len(channels)-first;assert 0<count<256
  airports.append((name,ident,first,count,regions.index(region)))
assert selected==set(a[1] for a in airports), 'Unknown/empty airport selection'
assert all(any(a[4]==i for a in airports) for i in range(8)), 'Keep one airport per state/territory'
assert len(channels)<65536 and len(services)<256
def packed_text(name,ident):
 s=name.ljust(16)+ident.ljust(8);v=sum((ord(c)-32)<<(6*i) for i,c in enumerate(s))
 return ','.join(str(b) for b in v.to_bytes(18,'little'))
out='#include "app/aircraft.h"\n'
out+='const char *const gAirRegions[8] = {'+','.join(json.dumps(x) for x in regions)+'};\n'
out+='const char *const gAirServices[] = {'+','.join(json.dumps(x) for x in services)+'};\n'
out+='const AirAirportPacked gAirports[] = {\n'+''.join('{{%s},%d,%d,%d},\n'%(packed_text(n,i),f,c,r) for n,i,f,c,r in airports)+'};\n'
out+='const AirChannel gAirChannels[] = {\n'+''.join('{%du,%d},\n'%c for c in unique)+'};\n'
out+='const uint16_t gAirChannelIndex[] = {'+','.join(str(i) for i in indices)+'};\n'
out+='const uint16_t gAirportsCount = sizeof(gAirports)/sizeof(gAirports[0]);\n'
(root/'app/aircraft_data.c').write_text(out)
test='// Uncompressed generator expectations, linked into host tests only.\n'
test+='static const AirAirport expectedAirports[] = {\n'+''.join('{%s,%s,%d,%d,%d},\n'%(json.dumps(n),json.dumps(i),f,c,r) for n,i,f,c,r in airports)+'};\n'
test+='static const uint32_t expectedFrequencies[] = {'+','.join(str(c[0])+'u' for c in channels)+'};\n'
test+='static const uint8_t expectedServices[] = {'+','.join(str(c[1]) for c in channels)+'};\n'
(root/'tests/aircraft_expected.h').write_text(test)
print(f'{len(airports)} airports, {len(channels)} entries, {len(unique)} unique frequency/service pairs')
