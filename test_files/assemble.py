import os
import sys
import inspect
import string

from qisa_as import QISA_Driver, qisa_qmap

currentdir = os.path.dirname(os.path.abspath(
                inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.append(currentdir)

from assemble_lib import *

# Argument check
CheckArgs(sys.argv)

rawinput = sys.argv[1]
# print("The QISA file name read from the argument is:", rawinput,  "\n")

CheckExtension(rawinput, [".qisa"])
CheckExistence(rawinput)

qisa_name = rawinput

hex_insn_fn = change_file_ext(qisa_name, '.dis.qisa')
bin_insn_fn = change_file_ext(qisa_name, '.bin')
linux_hex_insn_fn = get_linux_hex_file_ext(qisa_name, ".hex")
qmap_fn = get_qmap_in_same_dir(qisa_name)

# get hex format instructions from the assembly file.
# hex_instructions = GetHexInstructions(qisa_name)

print ("QISA_AS Version: ", QISA_Driver.getVersion())

driver = QISA_Driver()

driver.enableScannerTracing(False)
driver.enableParserTracing(False)
driver.setVerbose(False)

print ("Try loading quantum instructions from ", qmap_fn)
success = driver.loadQuantumInstructions(qmap_fn)
if not success:
    print ("Error: ", driver.getLastErrorMessage())
    print ("Failed to load quantum instructions from dictionaries. Abort.")
    exit()


print ("Sucessfully loaded quantum instruction specification.")

print ("parsing file ", qisa_name)
success = driver.assemble(qisa_name)
if not success:
  print ("Assembly terminated with errors:", driver.getLastErrorMessage())
  exit()

print ("Generated instructions:")
instHex = driver.getInstructionsAsHexStrings(False)
for inst in instHex:
  print ("  " + inst)
print()

print ("Saving instructions to file: ", bin_insn_fn)
success = driver.save(bin_insn_fn)

success = driver.disassemble(bin_insn_fn)
if not success:
  print ("Disassembly terminated with errors:")
  print (driver.getLastErrorMessage())
  exit()


print ("Saving disassembly to file: ", hex_insn_fn)
success = driver.save(hex_insn_fn)
