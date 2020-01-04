
class Single_qubit_addr_info {
  public:
    qaddr_type_t type;

    // directly stores the indices of qubits
    std::vector<size_t> qubit_indices;

    // use each bit in the mask to indicate if the qubit is selected or not
    bin_field_t mask;
}

class Multi_qubit_addr_info {
  public:
    qaddr_type_t type;

    // a list of qubit tuples, used by n-qubit gates, n >= 2
    std::vector<std::vector<size_t>> qubit_tuples;

    // use each bit in the mask to indicate if a qubit tuple is selected or not
    bin_field_t mask;
}

// a universal data structure which stores the address of quantum operations
class Q_tgt_addr {
  public:
    // the type of address
    // when it is HARDWIRE, the content inside this data structure is not used.
    qaddr_type_t type;

    // the VLIW width in the
    size_t vliw_width;

    // number of qubits specified by this target qubit register
    unsigned int num_qubits;

    // the number of register which stores the indrect address
    // For single-qubit operations, each indrect address can specify a list of qubits
    // For n-quibit gates, each indrect address can specify a list of n-qubit tuples
    // Currently, the indirect address is the mask.
    unsigned int indirect_addr_reg_num;

    // directly stores the indices of qubits
    std::vector<size_t> qubit_indices;

    // a list of qubit tuples, used by n-qubit gates, n >= 2
    std::vector<std::vector<size_t>> qubit_tuples;

    // use each bit in the mask to indicate if a qubit tuple is selected or not
    bin_field_t mask;
}