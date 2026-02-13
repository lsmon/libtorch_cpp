import os
import sys
import platform
import urllib.request
import zipfile

def download_and_install(root_path, version='2.5.0'):
    os_type = sys.platform
    arch = platform.machine()

    if os_type == "linux":
        if arch in ['aarch64', 'arm64']:
            url = f"https://download.pytorch.org/whl/cpu/torch-{version}-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl"
        else:
            url = f"https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-{version}%2Bcpu.zip"
    else:
        print(f"Unsupported OS: {os_type}")
        sys.exit(1)

    if not os.path.exists(root_path):
        os.makedirs(root_path)
    
    tmp_zip = os.path.join(root_path, f"libtorch-{version}.zip")

    print(f"Downloading LibTorch {version} from {url}...")
    try:
        # Standardize headers to avoid 403 Forbidden on some CDNs
        opener = urllib.request.build_opener()
        opener.addheaders = [('User-agent', 'Mozilla/5.0')]
        urllib.request.install_opener(opener)
        urllib.request.urlretrieve(url, tmp_zip)
    except Exception as e:
        print(f"Download failed: {e}")
        sys.exit(1)
    
    print("Extracting files...")
    zipfile.ZipFile(tmp_zip, 'r').extractall(root_path)
    print(f"LibTorch {version} installed succesfully to {root_path}")
    os.remove(tmp_zip)

    
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 install-libtorch.py <root_path> <version>")
        print("Example: python3 install-libtorch.py ./external/libtorch 2.2.1")
    else:
        # Strip 'v' prefix if provided (e.g., v2.2.1 -> 2.2.1)
        ver = sys.argv[2].lstrip('v') 
        download_and_install(sys.argv[1], ver)