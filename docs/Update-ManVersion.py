#!/usr/bin/env python3

# Copyright 2025-2026 Daniel McGuire
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

import sys
import re
from pathlib import Path


def update_man_version(old_version: str, new_version: str) -> None:
    man_dir = Path("./man")
    
    if not man_dir.exists():
        print(f"Error: Directory '{man_dir}' does not exist")
        sys.exit(1)
    
    man_extensions = {".1", ".3", ".5", ".7"}
    man_files = [
        f for f in man_dir.rglob("*") 
        if f.is_file() and f.suffix in man_extensions
    ]
    
    if not man_files:
        print(f"No man page files found in {man_dir}")
        return
    
    old_version_escaped = re.escape(old_version)
    
    files_updated = 0
    for file_path in man_files:
        try:
            content = file_path.read_text(encoding='utf-8')
            updated_content = re.sub(old_version_escaped, new_version, content)
            
            if updated_content != content:
                file_path.write_text(updated_content, encoding='utf-8')
                files_updated += 1
                
        except Exception as e:
            print(f"Error processing {file_path}: {e}")
    
    print(f"Updated all man pages from {old_version} to {new_version}")
    print(f"Files modified: {files_updated}/{len(man_files)}")


def main():
    if len(sys.argv) != 3:
        print(f"Usage: python {Path(__file__).name} <old version> <new version>")
        sys.exit(1)
    
    old_version = sys.argv[1]
    new_version = sys.argv[2]
    
    update_man_version(old_version, new_version)


if __name__ == "__main__":
    main()