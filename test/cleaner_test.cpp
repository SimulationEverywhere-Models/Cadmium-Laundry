//Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

//Time class header
#include <NDTime.hpp>

//Atomic model headers
#include <cadmium/basic_model/pdevs/iestream.hpp> //Atomic model for inputs
#include "../atomics/laundromat_cleaning.hpp"
#include "../atomics/laundromat_shipping.hpp"
#include "../atomics/hospital.hpp"

//C++ headers
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>
#include <sstream>


using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

struct out_clean : public out_port<int>{};

template<typename T>
class InputReader_Int : public iestream_input<int,T> {
public:
    InputReader_Int() = default;
    InputReader_Int(const char* file_path) : iestream_input<int,T>(file_path) {}
};

int main(int argc, char ** argv) {

    if (argc < 2) {
        cout << "Program used with wrong parameters. The program must be invoked as follow:";
        cout << argv[0] << " path to the input file " << endl;
        return 1;
    }
    /****** Input Reader atomic model instantiation *******************/
    string input = argv[1];
    const char * i_input = input.c_str();
    shared_ptr<dynamic::modeling::model> input_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_Int, TIME, const char* >("input_reader" , move(i_input));


    /****** Atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> cleaning
        = dynamic::translate::make_dynamic_atomic_model<Laundromat_cleaning, TIME, int, int>("cleaning", move(10), move(60));



    /** top level model **/


    dynamic::modeling::Ports iports_TOP = {};

    dynamic::modeling::Ports oports_TOP = {typeid(out_clean)};

    dynamic::modeling::Models submodels_TOP = {cleaning, input_reader};

    dynamic::modeling::EICs eics_TOP = {};

    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<Laundromat_cleaning_defs::clean, out_clean>("cleaning")
    };

    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<int>::out, Laundromat_cleaning_defs::dirty>("input_reader","cleaning")
    };

    shared_ptr<dynamic::modeling::coupled<TIME>> TOP = make_shared<dynamic::modeling::coupled<TIME>>("TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP );


    /*************** Loggers *******************/
    static ofstream out_messages("./simulation_results/test_cleaning_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){
            return out_messages;
        }
    };

    static ofstream out_state("./simulation_results/test_cleaning_state.txt");
    struct oss_sink_state{
        static ostream& sink(){
            return out_state;
        }
    };

    using state=logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>, oss_sink_state>;

    using log_messages=logger::logger<logger::logger_messages, dynamic::logger::formatter<TIME>, oss_sink_messages>;

    using global_time_mes=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_messages>;

    using global_time_sta=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_state>;

    using logger_top=logger::multilogger<state, log_messages, global_time_mes, global_time_sta>;

    /************** Runner call ************************/
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});

    r.run_until_passivate();

    return 0;
}
