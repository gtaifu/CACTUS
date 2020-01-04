#include "exe_flag_gen.h"

#include <vector>

namespace cactus {

Fce_logic::Fce_logic(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n) {

    msmt_res_his.init(NUM_HIS_MSMT_RES);

    SC_CTHREAD(log_msmt_his, clock);
    SC_CTHREAD(gen_exe_flag, clock);
}

void Fce_logic::log_msmt_his() {

    std::vector<sc_uint<1>> i_msmt_res_his;
    i_msmt_res_his.resize(msmt_res_his.size());

    while (true) {

        wait();

        if (reset.read()) {
            for (size_t i = 0; i < msmt_res_his.size(); ++i) {
                msmt_res_his[i].write(0);
                i_msmt_res_his[i] = 0;
            }
        } else if (QM2MRF_qubit_ena.read()) {
            // shift the value
            i_msmt_res_his[0] = QM2MRF_qubit_data.read();
            for (size_t i = 0; i < msmt_res_his.size() - 1; ++i) {
                i_msmt_res_his[i + 1] = msmt_res_his[i].read();
            }

            // update the value
            for (size_t i = 0; i < msmt_res_his.size(); ++i) {
                msmt_res_his[i].write(i_msmt_res_his[i]);
            }
        }
    }
}

void Fce_logic::gen_exe_flag() {

    sc_uint<NUM_EXE_FLAGS> exe_flag = 0;

    while (true) {

        wait();

        exe_flag[0] = 1;
        exe_flag[1] = msmt_res_his[0].read();
        exe_flag[2] = ~msmt_res_his[0].read();
        exe_flag[3] = msmt_res_his[0].read() ^ msmt_res_his[1].read();

        out_exe_flag.write(exe_flag);
    }
}
void Exe_flag_gen::config() {
    Global_config& global_config = Global_config::get_instance();
    num_qubits                   = global_config.num_qubits;
}

Exe_flag_gen::Exe_flag_gen(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n)
    , vec_fce_logic("fce_logic", Global_config::get_instance().num_qubits,
                    [&](const char* nm, size_t i) { return new Fce_logic(nm); }) {

    config();

    QM2MRF_qubit_data.init(num_qubits);
    QM2MRF_qubit_ena.init(num_qubits);
    exe_flag_per_qubit.init(num_qubits);

    for (size_t i = 0; i < vec_fce_logic.size(); i++) {
        vec_fce_logic[i].clock(clock);
        vec_fce_logic[i].reset(reset);
    }

    sc_assemble_vector(vec_fce_logic, &Fce_logic::QM2MRF_qubit_data).bind(QM2MRF_qubit_data);
    sc_assemble_vector(vec_fce_logic, &Fce_logic::QM2MRF_qubit_ena).bind(QM2MRF_qubit_ena);
    sc_assemble_vector(vec_fce_logic, &Fce_logic::out_exe_flag).bind(exe_flag_per_qubit);
}

}  // namespace cactus
