/**
 *  @file database_benchmark_driver.hpp
 *  @copyright defined in eosio/LICENSE.txt
 */

#include "database_benchmark_interface.hpp"

template<typename Database>
database_benchmark<Database>::database_benchmark(window window)
    : _window{window}
{
}

template<typename Database>
database_benchmark<Database>::~database_benchmark() = default;

template<typename Database>
void database_benchmark<Database>::set_program_options(boost::program_options::options_description& cli) {
    cli.add_options()
        ("seed,s",
         boost::program_options::value<unsigned int>(&_seed)->default_value(42),
         "Seed value for the random number generator.")

        ("lower-bound,l",
         boost::program_options::value<size_t>(&_lower_bound_inclusive)->default_value(std::numeric_limits<size_t>::min()),
         "Seed value for the random number generator.")

        ("upper-bound,u",
         boost::program_options::value<size_t>(&_upper_bound_inclusive)->default_value(std::numeric_limits<size_t>::max()),
         "Seed value for the random number generator.")
         
        ("num-of-accounts,n",
         boost::program_options::value<size_t>(&_num_of_accounts)->default_value(100000),
         "Number of unique individual accounts with a corresponding value; key/value pair.")
         
        ("num-of-swaps,w",
         boost::program_options::value<size_t>(&_num_of_swaps)->default_value(100000),
         "Number of swaps to perform during the benchmark.")
         
        ("max-key-length,k",
         boost::program_options::value<size_t>(&_max_key_length)->default_value(1023),
         "Maximum key byte vector size.")
         
        ("max-key-value,y",
         boost::program_options::value<size_t>(&_max_key_value)->default_value(255),
         "Maximum value to fill the key byte vector with.")
         
        ("max-value-length,v",
         boost::program_options::value<size_t>(&_max_value_length)->default_value(1023),
         "Maximum value byte vector size.")

        ("max-value-value,e",
         boost::program_options::value<size_t>(&_max_value_value)->default_value(255),
         "Maximum value to fill the value byte vector with.")
         
        ("help,h", "Print this help message and exit.");
}

template<typename Database>
void database_benchmark<Database>::execute_benchmark() {
    _gen_data.init(_seed,
                   _lower_bound_inclusive,
                   _upper_bound_inclusive,
                   _num_of_accounts,
                   _num_of_swaps,
                   _max_key_length,
                   _max_key_value,
                   _max_value_length,
                   _max_value_value);
      
    _initial_database_state();
    _execution_loop();
    loggerman->flush_all();
}

template<typename Database>
void database_benchmark<Database>::_initial_database_state() {
    clockerman->reset_clocker();
    
    std::cout << "Filling initial database state...\n" << std::flush;
    loggerman->print_progress(1,0);

    for (size_t i{}; i < _gen_data.accounts().size(); ++i) {
        // std::cout << i << std::endl;
        // std::cout << _gen_data.accounts().size() << std::endl;
        
        _database.put(_gen_data.accounts()[i], _gen_data.values()[i]);
        // _database.write();

        if (UNLIKELY(clockerman->should_log())) {
            loggerman->print_progress(i, _gen_data.accounts().size());
            clockerman->update_clocker();
        }
        
        // std::cout << "_initial_database_state():\n";
        // std::cout << "_gen_data.accounts()[" << i << "]: " <<
        //              std::string{_gen_data.accounts()[i].cbegin(), _gen_data.accounts()[i].cend()} << '\n';
        // std::cout << "_gen_data.values()[" << i << "]:   " <<
        //              std::string{_gen_data.values()[i].cbegin(), _gen_data.values()[i].cend()} << '\n';
    }

    _database.write();
    loggerman->print_progress(1,1);
    std::cout << "done.\n" << std::flush;
}

template<typename Database>
void database_benchmark<Database>::_execution_loop() {
    clockerman->reset_clocker();
    size_t transactions_per_second{};

    std::cout << "Benchmarking...\n" << std::flush;
    loggerman->print_progress(1,0);

    for (size_t i{}; i < _gen_data.num_of_swaps(); ++i) {
        // std::cout << "Outside.\n";
        _database.swap(_gen_data, i);
        // _database.write();
        
        if (UNLIKELY(clockerman->should_log())) {
            // std::cout << "Inside.\n";
            loggerman->log_tps(_narrow_window_metric(transactions_per_second));
            loggerman->log_total_vm_usage(_system_metrics.total_vm_usage());    
            transactions_per_second = 0;
            _database.write();
            
            loggerman->print_progress(i, _gen_data.num_of_swaps());
            clockerman->update_clocker();
        }
        transactions_per_second += 2;
    }

    // _database.write();
    loggerman->print_progress(1,1);
    std::cout << "done.\n" << std::flush;
}

template<typename Database>
size_t database_benchmark<Database>::_narrow_window_metric(size_t tps) {
    return tps/clockerman->narrow_window();
}