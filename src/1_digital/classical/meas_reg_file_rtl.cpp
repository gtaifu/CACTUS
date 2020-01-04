#include "meas_reg_file_rtl.h"

#include <iomanip>
#include <sstream>

namespace cactus {

void Meas_reg_file_rtl::config() {

    Global_config& global_config = Global_config::get_instance();

    num_qubits   = global_config.num_qubits;
    m_output_dir = global_config.output_dir;
}

Meas_reg_file_rtl::Meas_reg_file_rtl(const sc_core::sc_module_name& n)
    : sc_core::sc_module(n) {

    auto logger = get_logger_or_exit("console", CODE_POSITION);

    logger->trace("Start initialize {}...", this->name());

    config();

    open_telf_file();

    pending_meas_counter.init(num_qubits);

    Qp2MRF_qubit_ena_sig.init(num_qubits);
    Qp2MRF_qubit_ena_cancel_sig.init(num_qubits);
    Qm2MRF_qubit_data_sig.init(num_qubits);
    Qm2MRF_qubit_ena_sig.init(num_qubits);
    MRF2Clp_data.init(num_qubits);
    MRF2Clp_valid.init(num_qubits);

    qubit_valid.init(num_qubits);
    qubit_threshold.init(num_qubits);
    qubit_data.init(num_qubits);

    SC_THREAD(update_signals);
    sensitive << Qp2MRF_meas_issue << Qp2MRF_meas_cancel << Qm2MRF_meas_result;

    SC_CTHREAD(log_IO, clock.pos());

    SC_CTHREAD(interlocking_counter, clock.pos());
    SC_THREAD(interlocking_ready);
    sensitive << i_lock_counter;

    SC_CTHREAD(qubit_valid_counter, clock.pos());
    SC_THREAD(generate_signals);
    for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
        sensitive << pending_meas_counter[i];
    }

    SC_CTHREAD(output_register, clock.pos());

    SC_CTHREAD(clock_counter, clock.pos());
    SC_THREAD(write_output_file);
    for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
        sensitive << Qm2MRF_qubit_data_sig[i] << Qm2MRF_qubit_ena_sig[i];
    }

    logger->trace("Finished initializing {}...", this->name());
}

void Meas_reg_file_rtl::open_telf_file() {

    auto logger = get_logger_or_exit("telf_logger", CODE_POSITION);

    std::string msmt_result_veri_fn =
      sep_telf_fn(m_output_dir, this->name(), "msmt_result_for_verification");

    msmt_result_veri_out.open(msmt_result_veri_fn);

    if (msmt_result_veri_out.is_open()) {
        logger->trace("{}: measurement result verification file opened '{}'.", this->name(),
                      m_output_dir + "msmt_result_for_verification.csv");
    } else {
        logger->error(
          "{}: Failed to open measurement result verification file '{}'. Simulation aborts!",
          this->name(), m_output_dir + "msmt_result_for_verification.csv");
        exit(EXIT_FAILURE);
    }

    // Specify the signal type
    msmt_result_veri_out << "#MsmtResult" << std::endl;

    msmt_result_veri_out << "Clock cycle,    qubit result,    qubit valid" << std::endl;
}

void Meas_reg_file_rtl::close_telf_file() {
    auto logger = get_logger_or_exit("telf_logger", CODE_POSITION);

    if (msmt_result_veri_out.is_open()) msmt_result_veri_out.close();

    logger->trace("{}: measurement result file for verification (if open) has been close.",
                  this->name());
}

