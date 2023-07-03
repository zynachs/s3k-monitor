# classes to access elf file
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import Section

# converts permissions format from elf to format for monitor
def convert_permissions(permissions_code):
    # R
    if permissions_code == "0x2":
        return "0x1"
    # W
    elif permissions_code == "0x1":
        return "0x2"
    # X
    elif permissions_code == "0x4":
        return "0x4"
    # RW
    elif permissions_code == "0x3":
        return "0x3"
    # RX
    elif permissions_code == "0x6":
        return "0x5"
    # WX
    elif permissions_code == "0x5":
        return "0x6"
    # RWX
    elif permissions_code == "0x7":
        return "0x7"
    # if permission code (sh_flag) is none of above, return 0
    else:
        return "0"

# open elf file
appelf = ELFFile(open("app.elf", 'rb'))
# open output file
output = open('sectionsinfo.txt', 'w')

# naming sections
text_name = ".text"
data_name = ".data"
rodata_name = ".rodata"
bss_name = ".bss"
heap_name = ".heap"
stack_name = ".stack"

sections = [text_name, data_name, rodata_name, bss_name, heap_name, stack_name]

# print name, address, size and permissions for all sections in list.
for x in range(len(sections)):
    
    # get section from section name
    section = appelf.get_section_by_name(sections[x])

    if isinstance(section, Section)==True:
        # get size of section
        section_size = hex(section.data_size)
        # get addr of section
        section_addr = hex(section.header.sh_addr)
        # get section persissions flag
        section_permissions = convert_permissions(hex(section.header.sh_flags))

        # convert name to hex
        sections[x] = sections[x].encode('utf-8')
        sections[x] = sections[x][::-1]
        sections[x] = sections[x].hex()

        # print to file
        print(sections[x], file = output)
        print(section_addr, file = output)
        print(section_size, file = output)
        print(section_permissions, file = output)

else:
    print('  The file has no %s section' % section)




