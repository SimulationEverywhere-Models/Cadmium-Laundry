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


/***** Ports for coupled models *****/

struct in_dirty           : public in_port<int>{};
struct out_clean          : public out_port<int>{};
struct out_non_full_load  : public out_port<int>{};
struct out_load_delayed   : public out_port<string>{};
struct out_short_delivery : public out_port<int>{};
struct out_outage         : public out_port<int>{};

int main(int argc, char ** argv) {
    if (argc != 10) {
        cout << "Program used with wrong parameters. The program must be invoked as follow:";
        cout << argv[0] << "load_interval load_size clean_shipment_interval clean_shipment_size dirty_shipment_interval usage_mean usage_sd stockpile run_time" << endl;
        cout << "ex: " << argv[0] << " 60 10 3600 600 5400 800.0 50.0 1200 604800" << endl;
        cout << "    Every minute, a load of 10 items is cleaned." << endl <<
                "    Every hour a shipment of 600 items is sent to the hospital."<< endl <<
                "    Every hour and a half the hospital uses an average of 800.0 items with a standard deveation of 50.0."<< endl <<
                "    The hospital starts with 1200 items" << endl <<
                "    Run the simulation for 1 week" << endl;
        cout << "Of these paramaters, intervals are quantities of seconds as intagers, sizes are intagers, and mean and sd are reals" << endl;
        return 1;
    }
    int cleaning_time, cleaning_load_size, clean_shipping_time, clean_shipping_size, dirty_shipping_time, stockpile, total_run_time;
    double usage_mean, usage_sd;

    istringstream ss1(argv[1]);
    istringstream ss2(argv[2]);
    istringstream ss3(argv[3]);
    istringstream ss4(argv[4]);
    istringstream ss5(argv[5]);
    istringstream ss6(argv[6]);
    istringstream ss7(argv[7]);
    istringstream ss8(argv[8]);
    istringstream ss9(argv[9]);

    if(!((ss1 >> cleaning_time) && (ss1.eof()) &&
         (ss2 >> cleaning_load_size) && (ss2.eof()) &&
         (ss3 >> clean_shipping_time) && (ss3.eof()) &&
         (ss4 >> clean_shipping_size) && (ss4.eof()) &&
         (ss5 >> dirty_shipping_time) && (ss5.eof()) &&
         (ss6 >> usage_mean) && (ss6.eof()) &&
         (ss7 >> usage_sd) && (ss7.eof()) &&
         (ss8 >> stockpile) && (ss8.eof()) &&
         (ss9 >> total_run_time) && (ss9.eof())) ){
         cout << "One or more of those numbers were not formated correctly" << endl;
         return 1;
    }

    if(cleaning_time < 0){        cout << "load_interval must be >= 0";          return 1;}
    if(cleaning_load_size < 0){   cout << "load_size must be >= 0";              return 1;}
    if(clean_shipping_time <= 0){ cout << "clean_shipment_interval must be > 0"; return 1;}
    if(clean_shipping_size < 0){  cout << "clean_shipment_size must be >= 0";    return 1;}
    if(dirty_shipping_time <= 0){ cout << "dirty_shipment_interval must be > 0"; return 1;}
    if(usage_sd < 0){             cout << "usage_sd must be >= 0";               return 1;}
    if(stockpile < 0){            cout << "stockpile must be >= 0";              return 1;}
    if(total_run_time <= 0){      cout << "run_time must be > 0";                return 1;}

    cout << cleaning_time << " " << cleaning_load_size << " " << clean_shipping_time << " " << clean_shipping_size << " " << dirty_shipping_time << " " << usage_mean << " " << usage_sd << " " << stockpile << " " << total_run_time << "\n";

    /****** Atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> cleaning
        = dynamic::translate::make_dynamic_atomic_model<Laundromat_cleaning, TIME, int, int>("cleaning", move(cleaning_time), move(cleaning_load_size));
    shared_ptr<dynamic::modeling::model> shipping
        = dynamic::translate::make_dynamic_atomic_model<Laundromat_shipping, TIME, int, int>("shipping", move(clean_shipping_time), move(clean_shipping_size));
    shared_ptr<dynamic::modeling::model> hospital
        = dynamic::translate::make_dynamic_atomic_model<Hospital,            TIME, int, double, double, int>("hospital", move(dirty_shipping_time), move(usage_mean), move(usage_sd), move(stockpile));


    /** laundromat coupled model **/

    dynamic::modeling::Ports iports_Laundromat = {typeid(in_dirty)};
    dynamic::modeling::Ports oports_Laundromat = {typeid(out_clean),
                                                  typeid(out_non_full_load),
                                                  typeid(out_load_delayed),
                                                  typeid(out_short_delivery)};

    dynamic::modeling::Models submodels_Laundromat = {cleaning, shipping};
    dynamic::modeling::EICs eics_Laundromat = {
        dynamic::translate::make_EIC<in_dirty, Laundromat_cleaning_defs::dirty>("cleaning"),
    };
    dynamic::modeling::EOCs eocs_Laundromat = {
        dynamic::translate::make_EOC<Laundromat_shipping_defs::shipped,        out_clean>("shipping"),
        dynamic::translate::make_EOC<Laundromat_shipping_defs::short_delivery, out_short_delivery>("shipping"),
        dynamic::translate::make_EOC<Laundromat_cleaning_defs::non_full_load,  out_non_full_load>("cleaning"),
        dynamic::translate::make_EOC<Laundromat_cleaning_defs::load_delayed,   out_load_delayed>("cleaning")
    };
    dynamic::modeling::ICs ics_Laundromat = {
        dynamic::translate::make_IC<Laundromat_cleaning_defs::clean, Laundromat_shipping_defs::clean>("cleaning","shipping")
    };

    shared_ptr<dynamic::modeling::coupled<TIME>> Laundromat = make_shared<dynamic::modeling::coupled<TIME>>(
        "Laundromat", submodels_Laundromat, iports_Laundromat, oports_Laundromat, eics_Laundromat, eocs_Laundromat, ics_Laundromat
    );


    /** top level model **/


    dynamic::modeling::Ports iports_TOP = {};

    dynamic::modeling::Ports oports_TOP = {typeid(out_outage),
                                           typeid(out_non_full_load),
                                           typeid(out_load_delayed),
                                           typeid(out_short_delivery)};

    dynamic::modeling::Models submodels_TOP = {Laundromat, hospital};

    dynamic::modeling::EICs eics_TOP = {};

    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<out_non_full_load,     out_non_full_load>("Laundromat"),
        dynamic::translate::make_EOC<out_load_delayed,      out_load_delayed>("Laundromat"),
        dynamic::translate::make_EOC<out_short_delivery,    out_short_delivery>("Laundromat"),
        dynamic::translate::make_EOC<Hospital_defs::outage, out_outage>("hospital")
    };

    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<out_clean, Hospital_defs::clean>("Laundromat","hospital"),
        dynamic::translate::make_IC<Hospital_defs::dirty, in_dirty>("hospital","Laundromat")
    };

    shared_ptr<dynamic::modeling::coupled<TIME>> TOP = make_shared<dynamic::modeling::coupled<TIME>>("TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP );


    /*************** Loggers *******************/
    static ofstream out_messages("./simulation_results/laundry_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){
            return out_messages;
        }
    };

    static ofstream out_state("./simulation_results/laundry_state.txt");
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

    r.run_until(TIME({total_run_time/(60*60), (total_run_time/60)%60, total_run_time%60}));

    return 0;
}
