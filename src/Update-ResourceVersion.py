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


def dot_to_comma(version: str) -> str:
    return version.replace('.', ',')


def update_resource_version(old_version: str, new_version: str) -> None:
    app_dir = Path("./Executable")
    
    if not app_dir.exists():
        print(f"Error: Directory '{app_dir}' does not exist")
        sys.exit(1)
    
    old_version_comma = dot_to_comma(old_version)
    new_version_comma = dot_to_comma(new_version)
    
    old_version_string = f"{old_version}.0"
    new_version_string = f"{new_version}.0"
    
    old_version_comma_escaped = re.escape(old_version_comma)
    
    rc_files = list(app_dir.rglob("*.rc"))
    
    if not rc_files:
        print(f"No .rc files found in {app_dir}")
        return
    
    files_updated = 0
    for file_path in rc_files:
        try:
            lines = file_path.read_text(encoding='utf-8').splitlines(keepends=True)
            updated_lines = []
            file_modified = False
            
            for line in lines:
                new_line = line
                trimmed = line.lstrip()
                
                if trimmed.startswith("FILEVERSION") and re.search(old_version_comma_escaped, line):
                    new_line = re.sub(old_version_comma_escaped, new_version_comma, line)
                    file_modified = True

                elif trimmed.startswith("PRODUCTVERSION") and re.search(old_version_comma_escaped, line):
                    new_line = re.sub(old_version_comma_escaped, new_version_comma, line)
                    file_modified = True
                
                elif 'VALUE "FileVersion",' in line:
                    parts = line.split('"')
                    if len(parts) > 3 and parts[3] == old_version_string:
                        parts[3] = new_version_string
                        new_line = '"'.join(parts)
                        file_modified = True
                
                elif 'VALUE "ProductVersion",' in line:
                    parts = line.split('"')
                    if len(parts) > 3 and parts[3] == old_version_string:
                        parts[3] = new_version_string
                        new_line = '"'.join(parts)
                        file_modified = True
                
                updated_lines.append(new_line)
            
            if file_modified:
                file_path.write_text(''.join(updated_lines), encoding='utf-8')
                files_updated += 1
                
        except Exception as e:
            print(f"Error processing {file_path}: {e}")
    
    print(f"Updated .rc files from {old_version} to {new_version}")
    print(f"Files modified: {files_updated}/{len(rc_files)}")


def main():
    if len(sys.argv) != 3:
        print(f"Usage: python {Path(__file__).name} <old version> <new version>")
        sys.exit(1)
    
    old_version = sys.argv[1]
    new_version = sys.argv[2]
    
    update_resource_version(old_version, new_version)


if __name__ == "__main__":
    main()