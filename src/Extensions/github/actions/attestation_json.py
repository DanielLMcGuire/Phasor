# Copyright 2026 Daniel McGuire
# Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
# Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http:#www.apache.org/licenses/LICENSE-2.0
# or https:#phasor.pages.dev/LICENSE.txt
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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