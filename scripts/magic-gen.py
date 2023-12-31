#!/bin/env python3
import sys


# This program parses criu magic.h file and produces
# magic.py with all *_MAGIC constants except RAW and V1.
def main(argv):
    if len(argv) != 3:
        print("Usage: magic-gen.py path/to/image.h path/to/magic.py")
        exit(1)

    magic_c_header = argv[1]
    magic_py = argv[2]

    out = open(magic_py, 'w+')

    # all_magic is used to parse constructions like:
    # #define PAGEMAP_MAGIC          0x56084025
    # #define SHMEM_PAGEMAP_MAGIC    PAGEMAP_MAGIC
    all_magic = {}
    # and magic is used to store only unique magic.
    magic = {}

    f = open(magic_c_header, 'r')
    for line in f:
        split = line.split()

        if len(split) < 3:
            continue

        if not '#define' in split[0]:
            continue

        key = split[1]
        value = split[2]

        if value in all_magic:
            value = all_magic[value]
        else:
            magic[key] = value

        all_magic[key] = value

    out.write('#Autogenerated. Do not edit!\n')
    out.write('by_name = {}\n')
    out.write('by_val = {}\n')
    for k, v in list(magic.items()):
        # We don't need RAW or V1 magic, because
        # they can't be used to identify images.
        if v == '0x0' or v == '1' or k == '0x0' or v == '1':
            continue
        if k.endswith("_MAGIC"):
            # Just cutting _MAGIC suffix
            k = k[:-6]
        v = int(v, 16)
        out.write("by_name['" + k + "'] = " + str(v) + "\n")
        out.write("by_val[" + str(v) + "] = '" + k + "'\n")
    f.close()
    out.close()


if __name__ == "__main__":
    main(sys.argv)
