import hashlib
pw = "baguvix"
print(hashlib.md5(pw.encode()).hexdigest())

