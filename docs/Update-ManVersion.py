#!/usr/bin/env python3

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