void Meas_reg_file_rtl::update_signals() {

    Generic_meas_if meas_issue;
    Generic_meas_if meas_cancel;
    Generic_meas_if meas_result;

    std::vector<bool> vec_meas_ena;
    std::vector<bool> vec_meas_en_cancel;
    std::vector<bool> vec_meas_data;
    std::vector<bool> vec_meas_data_valid;
    bool              qp_meas_issue;

    while (true) {
        wait();

        // derive signals from measurement generic interface
        meas_issue  = Qp2MRF_meas_issue.read();
        meas_cancel = Qp2MRF_meas_cancel.read();
        meas_result = Qm2MRF_meas_result.read();

        vec_meas_ena        = meas_issue.get_meas_ena();
        vec_meas_en_cancel  = meas_cancel.get_meas_ena_cancel();
        vec_meas_data       = meas_result.get_meas_data();
        vec_meas_data_valid = meas_result.get_meas_data_valid();

        qp_meas_issue = false;
        for (size_t i = 0; i < vec_meas_ena.size(); ++i) {
            Qp2MRF_qubit_ena_sig[i].write(vec_meas_ena[i] ? 1 : 0);
            qp_meas_issue |= vec_meas_ena[i];
        }
        Qp2MRF_meas_issue_sig.write(qp_meas_issue);

        for (size_t i = 0; i < vec_meas_en_cancel.size(); ++i) {
            Qp2MRF_qubit_ena_cancel_sig[i].write(vec_meas_en_cancel[i] ? 1 : 0);
        }

        for (size_t i = 0; i < vec_meas_data.size(); ++i) {
            Qm2MRF_qubit_data_sig[i].write(vec_meas_data[i] ? 1 : 0);
        }

        for (size_t i = 0; i < vec_meas_data_valid.size(); ++i) {
            Qm2MRF_qubit_ena_sig[i].write(vec_meas_data_valid[i] ? 1 : 0);
        }
    }
}

void Meas_reg_file_rtl::log_IO() {

    auto logger = get_logger_or_exit("MRF_logger", CODE_POSITION);

    std::stringstream ss;
    std::stringstream ss_tmp;
    bool              valid_msmt_res = false;

    while (true) {

        wait();

        ss.str("");
        ss << "@" << sc_core::sc_time_stamp() << ",";
        ss << "Meas_reg_file_rtl IO:\n";
        if (Clp2MRF_meas_issue.read())
            ss << "\tSlice -> MRF: Clp2MRF_meas_issue: " << Clp2MRF_meas_issue.read() << std::endl;
        if (Qp2MRF_meas_issue_sig.read())
            ss << "\tQuant -> MRF: Qp2MRF_meas_issue: " << Qp2MRF_meas_issue_sig.read()
               << std::endl;

        ss_tmp.str("");
        valid_msmt_res = false;

        ss_tmp << "\n\tUHFQC result -> MRF: [";
        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {

            ss_tmp << i << ":(" << Qm2MRF_qubit_ena_sig[i].read() << ", "
                   << Qm2MRF_qubit_data_sig[i].read() << ") ";

            if (Qm2MRF_qubit_ena_sig[i].read()) {
                valid_msmt_res = true;
            }
        }
        ss_tmp << "]\n";

        if (valid_msmt_res) {
            ss << ss_tmp.str();
        }

        ss << "\n\tMRF -> Slice: MRF2Clp_ready: " << MRF2Clp_ready.read() << "\n";
        ss << "\tMRF result -> Slice: [";
        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {

            ss << i << ":(" << MRF2Clp_valid[i].read() << ", " << MRF2Clp_data[i].read() << ") ";
        }
        ss << "]\n";

        ss << "\tpending_meas_counter: [";
        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {

            ss << pending_meas_counter[i].read() << " ";
        }
        ss << "]\n";

        logger->debug("{}: {}", this->name(), ss.str());
    }
}

void Meas_reg_file_rtl::interlocking_counter() {
    auto logger = get_logger_or_exit("MRF_logger", CODE_POSITION);

    sc_uint<G_INTERLOCK_COUNTER_BITS> v_lock_counter;
    std::stringstream                 ss;

    while (true) {
        wait();

        v_lock_counter = i_lock_counter.read();

        ss.str("");
        ss << "@" << sc_core::sc_time_stamp() << ",";
        logger->debug("{}: {}, v_lock_counter: {}", this->name(), ss.str(),
                      static_cast<int>(v_lock_counter));

        if (Clp2MRF_meas_issue.read()) {
            v_lock_counter = v_lock_counter + 1;
        }

        if (Qp2MRF_meas_issue_sig.read()) {
            v_lock_counter = v_lock_counter - 1;
        }

        if (reset.read()) v_lock_counter = 0;

        i_lock_counter.write(v_lock_counter);
    }
}

void Meas_reg_file_rtl::interlocking_ready() {
    while (true) {
        wait();

        i_lock_ready.write(1);

        if (i_lock_counter.read() != 0) {
            i_lock_ready.write(0);
        }
        if (Clp2MRF_meas_issue.read()) {
            i_lock_ready.write(0);
        }
    }
}

