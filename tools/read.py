import struct

import sys


MAX_FS_FILES = 64

ENTRY_SIZE = 74

FS_TABLE_SECTOR = 2

SECTOR_SIZE = 512

TABLE_OFFSET = FS_TABLE_SECTOR * SECTOR_SIZE


class FileEntry:

    def __init__(self, used, name, folder, size, start_sector):

        self.used = used

        self.name = name.decode('utf-8', errors='replace').rstrip('\x00')

        self.folder = folder.decode('utf-8', errors='replace').rstrip('\x00')

        self.size = size

        self.start_sector = start_sector


    def is_folder(self):

        return self.start_sector == 0xFFFF


    def full_path(self):

        return f"{self.folder}/{self.name}" if self.folder else self.name


    def __repr__(self):

        return f"{self.name} | {self.folder} | {self.size} bytes @ sector {self.start_sector}"


def read_file_table(img_file):

    entries = []

    with open(img_file, "rb") as f:

        f.seek(TABLE_OFFSET)

        for _ in range(MAX_FS_FILES):

            raw = f.read(ENTRY_SIZE)

            if len(raw) < ENTRY_SIZE:

                break

            unpacked = struct.unpack("<I32s32sIH", raw)

            entry = FileEntry(*unpacked)

            if entry.used and not entry.is_folder():

                entries.append(entry)

    return entries


def read_file_data(img_file, entry):

    with open(img_file, "rb") as f:

        f.seek(entry.start_sector * SECTOR_SIZE)

        data = f.read(entry.size)

        return data.decode('utf-8', errors='replace')


if __name__ == "__main__":

    if len(sys.argv) < 2:

        print("python tools/read.py disk shit")

        sys.exit(1)


    img = sys.argv[1]

    files = read_file_table(img)


    for file in files:

        print(f"== {file.full_path()} ==")

        print(read_file_data(img, file))

        print("-" * 40) 