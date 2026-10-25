"""Print every Vulkan loader DLL visible to the Windows CI environment."""

import os
from pathlib import Path


def directories():
    seen = set()

    def add(value):
        if not value:
            return
        path = Path(str(value).strip('"'))
        try:
            key = str(path.resolve()).lower()
        except OSError:
            key = str(path).lower()
        if key not in seen:
            seen.add(key)
            yield path

    for entry in os.environ.get("PATH", "").split(os.pathsep):
        yield from add(entry)

    sdk = os.environ.get("VULKAN_SDK")
    if sdk:
        for relative in ("Bin", "bin", "runtime/x64", "runtime/x86"):
            yield from add(Path(sdk) / relative)

    system_root = os.environ.get("SystemRoot", r"C:\Windows")
    yield from add(Path(system_root) / "System32")
    yield from add(Path(system_root) / "SysWOW64")


print(f"VULKAN_SDK={os.environ.get('VULKAN_SDK', '<unset>')}")
print(f"SystemRoot={os.environ.get('SystemRoot', '<unset>')}")

found = []
for directory in directories():
    candidate = directory / "vulkan-1.dll"
    if candidate.is_file():
        found.append(candidate)
        print(f"FOUND: {candidate.resolve()} ({candidate.stat().st_size} bytes)")

if not found:
    print("NOT FOUND: vulkan-1.dll")
else:
    print(f"Found {len(found)} Vulkan loader location(s).")
