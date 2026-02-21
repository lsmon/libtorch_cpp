import os
import sys
import platform
import urllib.request
import zipfile

def download_and_install(root_path, version='2.5.0', cuda_version='121'):
    os_type = sys.platform
    arch = platform.machine()

    # Note: 'cpu' can be passed as cuda_version to get the CPU-only build
    if os_type == "linux":
        if arch in ['aarch64', 'arm64']:
            # ARM64 usually uses wheels or specific Jetpack builds; 
            # PyTorch doesn't provide standard GPU .zip for ARM64 on this CDN.
            url = f"https://download.pytorch.org/whl/cpu/torch-{version}-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl"
        else:
            # x86_64 logic
            if cuda_version == 'cpu':
                url = f"https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-{version}%2Bcpu.zip"
            else:
                # Format: cu118, cu121, cu124, etc.
                cuda_tag = f"cu{cuda_version.replace('.', '')}"
                url = f"https://download.pytorch.org/libtorch/{cuda_tag}/libtorch-cxx11-abi-shared-with-deps-{version}%2B{cuda_tag}.zip"
    else:
        print(f"Unsupported OS: {os_type}")
        sys.exit(1)

    if not os.path.exists(root_path):
        os.makedirs(root_path)
    
    tmp_zip = os.path.join(root_path, f"libtorch-{version}.zip")

    print(f"Downloading LibTorch {version} (CUDA {cuda_version}) from {url}...")
    try:
        opener = urllib.request.build_opener()
        opener.addheaders = [('User-agent', 'Mozilla/5.0')]
        urllib.request.install_opener(opener)
        urllib.request.urlretrieve(url, tmp_zip)
    except Exception as e:
        print(f"Download failed: {e}")
        print("Check if the version/CUDA combination exists at download.pytorch.org")
        sys.exit(1)
    
    print("Extracting files...")
    with zipfile.ZipFile(tmp_zip, 'r') as zip_ref:
        zip_ref.extractall(root_path)
    
    print(f"LibTorch {version} installed successfully to {root_path}")
    os.remove(tmp_zip)

if __name__ == "__main__":
    # Usage: python3 script.py <path> <version> <cuda_ver>
    # Example: python3 script.py ./libtorch 2.5.0 121
    if len(sys.argv) < 3:
        print("Usage: python3 install-libtorch.py <root_path> <version> [cuda_version]")
        print("Example: python3 install-libtorch.py ./external/libtorch 2.5.0 121")
        print("Use 'cpu' as cuda_version for non-GPU builds.")
    else:
        ver = sys.argv[2].lstrip('v')
        cuda_ver = sys.argv[3] if len(sys.argv) > 3 else '121'
        download_and_install(sys.argv[1], ver, cuda_ver)
