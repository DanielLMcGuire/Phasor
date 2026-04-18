import json
import glob
import os
import hashlib

fileglobs = [
    "install/**/bin/*",
    "install/**/lib/*",
    "install/**/lib/**/*",
    "install/**/Library/**/*",
]

def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()

hashes = {}

for pattern in fileglobs:
    for path in glob.glob(pattern, recursive=True):
        if os.path.isfile(path):
            normalized = path.replace("\\", "/")
            if normalized not in hashes:
                hashes[normalized] = sha256_file(path)

files = [
    {"path": path, "sha256": digest}
    for path, digest in sorted(hashes.items())
]

data = {
    "os": "${{ matrix.os }}",
    "cc": "${{ matrix.c_compiler }}",
    "cxx": "${{ matrix.cpp_compiler }}",
    "attestation_url": "${{ steps.attest.outputs.attestation-url }}",
    "attestation_id": "${{ steps.attest.outputs.attestation-id }}",
    "commit": "${{ github.sha }}",
    "run_id": "${{ github.run_id }}",
    "files": files,
}

os.makedirs("install", exist_ok=True)

with open("install/attestations.json", "w") as f:
    json.dump(data, f, indent=2)