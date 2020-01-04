#include "event_queue_manager.h"

namespace cactus {

void Event_queue_manager::config() {
    Global_config& global_config = Global_config::get_instance();

    m_num_qubits = global_config.num_qubits;

    is_telf_on = true;
    telf_fn    = sep_telf_fn(global_config.output_dir, this->name(), "event_queue_manager");
}

Event_queue_manager::Event_queue_manager(const sc_core::sc_module_name& n)
    : Telf_module(n)
    , event_queue(32) {

    auto logger = get_logger_or_exit("console");
    logger->trace("Start initializing {}...", this->name());

    config();
    open_telf_file();

    // methods
    SC_CTHREAD(write_fifo, in_clock.pos());

    SC_CTHREAD(read_fifo, in_50MHz_clock.pos());

    SC_CTHREAD(do_output, in_50MHz_clock.pos());

    SC_CTHREAD(generate_count_finish_sig, in_50MHz_clock.pos());

    SC_THREAD(generate_counter_start);
    sensitive << i_run << i_run_pos_old << counter_finished_sig << reset;

    SC_THREAD(generate_event_queue_read_sig);
    sensitive << counter_finished_sig << i_run_pos << i_run_pos_old << reset;

    SC_THREAD(generate_run_pos_sig);
    sensitive << i_run << i_run_old << reset;

    SC_THREAD(write_state);
    sensitive << i_error_state << reset;

    SC_CTHREAD(counter_control, in_50MHz_clock.pos());

    SC_CTHREAD(log_telf, in_50MHz_clock.pos());

    logger->trace("Finished initializing {}...", this->name());
}

// cthread
void Event_queue_manager::write_fifo() {

    Q_pipe_interface q_pipe_interface;

    while (true) {
        wait();

        q_pipe_interface.reset();  // clear everything at the begin of every cycle
        q_pipe_interface = in_q_pipe_interface.read();

        // queued valid event info
        if (q_pipe_interface.if_content.valid_wait) {
            event_queue.write(q_pipe_interface);
        }
    }
}

// cthread
void Event_queue_manager::read_fifo() {

    auto logger = get_logger_or_exit("console");

    while (true) {
        wait();

        // whether event queue is almost full
        if (event_queue.num_available() >= 16) {
            out_eq_almostfull.write(true);
        } else {
            out_eq_almostfull.write(false);
        }

        // received read request
        if (event_queue_read_sig.read()) {

            q_pipe_interface_sig.write(event_queue.read());

            // whether event queue is empty
            if (event_queue.num_available() != 0) {
                eq_empty.write(false);
            } else {
                eq_empty.write(true);
            }
        }
    }
}

// thread
void Event_queue_manager::generate_run_pos_sig() {

    while (true) {
        wait();
        // detect a rising edge on the run signal
        if (i_run.read() && !i_run_old.read()) {
            i_run_pos.write(true);
        } else {
            i_run_pos.write(false);
        }
    }
}

// cthread
void Event_queue_manager::counter_control() {

    bool run_val;

    while (true) {
        wait();

        if (i_run.read() && eq_empty.read()) {
            i_error_state.write(1);
        }

        // synchronize run to the clock domain
        run_val = run.read();
        i_run.write(run_val);

        // detects a rising edge on the run signal, and delays the rising edge for one clock
        i_run_old.write(i_run.read());
        i_run_pos_old.write(i_run_pos.read());
    }
}

// thread
void Event_queue_manager::write_state() {
    while (true) {
        wait();

        // output event queue state
        out_error_state.write(i_error_state.read());
    }
}

// thread
void Event_queue_manager::generate_event_queue_read_sig() {

    auto logger = get_logger_or_exit("telf_logger");

    while (true) {
        wait();

        // generate read request signal for the event queue
        // i_run_pos_old is used to generate an extra read request for next event
        if (i_run_pos_old.read() || i_run_pos.read() || counter_finished_sig.read()) {
            event_queue_read_sig.write(true);

            logger->trace("{}: generate event queue read request @{}", this->name(),
                          sc_core::sc_time_stamp().to_string());
        } else {
            event_queue_read_sig.write(false);
        }
    }
}

// thread
void Event_queue_manager::generate_counter_start() {

    auto logger = get_logger_or_exit("telf_logger");

    while (true) {
        wait();

        // generate counter start signal of each event
        if (!i_run.read() || i_error_state.read()) {
            counter_start.write(false);
        } else {
            if (i_run_pos_old.read() | counter_finished_sig.read()) {
                counter_start.write(true);

                logger->trace("{}: counter start @{}", this->name(),
                              sc_core::sc_time_stamp().to_string());
            } else {
                counter_start.write(false);
            }
        }
    }
}

// cthread
void Event_queue_manager::generate_count_finish_sig() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface q_pipe_interface;

