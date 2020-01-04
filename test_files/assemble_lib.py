import os
import sys
import inspect
import string

from qisa_as import QISA_Driver, qisa_qmap

def Conv2Addr(addr):
    if (not is_number(addr)):
        raise ValueError("Conv2Addr: Input parameter is not a number.")

    return "%4x:" % (addr)

def CheckExistence(input_asm_name):
    if not os.path.isfile(input_asm_name):
        print("\tError! The file %s does not exist" % input_asm_name)
        exit(0)

def CheckExtension(input_asm_name, target_ext):
    if type(target_ext) is not list:
        raise ValueError("The parameter target_ext should be a list.")

    if len(target_ext) < 1:
        raise ValueError("Empty extension list is given.")

    ExtCheck = False
    for ext in target_ext:
        if len(input_asm_name) < len(ext):
            continue
        if (input_asm_name[-len(ext):] == ext):
            ExtCheck = True
            break

    if (not ExtCheck):
        print("\t Error! The input asm file should have the extension:")
        for ext in target_ext:
            print("\t", ext)
        exit(0)

def CheckArgs(args):
    # print('Number of arguments:    ', len(args))
    # print('Argument List:')
    # for arg in args:
    #     print("    ", arg)
    # print()

    if len(args) != 2:
        print("Error: Only one QISA file name argument is required.")
        exit(0)

def filename_operation(qumis_name):
    pathname = os.path.dirname(qumis_name)
    base_name = os.path.splitext(os.path.basename(qumis_name))[0]
    init_mem_name = pathname + "\\Init_ICache_" + base_name + '.mem'
    init_mem_do_name = pathname + "\\InitMem.do"
    return (init_mem_name, init_mem_do_name)

def change_file_ext(qumis_name, ext):
    pathname = os.path.dirname(qumis_name)
    base_name = os.path.splitext(os.path.basename(qumis_name))[0]
    fn = os.path.join(pathname, base_name + ext)
    return fn

def get_qmap_in_same_dir(qisa_name, qisa_opcode_fn = ""):
    if qisa_opcode_fn is None or len(qisa_opcode_fn) == 0:
        qisa_opcode_fn = "qisa_opcodes.qmap"

    pathname = os.path.dirname(qisa_name)

    fn = os.path.join(pathname, qisa_opcode_fn)

    return fn

def get_linux_hex_file_ext(qumis_name, ext):
    pathname = r"D:/Projects/Share/CCLight_Linux/src/arm/cclight"
    base_name = os.path.splitext(os.path.basename(qumis_name))[0]
    fn = os.path.join(pathname, base_name + ext)
    return fn

# Assemble
def GetHexInstructions(qisa_file_name):
    print ("QISA_AS Version: ", QISA_Driver.getVersion())

    driver = QISA_Driver()

    driver.enableScannerTracing(False)
    driver.enableParserTracing(False)
    driver.setVerbose(True)

    print ("parsing file ", qisa_file_name)
    success = driver.assemble(qisa_file_name)

    if success:
        print ("Generated assembly:")
        print (driver.getInstructionsAsHexStrings(False))

        # print ("Saving instructions to file: ", outputFilename)
        # driver.save(outputFilename)

    else:
        print ("Assembly terminated with errors:")
        print (driver.getLastErrorMessage())

    return driver.getInstructionsAsHexStrings(False)

def print_instr(instr_name, instr, ln=False):
    if instr == []:
        raise ValueError("The instruction {} is empty.".format(instr_name))

    print(type(instr))
    print(len(instr))
    print(instr)
    # print("{}:".format(instr_name))
    # if ln is True:
    #     for l, i in enumerate(instr):
    #         print("{:<6d}: {}".format(l, i))
    # else:
    #     for i in instr:
    #         print("      {}".format(i))

# Append Nop instructions to fill the init_mem file.
def AppendHexNops(hex_instructions):
    while len(hex_instructions) < pow(2, 13):
        hex_instructions.append(bin_to_hex("0", 8))
    assert((len(hex_instructions) % 4) == 0)

    return hex_instructions

def Write_init_mem(init_mem_name, InitICacheMemheader, hex_instructions):
    try:
        init_mem_file = open(init_mem_name, 'w')
    except:
        print("Error: fail to open file " + init_mem_name + ".")

    init_mem_file.write(InitICacheMemheader)

    for addr in range(len(hex_instructions) - 1, -1, -1):
        if (addr % 4) == 3:
            init_mem_file.write(Conv2Addr(addr))
        init_mem_file.write(" ")
        init_mem_file.write(hex_instructions[addr])

        if (addr % 4) == 0:
            init_mem_file.write("\n")

    init_mem_file.close()

def Write_init_mem_do(init_mem_do_name, InitMemDoHeader, init_mem_name, InitMemDoMemName):
    # print("init_mem_do_name: ", init_mem_do_name)
    try:
        init_mem_do_file = open(init_mem_do_name, 'w')
    except:
        print("Error: fail to open file " + init_mem_do_name + ".")
    init_mem_do_file.write(InitMemDoHeader)
    init_mem_do_file.write(os.path.basename(init_mem_name))
    init_mem_do_file.write(" ")
    init_mem_do_file.write(InitMemDoMemName)
    init_mem_do_file.close()
