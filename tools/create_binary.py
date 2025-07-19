import struct
import sys
import os

def read_elf_header(data):
    # e_ident[16], e_type, e_machine, e_version, e_entry, e_phoff, e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx
    
    if len(data) < 52:
        raise ValueError("File too small to be elf")
    
    if data[0:4] != b'\x7fELF':
        raise ValueError("Not an elf file")
    
    header = struct.unpack('<16sHHIIIIIHHHHHH', data[0:52])
    
    return {
        'entry': header[4],
        'phoff': header[5],
        'shoff': header[6],
        'phnum': header[10],
        'shnum': header[12],
        'shstrndx': header[13]
    }

def read_program_headers(data, phoff, phnum):
    programs = []
    for i in range(phnum):
        offset = phoff + i * 32  # program header :3
        if offset + 32 > len(data):
            break
            
        ph = struct.unpack('<IIIIIIII', data[offset:offset+32])
        programs.append({
            'type': ph[0],
            'offset': ph[1],
            'vaddr': ph[2],
            'paddr': ph[3],
            'filesz': ph[4],
            'memsz': ph[5],
            'flags': ph[6],
            'align': ph[7]
        })
    
    return programs

def extract_code_section(data):
    """extract the executable code from the shit ass elf"""
    try:
        header = read_elf_header(data)
        programs = read_program_headers(data, header['phoff'], header['phnum'])
        
        for prog in programs:
            if prog['type'] == 1 and prog['flags'] & 1:
                start = prog['offset']
                size = prog['filesz']
                code_data = data[start:start + size]
                
                entry_offset = header['entry'] - prog['vaddr']
                
                print(f"Offset 0x{start:08X}")
                print(f"Virtual addr 0x{prog['vaddr']:08X}")
                print(f"Entry point 0x{header['entry']:08X}")
                print(f"Entry offset in segment 0x{entry_offset:08X}")
                
                return code_data, entry_offset
                
    except Exception as e:
        print(e)

    return data, 0

def create_binary(elf_file, output_file):
    
    with open(elf_file, 'rb') as f:
        elf_data = f.read()
        
    code_data = extract_code_section(elf_data)
    
    BINARY_MAGIC = 0x0042494E
    
    entry_point_offset = 0
    
    # make header
    header = struct.pack('<III', 
        BINARY_MAGIC,
        entry_point_offset,
        len(code_data)
    )
    
    with open(output_file, 'wb') as f:
        f.write(header)
        f.write(code_data)
    
    print(f"0x{BINARY_MAGIC:08X}")
    print(f"{entry_point_offset}")

def main():
    if len(sys.argv) != 3:
        print("python3 tools/create_binary.py input output")
        sys.exit(1)
    
    elf_file = sys.argv[1]
    output_file = sys.argv[2]
    
    create_binary(elf_file, output_file)

if __name__ == "__main__":
    main()