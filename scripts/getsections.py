# classes to access elf file
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import Section

# statically sets permissions for each section
def get_permissions(section_name):
    if section_name == ".text":
        #RX
        return "0x5" 
    elif section_name == ".data":
        #RW
        return "0x3"
    elif section_name == ".rodata":
        #R
        return "0x1"
    elif section_name == ".bss":
        #RW
        return "0x1"
    elif section_name == ".heap":
        #R
        return "0x1"
    elif section_name == ".stack":
        #RW
        return "0x3"
    else:
        return

# open elf file
appelf = ELFFile(open("build/app.elf", 'rb'))
# open output files
output = open('scripts/sectionsinfo.txt', 'w')

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
        section_permissions = get_permissions(sections[x])

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
