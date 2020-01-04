import hashlib
from pycqed.instrument_drivers.physical_instruments._CCL.CCLightMicrocode import CCLightMicrocode

def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()


mc = CCLightMicrocode()

mc_fn = r"cs_test.txt"
mc.load_microcode(mc_fn)

f = open("ControlStore.txt", "w")

hash_value = md5(mc_fn)

f.write("# md5 of original cs_test.txt: {:s}\n".format(hash_value))

for i in mc.microcode:
    f.write("%08x" % i)
    f.write("\n")

f.close()

print("Successfully wrote hex into ControlStore.txt")

mc.write_to_bin("ControlStore.bin")

print("Successfully wrote bin into ControlStore.bin")
