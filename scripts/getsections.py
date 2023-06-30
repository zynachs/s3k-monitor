# classes to access elf file
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import Section

# open elf file
appelf = ELFFile(open("build/app.elf", 'rb'))
# open output file
output = open('scripts/sectionsinfo.txt', 'w')


# naming sections
text_name = ".text"
data_name = ".data"
rodata_name = ".rodata"
bss_name = ".bss"
heap_name = ".heap"
stack_name = ".stack"


### .text ###
# get section from section name
text = appelf.get_section_by_name(text_name)

if isinstance(text, Section)==True:
    # get size of section
    text_size = hex(text.data_size)
    # get addr of section
    text_addr = hex(text.header.sh_addr)

    # convert name to hex
    text_name = text_name.encode('utf-8')
    text_name = text_name.hex()

    # print to file
    print(text_name, file = output)
    print(text_addr, file = output)
    print(text_size, file = output)

else:
    print('  The file has no %s section' % text_name)

### .data ###
# get section from section name
data = appelf.get_section_by_name(data_name)
if isinstance(data, Section)==True:
    # get size of section
    data_size = hex(data.data_size)
    # get addr of section
    data_addr = hex(data.header.sh_addr)

    # convert name to hex
    data_name = data_name.encode('utf-8')
    data_name = data_name.hex()

    # print to file
    print(data_name, file = output)
    print(data_addr, file = output)
    print(data_size, file = output)

else:
    print('  The file has no %s section' % data_name)

### .rodata ###
# get section from section name
rodata = appelf.get_section_by_name(rodata_name)
if isinstance(rodata, Section)==True:
    # get size of section
    rodata_size = hex(rodata.data_size)
    # get addr of section
    rodata_addr = hex(rodata.header.sh_addr)

    # convert name to hex
    rodata_name = rodata_name.encode('utf-8')
    rodata_name = rodata_name.hex()

    # print to file
    print(rodata_name, file = output)
    print(rodata_addr, file = output)
    print(rodata_size, file = output)

else:
    print('  The file has no %s section' % rodata_name)

### .bss ###
# get section from section name
bss = appelf.get_section_by_name(bss_name)
if isinstance(bss, Section)==True:
    # get size of section
    bss_size = hex(bss.data_size)
    # get addr of section
    bss_addr = hex(bss.header.sh_addr)

    # convert name to hex
    bss_name = bss_name.encode('utf-8')
    bss_name = bss_name.hex()

    # print to file
    print(bss_name, file = output)
    print(bss_addr, file = output)
    print(bss_size, file = output)

else:
    print('  The file has no %s section' % bss_name)

### .heap ###
# get section from section name
heap = appelf.get_section_by_name(heap_name)
if isinstance(heap, Section)==True:
    # get size of section
    heap_size = hex(heap.data_size)
    # get addr of section
    heap_addr = hex(heap.header.sh_addr)

    # convert name to hex
    heap_name = heap_name.encode('utf-8')
    heap_name = heap_name.hex()

    # print to file
    print(heap_name, file = output)
    print(heap_addr, file = output)
    print(heap_size, file = output)

else:
    print('  The file has no %s section' % heap_name)

### .stack ###
# get section from section name
stack = appelf.get_section_by_name(stack_name)
if isinstance(stack, Section)==True:
    # get size of section
    stack_size = hex(stack.data_size)
    # get addr of section
    stack_addr = hex(stack.header.sh_addr)

    # convert name to hex
    stack_name = stack_name.encode('utf-8')
    stack_name = stack_name.hex()

    # print to file
    print(stack_name, file = output)
    print(stack_addr, file = output)
    print(stack_size, file = output)

else:
    print('  The file has no %s section' % stack_name)




