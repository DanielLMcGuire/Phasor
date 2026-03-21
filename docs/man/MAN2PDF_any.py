#!/usr/bin/env python3
import subprocess
import os

subprocess.run([
    "docker", "run", "--rm",
    "-v", f"{os.getcwd()}:/data",
    "ubuntu", "bash", "-c",
    "apt-get update -qq && apt-get install -y -qq groff ghostscript && bash /data/MAN2PDF_linux.sh"
])