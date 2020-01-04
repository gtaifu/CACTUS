#include "addr_mask_decoder.h"

#include <sstream>

namespace cactus {

void Address_decoder::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    out_edges_of_qubit = global_config.out_edges_of_qubit;
    in_edges_of_qubit  = global_config.in_edges_of_qubit;
}

Address_decoder::Address_decoder(const sc_core::sc_module_name& n)
    : Telf_module(n) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();

    SC_THREAD(mask_decode);
    sensitive << in_q_pipe_interface;

    logger->trace("Finished initializing {}...", this->name());
}

void Address_decoder::mask_decode() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface         q_pipe_interface;
    Fledged_qop              qop;
    std::vector<Fledged_qop> vec_qop;
    std::vector<size_t>      qubit_tuple;

    std::stringstream ss;  // log stringstream

    vec_qop.resize(m_num_qubits);  // used for hardwire addressing

    while (true) {
        wait();

        ss.str("");  // clear log stringstream

        q_pipe_interface = in_q_pipe_interface.read();

        for (size_t i = 0; i < m_num_qubits; ++i) {  // clear qop at the begin of every cycle
            vec_qop[i].reset();
        }

        if (reset.read()) {  // when received reset signals
            q_pipe_interface.reset();
            for (size_t i = 0; i < m_num_qubits; ++i) {  // push hardwire operation
                q_pipe_interface.ops.push_back(vec_qop[i]);
            }

            out_q_pipe_interface.write(q_pipe_interface);

            continue;
        }

        if (q_pipe_interface.if_content.valid_qop) {
            qop = q_pipe_interface.ops[0];
            // addressing type is indirect reg num
            if (qop.addr.type.c_type == INDIRECT_REG_NUM) {

                if (qop.addr.type.q_num_type ==
                    SINGLE) {  // check each mask bit of single-qubit operation

                    ss << "{";
                    for (size_t i = 0; i < qop.addr.sq_op_addr.mask.get_width(); ++i) {
                        if ((qop.addr.sq_op_addr.mask.get_value() >> i) & 0x1) {

                            vec_qop[i]                  = qop;  // set i-th hardwire qubit
                            vec_qop[i].timing           = q_pipe_interface.timing;
                            vec_qop[i].addr.type.c_type = HARDWIRE;
                            vec_qop[i].addr.sq_op_addr.qubit_indices.push_back(i);

                            ss << i << " ";
                        }
                    }

                    ss << "}";
                    logger->debug("{}: type:single, mask:0x{:x}, indice:{}", this->name(),
                                  qop.addr.sq_op_addr.mask.get_value(), ss.str());
                } else {  // check each mask bit of multi-qubit operation, get left qubit and right
                    // qubit
                    ss << "{";
                    size_t left_qubit, right_qubit;
                    for (size_t i = 0; i < qop.addr.mq_op_addr.mask.get_width(); ++i) {

                        if ((qop.addr.mq_op_addr.mask.get_value() >> i) & 0x1) {  // mask bit valid
                            for (size_t j = 0; j < out_edges_of_qubit.size();
                                 ++j) {  // left qubit addr
                                std::vector<unsigned int>::iterator it = std::find(
                                  out_edges_of_qubit[j].begin(), out_edges_of_qubit[j].end(), i);

                                if (it != out_edges_of_qubit[j].end()) {
                                    left_qubit = j;
                                    break;
                                }
                            }

                            for (size_t k = 0; k < in_edges_of_qubit.size();
                                 ++k) {  // right qubit addr
                                std::vector<unsigned int>::iterator it = std::find(
                                  in_edges_of_qubit[k].begin(), in_edges_of_qubit[k].end(), i);
                                if (it != in_edges_of_qubit[k].end()) {
                                    right_qubit = k;
                                    break;
                                }
                            }

                            // set left qubit and right qubit
                            qubit_tuple.push_back(left_qubit);
                            qubit_tuple.push_back(right_qubit);

                            vec_qop[left_qubit]                   = qop;
                            vec_qop[right_qubit]                  = qop;
                            vec_qop[left_qubit].timing            = q_pipe_interface.timing;
                            vec_qop[right_qubit].timing           = q_pipe_interface.timing;
                            vec_qop[left_qubit].addr.type.c_type  = HARDWIRE;
                            vec_qop[right_qubit].addr.type.c_type = HARDWIRE;
                            vec_qop[left_qubit].addr.mq_op_addr.qubit_tuples.push_back(qubit_tuple);
                            vec_qop[right_qubit].addr.mq_op_addr.qubit_tuples.push_back(
                              qubit_tuple);

                            qubit_tuple.clear();

                            ss << "(" << left_qubit << " " << right_qubit << ")"
                               << " ";
                        }  // end of mask bit valid
                    }      // end of check each mask bit

                    ss << "}";
                    logger->debug("{}: type:multiple, mask:0x{:x}, tuple:{}", this->name(),
                                  qop.addr.mq_op_addr.mask.get_value(), ss.str());
                }  //  end of multipul qubits operation

            } else if (qop.addr.type.c_type == INDIRECT_REG_CONTENT) {
                // addressing type is indirect reg num
                if (qop.addr.type.q_num_type == SINGLE) {  // single-qubit operation
                    ss << "{";

                    // if operation is mock_meas and default register 0 has no assigned qubits
                    if (qop.op.name == "mock_meas") {
                        qop.addr.sq_op_addr.qubit_indices.clear();
                        for (size_t i = 0; i < m_num_qubits; ++i) {
                            qop.addr.sq_op_addr.qubit_indices.push_back(i);
                        }
                    }

                    for (size_t i = 0; i < qop.addr.sq_op_addr.qubit_indices.size(); ++i) {

                        size_t qubit = qop.addr.sq_op_addr.qubit_indices[i];

                        vec_qop[qubit]                  = qop;  // set i-th hardwire qubit
                        vec_qop[qubit].timing           = q_pipe_interface.timing;
                        vec_qop[qubit].addr.type.c_type = HARDWIRE;
                        vec_qop[qubit].addr.sq_op_addr.qubit_indices.clear();
                        vec_qop[qubit].addr.sq_op_addr.qubit_indices.push_back(qubit);

                        ss << qubit << " ";
                    }

                    ss << "}";
                    logger->debug("{}: type:single, indice:{}", this->name(), ss.str());

                } else {  // multi-qubit operation
                    size_t left_qubit, right_qubit;
                    ss << "{";
                    for (size_t i = 0; i < qop.addr.mq_op_addr.qubit_tuples.size(); ++i) {
                        left_qubit  = qop.addr.mq_op_addr.qubit_tuples[i][0];
                        right_qubit = qop.addr.mq_op_addr.qubit_tuples[i][1];

                        // set left qubit and right qubit
                        qubit_tuple.push_back(left_qubit);
                        qubit_tuple.push_back(right_qubit);

                        vec_qop[left_qubit]                  = qop;
                        vec_qop[left_qubit].timing           = q_pipe_interface.timing;
                        vec_qop[left_qubit].addr.type.c_type = HARDWIRE;
                        vec_qop[left_qubit].addr.mq_op_addr.qubit_tuples.clear();
                        vec_qop[left_qubit].addr.mq_op_addr.qubit_tuples.push_back(qubit_tuple);

                        vec_qop[right_qubit]                  = qop;
                        vec_qop[right_qubit].timing           = q_pipe_interface.timing;
                        vec_qop[right_qubit].addr.type.c_type = HARDWIRE;
                        vec_qop[right_qubit].addr.mq_op_addr.qubit_tuples.clear();
                        vec_qop[right_qubit].addr.mq_op_addr.qubit_tuples.push_back(qubit_tuple);

                        qubit_tuple.clear();

                        ss << "(" << left_qubit << " " << right_qubit << ")"
                           << " ";
                    }

                    ss << "}";
                    logger->debug("{}: type:multiple, tuple:{}", this->name(), ss.str());
                }
            } else {
                // other addressing mode do not need address decoder
            }
        }

        q_pipe_interface.ops.clear();
        for (size_t i = 0; i < m_num_qubits; ++i) {  // push hardwire operation
            q_pipe_interface.ops.push_back(vec_qop[i]);
        }

        out_q_pipe_interface.write(q_pipe_interface);
    }
}

void Address_decoder::add_telf_header() {}

void Address_decoder::add_telf_line() {}

}  // namespace cactus
