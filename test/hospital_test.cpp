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

struct out_dirty : public out_port<int>{};

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
    shared_ptr<dynamic::modeling::model> hospital
        = dynamic::translate::make_dynamic_atomic_model<Hospital,            TIME, int, double, double, int>("hospital", move(5400), move(800), move(50), move(0));



    /** top level model **/


    dynamic::modeling::Ports iports_TOP = {};

    dynamic::modeling::Ports oports_TOP = {typeid(out_dirty)};

    dynamic::modeling::Models submodels_TOP = {hospital, input_reader};

    dynamic::modeling::EICs eics_TOP = {};

    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<Hospital_defs::dirty, out_dirty>("hospital")
    };

    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<int>::out, Hospital_defs::clean>("input_reader","hospital")
    };

    shared_ptr<dynamic::modeling::coupled<TIME>> TOP = make_shared<dynamic::modeling::coupled<TIME>>("TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP );


    /*************** Loggers *******************/
    static ofstream out_messages("./simulation_results/test_hospital_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){
            return out_messages;
        }
    };

    static ofstream out_state("./simulation_results/test_hospital_state.txt");
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


    r.run_until(TIME({24*7}));

    return 0;
}
