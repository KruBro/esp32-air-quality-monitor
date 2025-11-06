# gen_token.py — generate a 32-byte (256-bit) token and write token.h
import os
token = os.urandom(32).hex()   # 64 hex chars
header = f"""// token.h - auto generated\n#pragma once\nstatic const char WS_BEARER_TOKEN[] = \"{token}\";\n"""
with open("token.h","w") as f:
    f.write(header)
print("Wrote token.h with token:", token)
