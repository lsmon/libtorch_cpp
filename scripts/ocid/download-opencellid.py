import os
import sys
import urllib.request
import gzip
import shutil

mcc_list = [310, 311, 312, 313, 314, 315, 316]  # List of MCCs to download, e.g., 310 for USA
def download_opencellid(root_path, token):
    url = "https://opencellid.org/ocid/downloads"
    url += f"?token={token}"
    
    if not os.path.exists(root_path):
        os.makedirs(root_path)
        
    for mcc in mcc_list:
        mcc_url = f"{url}&type=mcc&file={mcc}.csv.gz"
        print(f"Downloading OpenCellID data for MCC {mcc} from {mcc_url}...")
        try:
            tmp_zip = os.path.join(root_path, f"{mcc}.csv.gz")
            output_csv = os.path.join(root_path, f"{mcc}.csv")
            # Standardize headers to avoid 403 Forbidden on some CDNs
            opener = urllib.request.build_opener()
            opener.addheaders = [('User-agent', 'Mozilla/5.0')]
            urllib.request.install_opener(opener)
            urllib.request.urlretrieve(mcc_url, tmp_zip)
            print(f"Download completed for MCC {tmp_zip}.")
        except Exception as e:
            print(f"Download failed for MCC {mcc}: {e}")
            continue
        
        print(f"Extracting files for MCC {mcc}...")

        # 2. Check if it's actually a Gzip file by looking at the magic bytes
        local_filename = tmp_zip  # Use the same path as tmp_zip
        with open(tmp_zip, 'rb') as f:
            magic = f.read(2)
            if magic != b'\x1f\x8b':
                f.seek(0)
                print("Error: The downloaded file is not a Gzip file!")
                print("Content looks like:", f.read(100))
                return

        # 3. Decompress
        print(f"Decompressing... {local_filename} to {output_csv}")
        with gzip.open(local_filename, 'rb') as f_in:
            with open(output_csv, 'wb') as f_out:
                shutil.copyfileobj(f_in, f_out)  
                
        print(f"OpenCellID data for MCC {mcc} downloaded and extracted successfully.")
        os.remove(tmp_zip)
        data = f"{root_path}/data/ocid/{mcc}.csv"
        os.makedirs(os.path.dirname(data), exist_ok=True)
        shutil.move(output_csv, data)
        print(f"File moved to {data}.")

    print("All downloads completed.")
    
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 download-opencellid.py <root_path> <token>")
        print("Example: python3 download-opencellid.py ./data/opencellid YOUR_API_TOKEN")
    else:
        download_opencellid(sys.argv[1], sys.argv[2])
