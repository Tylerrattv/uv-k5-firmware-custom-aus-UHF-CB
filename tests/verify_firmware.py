"""Verify the built flash image and the upstream packed format."""
import ast
import binascii
import hashlib
import pathlib
import struct

raw = pathlib.Path("firmware.bin").read_bytes()
packed = pathlib.Path("firmware.packed.bin").read_bytes()
assert 0x2000 <= len(raw) <= 60 * 1024, len(raw)
sp, reset = struct.unpack_from("<II", raw)
assert 0x20000000 < sp <= 0x20004000 and sp % 8 == 0, hex(sp)
assert reset & 1 and (reset & ~1) < len(raw), hex(reset)
assert len(packed) == len(raw) + 18
assert binascii.crc_hqx(packed[:-2], 0) == int.from_bytes(packed[-2:], "little")
tree = ast.parse(pathlib.Path("fw-pack.py").read_text())
key = next(ast.literal_eval(n.value) for n in tree.body if isinstance(n, ast.Assign)
           and any(isinstance(t, ast.Name) and t.id == "OBFUSCATION" for t in n.targets))
plain = bytes(value ^ key[i % len(key)] for i, value in enumerate(packed[:-2]))
assert plain[:0x2000] + plain[0x2010:] == raw
assert plain[0x2000:0x2010].startswith(b"*AUSCB ")
print(f"Verified vectors, 60 KB limit, packed CRC and payload: {len(raw)} raw bytes, {len(packed)} packed bytes")
print("SHA256", hashlib.sha256(packed).hexdigest())
