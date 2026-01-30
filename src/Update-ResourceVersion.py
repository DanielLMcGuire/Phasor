#!/usr/bin/env python3

import sys
import re
from pathlib import Path


def dot_to_comma(version: str) -> str:
    return version.replace('.', ',')


def update_resource_version(old_version: str, new_version: str) -> None:
    app_dir = Path("./App")
    
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
        print("Usage: python update_resource_version.py <old version> <new version>")
        sys.exit(1)
    
    old_version = sys.argv[1]
    new_version = sys.argv[2]
    
    update_resource_version(old_version, new_version)


if __name__ == "__main__":
    main()