    while (true) {
        wait();

        q_pipe_interface = q_pipe_interface_sig.read();
        if (counter_start.read()) {
            // event ready to output
            q_pipe_interface_next_sig.write(q_pipe_interface);
            target_count_value.write(q_pipe_interface.timing.wait_time);
        }

        // counter is running
        if (counter_running.read()) {
            if (counter < target_count_value.read()) {
                counter.write(counter.read() + 1);
            } else {
                counter_running.write(false);
                counter.write(0);
            }
        }

        // when counter start, set counter_running and counter value
        if (counter_start.read()) {
            counter_running.write(true);
            counter.write(1);
        }

        // whether reached the last one count
        if (counter_start.read() && (q_pipe_interface.timing.wait_time == 2)) {
            last_but_one.write(1);
        } else if ((counter.read() == (target_count_value.read() - 2))) {
            last_but_one.write(1);
        } else {
            last_but_one.write(0);
        }

        // ready to output event or not
        if (counter_running.read() && last_but_one.read()) {
            counter_finished_sig.write(true);
        } else if (counter_start.read() && (q_pipe_interface.timing.wait_time == 1)) {
            counter_finished_sig.write(true);
        } else {
            counter_finished_sig.write(false);
        }
    }
}

// cthread
void Event_queue_manager::do_output() {

    auto logger = get_logger_or_exit("telf_logger");

    Q_pipe_interface q_pipe_interface;

    while (true) {
        wait();
        q_pipe_interface.reset();

        // output event
        if (counter_finished_sig.read()) {
            q_pipe_interface = q_pipe_interface_next_sig.read();
            out_q_pipe_interface.write(q_pipe_interface);

            logger->trace("{}: dequeue @{}, timing label '0x{:x}'", this->name(),
                          sc_core::sc_time_stamp().to_string(), q_pipe_interface.timing.label);
        } else {
            q_pipe_interface.ops.resize(m_num_qubits);
            out_q_pipe_interface.write(q_pipe_interface);
        }
    }
}

void Event_queue_manager::log_telf() {
    Q_pipe_interface q_pipe_interface;
    while (true) {

        wait();

        q_pipe_interface.reset();
        q_pipe_interface = out_q_pipe_interface.read();

        if (is_telf_on) {
            telf_os << q_pipe_interface;
        }
    }
}

void Event_queue_manager::add_telf_header() {
    telf_os << std::setfill(' ') << std::setw(7) << "content"
            << " "  // instruction type
            << std::setfill(' ') << std::setw(9) << "time_type"
            << " "  // timing type
            << std::setfill(' ') << std::setw(9) << "wait_time"
            << " "  // wait time
            << std::setfill(' ') << std::setw(5) << "label"
            << " "  // timing label
            << std::setfill(' ') << std::setw(9) << "addr_type"
            << " "  // addr type
            << std::setfill(' ') << std::setw(6) << "q_type"
            << " "  // single or multi qubit operation
            << std::setfill(' ') << std::setw(10) << "reg_num"
            << " "  // register num
            << std::setfill(' ') << std::setw(10) << "mask"
            << " "  // addr mask
            << std::setfill(' ') << std::setw(24) << "qubit_addr"
            << " "  // qubit address
            << std::setfill(' ') << std::setw(8) << "op_type"
            << " "  // operation type
            << std::setfill(' ') << std::setw(10) << "op "
            << " "  // operation
            << std::endl;
}

void Event_queue_manager::add_telf_line() {}

Event_queue_manager::~Event_queue_manager() { close_telf_file(); }

}  // end of namespace cactus