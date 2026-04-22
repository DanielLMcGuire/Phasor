#!/usr/bin/env python3
import subprocess
import os
import sys

if subprocess.run(["docker", "--version"], capture_output=True).returncode != 0:
    print("Error: Docker is not installed. Please install it from https://docker.com/desktop", file=sys.stderr)
    sys.exit(1)

subprocess.run([
    "docker", "run", "--rm",
    "-v", f"{os.getcwd()}:/data",
    "ubuntu", "bash", "-c",
    "apt-get update -qq && apt-get install -y -qq groff ghostscript && bash /data/MAN2PDF_linux.sh"
])