void Meas_reg_file_rtl::qubit_valid_counter() {
    sc_uint<G_INTERLOCK_COUNTER_BITS> v_counter;
    sc_uint<1>*                       v_qp_qubit_ena;
    sc_uint<1>*                       v_qp_qubit_ena_cancel;
    sc_uint<1>*                       v_qm_qubit_ena;
    sc_uint<1>*                       v_qubit_data;
    sc_uint<1>*                       v_qm_qubit_data;
    v_qp_qubit_ena        = new sc_uint<1>[num_qubits];
    v_qp_qubit_ena_cancel = new sc_uint<1>[num_qubits];
    v_qm_qubit_data       = new sc_uint<1>[num_qubits];
    v_qm_qubit_ena        = new sc_uint<1>[num_qubits];
    v_qubit_data          = new sc_uint<1>[num_qubits];
    while (true) {
        wait();

        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
            v_qp_qubit_ena[i]        = Qp2MRF_qubit_ena_sig[i].read();
            v_qp_qubit_ena_cancel[i] = Qp2MRF_qubit_ena_cancel_sig[i].read();
            v_qm_qubit_ena[i]        = Qm2MRF_qubit_ena_sig[i].read();
            v_qm_qubit_data[i]       = Qm2MRF_qubit_data_sig[i].read();
        }

        for (int q = 0; q < static_cast<int>(num_qubits); q++) {
            v_counter = pending_meas_counter[q].read();

            if (v_qp_qubit_ena[q] == 1) {
                v_counter = v_counter + 1;
            }
            if (v_qp_qubit_ena_cancel[q] == 1) {
                v_counter = v_counter - 1;
            }
            if (v_qm_qubit_ena[q] == 1) {
                v_counter = v_counter - 1;
                // update the data register when new data is available
                v_qubit_data[q] = v_qm_qubit_data[q];
            }
            if (reset.read()) v_counter = 0;

            pending_meas_counter[q].write(v_counter);
        }

        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
            qubit_data[i].write(v_qubit_data[i]);
        }
    }
}

void Meas_reg_file_rtl::generate_signals() {
    sc_uint<1>* v_qubit_valid;
    v_qubit_valid = new sc_uint<1>[num_qubits];
    while (true) {
        wait();

        for (int q = 0; q < static_cast<int>(num_qubits); q++) {
            if (pending_meas_counter[q].read() == 0)
                v_qubit_valid[q] = 1;
            else
                v_qubit_valid[q] = 0;

            // if (pending_meas_counter[q].read() == 32)
            // qubit_threshold.write(1);
            qubit_valid[q].write(v_qubit_valid[q]);
        }
    }
}

void Meas_reg_file_rtl::output_register() {
    while (true) {
        wait();

        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
            MRF2Clp_data[i].write(qubit_data[i].read());
        }

        for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
            // if there is a measure instruction not processed by quantum pipeline yet, all qubits
            // are invalid
            if (!i_lock_ready.read() || Clp2MRF_meas_issue.read())
                MRF2Clp_valid[i].write(0);
            else
                MRF2Clp_valid[i].write(qubit_valid[i].read());
        }

        // since threshold is not useful at current stage
        MRF2Clp_ready.write(1);

        if (reset.read()) {
            MRF2Clp_ready.write(1);
            for (int i = 0; i < static_cast<int>(num_qubits); ++i) {
                MRF2Clp_valid[i].write(1);
            }
        }
    }
}

void Meas_reg_file_rtl::clock_counter() {
    while (true) {
        wait();
        num_cycles++;
    }
}

void Meas_reg_file_rtl::write_output_file() {
    while (true) {
        wait();

        // Write the text output
        msmt_result_veri_out << std::setfill(' ') << std::setw(11) << num_cycles;
        msmt_result_veri_out << ",    " << std::setfill(' ') << std::setw(5) << "0b'";
        for (int i = (num_qubits - 1); i >= 0; i--) {
            msmt_result_veri_out << Qm2MRF_qubit_data_sig[i].read();
        }
        msmt_result_veri_out << ",    " << std::setfill(' ') << std::setw(4) << "0b'";
        for (int i = (num_qubits - 1); i >= 0; i--) {
            msmt_result_veri_out << Qm2MRF_qubit_ena_sig[i].read();
        }
        msmt_result_veri_out << std::endl;
    }
}

Meas_reg_file_rtl::~Meas_reg_file_rtl() { close_telf_file(); }

}  // namespace cactus
