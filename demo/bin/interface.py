import numpy as np
import logging
from quantumsim.sparsedm import *
from quantumsim.circuit import *
from quantumsim.ptm import *
import random
# import sys

# FORMATTER = logging.Formatter("%(asctime)s — %(name)s — %(levelname)s — %(message)s")

# def get_console_handler():
#   console_handler = logging.StreamHandler(sys.stdout)
#   console_handler.setFormatter(FORMATTER)
#   return console_handler

log = logging.getLogger(__name__)

# log.setLevel(logging.DEBUG)
# log.addHandler(get_console_handler())
#log.propagate = False

round_precision = 4


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


class interface_quantumsim:
    """
    This is the class used to interact with the kernel of QuantumSim.
    """

    # Allowed exception levels:
    # CRITICAL	50
    # ERROR	    40
    # WARNING	30
    # INFO	    20
    # DEBUG	    10
    # NOTSET	0
    exceptionLevel = logging.DEBUG

    def __init__(self):
        self.num_qubit = 0
        self.sdm = None
        self.ptm = None
        self.t1 = np.inf
        self.t2 = np.inf
        self.gamma = None
        self.lamda = None

        # default sampler
        self.sampler = uniform_noisy_sampler(readout_error=0.03, seed=42)
        self.sampler.send(None)
        self.readout_error = 0  # 0.03
        self.seed = 42

        # The measurement results recording dict
        self.measurements = {}
        self.current_measurement = None

        self.error_on = True

    def init_dm(self, num_qubit):

        self.num_qubit = num_qubit

        qubit_names = []
        for i in range(0, num_qubit):
            qubit_names.append(str(i))

        self.sdm = SparseDM(qubit_names)

        for i in range(0, num_qubit):
            self.measurements[str(i)] = []

        log.info("QuantumSim: the density matrix has been initialized successfully.")

    def extract_angle_from_op_name(self, name):
        pass

    def prepare_rotation_ptm(self, axis, angle):
        """
        This function prepares a Pauli Transfer Matrix for a single-qubit rotation.
        Input parameters:
            axis:   the rotation axis, which can be 'x', 'y', or 'z'
            angle:  the rotation angle in degree, can be int or float type

        TODO: this function is used to replace the function prepare_ptm()
        """
        axis = axis.lower()
        assert(axis in ['x', 'y', 'z'])
        assert(is_number(angle))

        if axis == 'x':
            self.ptm = rotate_x_ptm(angle*np.pi/180)
        elif axis == 'y':
            self.ptm = rotate_y_ptm(angle*np.pi/180)
        elif axis == 'z':
            self.ptm = rotate_y_ptm(angle*np.pi/180)
        else:
            log.error("QuantumSim: undefined axis found: {}".format(axis))

    def prepare_ptm(self, quantum_operation):
        angle = 0
        # gates of eQASM use '_' to replace decimal point, convert it back now
        quantum_operation = quantum_operation.lower().replace('_', '.').strip('r')
        log.debug("QuantumSim: prepare a PTM for operation %s",
                  quantum_operation)

        if quantum_operation == "h":
            self.ptm = hadamard_ptm()
        elif quantum_operation == "x":
            self.ptm = rotate_x_ptm(np.pi)
        elif quantum_operation == "y":
            self.ptm = rotate_y_ptm(np.pi)
        elif quantum_operation == "z":
            self.ptm = rotate_z_ptm(np.pi)
        elif quantum_operation == "s":
            self.ptm = rotate_z_ptm(np.pi/2)
        elif quantum_operation == "t":
            self.ptm = rotate_z_ptm(np.pi/4)
        elif quantum_operation == "sdg":
            self.ptm = rotate_z_ptm(-np.pi/2)
        elif quantum_operation == "tdg":
            self.ptm = rotate_z_ptm(-np.pi/4)
        elif "x" in quantum_operation:
            if "m" in quantum_operation.split("x")[1]:
                angle = -float(quantum_operation.split("xm")[1])
            else:
                angle = float(quantum_operation.split("x")[1])
            self.ptm = rotate_x_ptm((angle*np.pi)/180)
        elif "y" in quantum_operation:
            if "m" in quantum_operation.split("y")[1]:
                angle = -float(quantum_operation.split("ym")[1])
            else:
                angle = float(quantum_operation.split("y")[1])
            self.ptm = rotate_y_ptm((angle*np.pi)/180)
        elif "z" in quantum_operation:
            if "m" in quantum_operation.split("z")[1]:
                angle = -float(quantum_operation.split("zm")[1])
            else:
                angle = float(quantum_operation.split("z")[1])
            self.ptm = rotate_z_ptm((angle*np.pi)/180)
        else:
            self.ptm = []

        #log.debug(self.ptm, end="\n\n")
        log.debug(self.ptm)

    def apply_ptm(self, bit):
        log.debug("The following PTM is applied on qubit %s:", bit)
        #log.debug(self.ptm.round(round_precision), end="\n\n")
        log.debug(self.ptm.round(round_precision))
        self.sdm.apply_ptm(bit, self.ptm)

    def apply_mock_meas(self, fn: str):
        np.save(fn, self.sdm.full_dm.to_array())

    def apply_measurement(self, bit):

        log.debug("QuantumSim: prepare to measure qubit %s", bit)

        # Apply pending ptms
        self.sdm.combine_and_apply_single_ptm(bit)
        # self.apply_all_pending()

        log.debug("The full density matrix before applying measurement:")
        #log.debug(self.sdm.full_dm.to_array().round(
            #round_precision), end="\n\n")
        log.debug(self.sdm.full_dm.to_array().round(
            round_precision))

        # Obtain the two partial traces (p0, p1) that define the probabilities
        # for measuring bit in state (0, 1)
        p0, p1 = self.sdm.peak_measurement(bit)
        log.debug("partial traces for this measurement: {}, {}".format(p0, p1))

        # Sample from these two possibilities

        # using default sample with seed random
        # declare, project, cond_prob = self.sampler.send((p0, p1))

        # using fully random
        r = random.random()
        log.debug("the random value for measuremnt: {}".format(r))

        if r < p0 / (p0 + p1):
            project = 0
        else:
            project = 1

        r = random.random()
        if r < self.readout_error:
            decl = 1 - project
            prob = self.readout_error
        else:
            decl = project
            prob = 1 - self.readout_error

        # Record this measurement result in the corresponding qubit list
        self.measurements[bit].append(project)
        self.current_measurement = project
        # Project a bit to a fixed state, making it classical and reducing the size of the full density matrix.
        self.sdm.project_measurement(bit, project)
        log.info("The state of qubit %s is measured. The result is: %d", bit, project)
        self.sdm.renormalize()

    def return_measurement_result(self):
        return self.current_measurement

    # For superconducting qubits, the only two-qubit operation is cphase.
    # Currently the two-qubit ptm is set to default cphase ptm in the backend.
    def prepare_two_ptm(self):
        log.debug("QuantumSim: operation to apply: CZ")

        self.ptm = self.sdm._cphase_ptm

    def apply_two_ptm(self, bit0, bit1):

        log.debug("QuantumSim: CZ applied on qubit {} and {}.".format(
            bit0, bit1))

        self.sdm.apply_two_ptm(bit0, bit1, self.ptm)

    def calculate_gamma_lamda(self, duration):
        t1 = self.t1
        t2 = self.t2

        if t2 == 2 * t1:
            t_phi = np.inf
        else:
            t_phi = 1 / (1 / t2 - 1 / (2 * t1)) / 2

        self.gamma = 1 - np.exp(-duration / t1)
        self.lamda = 1 - np.exp(-duration / t_phi)

    def prepare_idling_ptm(self):
        if self.error_on == True:
            self.ptm = amp_ph_damping_ptm(self.gamma, self.lamda)
        else:
            self.ptm = amp_ph_damping_ptm(0, 0)

    def print_classical_state(self):
        log.info("classical state: ")
        for i in range(0, self.sdm.no_qubits):
            #log.debug("q%d: %d", i, self.sdm.classical[str(i)], end=" ")
            log.debug("q%d: %d", i, self.sdm.classical[str(i)])
        log.debug("")

    def print_ptm_to_do(self, qubit_name):
        """
        Dump the Pauli Transfer Matrix pending on the qubit.
        Input parameters:
            qubit_name:     the qubit number
        """
        log.debug("\nPTM to be applied on q{}:".format(qubit_name))

        for i in range(0, len(self.sdm.single_ptms_to_do[qubit_name])):
            log.debug(np.array2string(
                self.sdm.single_ptms_to_do[qubit_name][i]))

    def apply_all_pending(self):
        self.sdm.apply_all_pending()

    def print_full_dm(self):
        str_out = '\n+' + '-' * 20 + "The full density matrix at now is:" + '-' * 20 + '+'
        str_out = str_out + \
            np.array2string(self.sdm.full_dm.to_array().round(round_precision))
        str_out = str_out + '+' + '-' * 77 + '+\n'
        log.info(str_out)

    def print_final_result(self):
        str_out = '\n+' + '-' * 20 + "The full density matrix at now is:" + '-' * 20 + '+'
        str_out = str_out + \
            np.array2string(self.sdm.full_dm.to_array().round(round_precision))
        str_out = str_out + '+' + '-' * 77 + '+\n'
        log.debug(str_out)

        # TODO: reformat the following code to dump the correct measurement result
        # str_out = '\n+' + '-' * 20 + \
        #     "The measurement results of all qubits are:" + '-' * 20 + '+'
        # for i in range(0, self.num_qubit):
        #     print("\nThe measurement results of qubit", i, "are:")
        #     lines = round(len(self.measurements[str(i)])/30) + 1
        #     for j in range(0, lines):
        #         print((self.measurements[str(i)][j * 30:(j + 1) * 30]))
        # print('+', '-' * 77, '+\n')

    def record_msmt_results(self):
        f = open("qvm_msmt_result.txt", "w+")

        for i in range(0, self.num_qubit):
            f.write("\nThe measurement results of qubit " + str(i) + " are: ")
            for j in range(0, len(self.measurements[str(i)])):
                f.write(str(self.measurements[str(i)][j]) + ", ")

        f.close()
