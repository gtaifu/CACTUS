from qusim_server import QuantumCircuit, Auth
from gates import *
import json
import logging
logging.basicConfig(level=logging.INFO)
# log = logging.getLogger(__name__)


class interface_QIcircuit:
    """
    This is the class used to interact with the kernel of QIcircuit.
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
        self.circuit = None

        QUSIM_AUTHORIZATION_IP = 'qcapi.supremacyfuture.com'
        QUSIM_AUTHORIZATION_PORT = 80
        QUSIM_AUTHORIZATION_TOKEN = ''

        # valid token should be provided
    
        self.auth = Auth(
            QUSIM_AUTHORIZATION_IP,
            QUSIM_AUTHORIZATION_TOKEN,
            QUSIM_AUTHORIZATION_PORT
        )
        # The measurement results recording dict

        self.measurements = {}
        self.current_measurement = None

    def init_circuit(self, num_qubit):

        self.num_qubit = num_qubit

        self.circuit = QuantumCircuit(
            L=self.num_qubit, auth=self.auth, interactive=True)

        logging.debug(
            "QIcircuit: the circuit has been initialized successfully.")

    def add_single_qubit_operation(self, operation, qubit):
        self.circuit.add(key=qubit, op=operation)
        logging.debug(
            "QIcircuit: added opeartion %s on qubit %d ", operation, qubit)

    def add_two_qubit_operation(self, operation, qubit0, qubit1):
        self.circuit.add(key=(qubit0, qubit1), op=operation)
        logging.debug("QIcircuit: added opeartion %s on qubit %d %d ",
                      operation, qubit0, qubit1)

    def add_measurement(self, qubit):
        self.circuit.add_observer(key=qubit, auto_reset=True, keep=1)
        logging.debug("QIcircuit: added measurement on qubit %d", qubit)

    def return_measurement_result(self):
        result = self.circuit.run()
        return json.dumps(result)
        #file = open('result.json', 'w', encoding='utf-8')
        #json.dump(result, file)

    def clear_circuit(self):
        self.circuit.clear()
