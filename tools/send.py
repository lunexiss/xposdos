import struct
import sys
import os
from shutil import copyfile

MAX_FS_FILES = 64
ENTRY_SIZE = 74 
FS_TABLE_SECTOR = 2
SECTOR_SIZE = 512
TABLE_OFFSET = FS_TABLE_SECTOR * SECTOR_SIZE
FS_START_SECTOR = 100

def write_binary_safely(img_path, file_path, folder=""):
    temp_img = img_path + ".tmp"
    
    try:
        if not os.path.exists(img_path):
            raise RuntimeError(f"Disk image {img_path} not found")
        if not os.path.exists(file_path):
            raise RuntimeError(f"Source file {file_path} not found")

        copyfile(img_path, temp_img)
        
        with open(temp_img, "r+b") as f:
            used_sectors = set()
            f.seek(TABLE_OFFSET)
            for _ in range(MAX_FS_FILES):
                entry = f.read(ENTRY_SIZE)
                if len(entry) < ENTRY_SIZE:
                    break
                try:
                    used, _, _, size, start_sector = struct.unpack("<i32s32sIH", entry)
                    if used:
                        used_sectors.add(start_sector)
                except struct.error:
                    continue

            with open(file_path, "rb") as src_file:
                file_data = src_file.read()
            file_size = len(file_data)
            
            if file_size == 0:
                raise RuntimeError("Empty source file")
                
            required_sectors = (file_size + SECTOR_SIZE - 1) // SECTOR_SIZE
            
            start_sector = None
            consecutive_free = 0
            current_sector = FS_START_SECTOR
            
            while current_sector < FS_START_SECTOR + 1024:
                f.seek(current_sector * SECTOR_SIZE)
                try:
                    sector_data = f.read(SECTOR_SIZE)
                    if len(sector_data) != SECTOR_SIZE:
                        break
                        
                    if all(b == 0 for b in sector_data) and current_sector not in used_sectors:
                        if start_sector is None:
                            start_sector = current_sector
                        consecutive_free += 1
                        if consecutive_free >= required_sectors:
                            break
                    else:
                        start_sector = None
                        consecutive_free = 0
                except:
                    break
                
                current_sector += 1
            else:
                raise RuntimeError("Not enough contiguous free space")
            
            if start_sector is None:
                raise RuntimeError("Failed to find suitable sector")
            
            f.seek(TABLE_OFFSET)
            entry_pos = None
            for i in range(MAX_FS_FILES):
                try:
                    entry = f.read(ENTRY_SIZE)
                    if len(entry) < 4:
                        break
                    used = struct.unpack("<i", entry[:4])[0]
                    if not used:
                        entry_pos = i
                        break
                except struct.error:
                    continue

            if entry_pos is None:
                raise RuntimeError("No free file entries available")
            
            try:
                filename = os.path.basename(file_path).encode('ascii').ljust(32, b'\x00')[:32]
                folder_encoded = folder.encode('ascii').ljust(32, b'\x00')[:32]
                entry = struct.pack("<i32s32sIH",
                    1,  # used shi
                    filename,
                    folder_encoded,
                    file_size,
                    start_sector)
            except struct.error as e:
                raise RuntimeError(f"Failed to pack file entry {str(e)}")
            
            try:
                f.seek(start_sector * SECTOR_SIZE)
                bytes_written = f.write(file_data)
                if bytes_written != file_size:
                    raise RuntimeError("Failed to write complete file")
                
                padding = (required_sectors * SECTOR_SIZE) - file_size
                if padding > 0:
                    f.write(b'\x00' * padding)
                
                f.seek(TABLE_OFFSET + entry_pos * ENTRY_SIZE)
                f.write(entry)
                
                f.seek(start_sector * SECTOR_SIZE)
                written_data = f.read(file_size)
                if written_data != file_data:
                    raise RuntimeError("Verification failed data mismatch")
                
            except IOError as e:
                raise RuntimeError(f"Disk write fsiled {str(e)}")
            
        os.replace(temp_img, img_path)
        print(f"Successfully wrote {os.path.basename(file_path)} to sector {start_sector}")
        return True
        
    except Exception as e:
        print(f"Error: {str(e)}")
        if os.path.exists(temp_img):
            try:
                os.remove(temp_img)
            except:
                pass
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("python send.py disk file folder (optional idk)")
        sys.exit(1)
    
    success = write_binary_safely(
        sys.argv[1], 
        sys.argv[2], 
        sys.argv[3] if len(sys.argv) > 3 else ""
    )
    sys.exit(0 if success else 1)