
#ifndef __LAUNDROMAT_CLEANING_HPP__
#define __LAUNDROMAT_CLEANING_HPP__


#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

using namespace cadmium;
using namespace std;

//Port definition
    struct Laundromat_cleaning_defs{
        struct dirty         : public in_port<int> {};
        struct clean         : public out_port<int> {};
        struct non_full_load : public out_port<int> {};
        struct load_delayed  : public out_port<string> {};
    };

    template<typename TIME>
    class Laundromat_cleaning{
        public:
            //Parameters to be overwriten when instantiating the atomic model
            int max_load;
            TIME load_time;
            // default constructor
            Laundromat_cleaning(){}
            Laundromat_cleaning(int _load_time, int _max_load) noexcept{
                max_load  = _max_load;
                load_time = TIME({_load_time/(60*60), (_load_time/60)%60, _load_time%60});

                state.amount_dirty        = 0;
                state.load_size           = 0;
                state.load_time_remaining = TIME("00:00:00");
                state.load_delay_time     = TIME("00:00:00");
            }

            // state definition
            struct state_type{
                int amount_dirty;
                int load_size;
                TIME load_time_remaining;
                TIME load_delay_time;
            };
            state_type state;
            // ports definition
            using input_ports=std::tuple<typename Laundromat_cleaning_defs::dirty>;
            using output_ports=std::tuple<typename Laundromat_cleaning_defs::clean,
                                          typename Laundromat_cleaning_defs::non_full_load,
                                          typename Laundromat_cleaning_defs::load_delayed>;


            // internal transition
            void internal_transition() {
                if(state.amount_dirty > 0){/*start a new load*/
                	state.load_time_remaining = load_time;
                	state.load_size = min(state.amount_dirty, max_load);
                	state.amount_dirty -= state.load_size;

                }else{/*wait for laundry to arive*/
                	state.load_size = 0;
                	state.load_time_remaining = TIME("00:00:00");
                }
                state.load_delay_time = TIME("00:00:00");
            }

            // external transition
            void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
                for(auto el : get_messages<typename Laundromat_cleaning_defs::dirty>(mbs)){
                    state.amount_dirty += el;
                }
                if(state.load_size > 0){/*we are running a load right now*/
                	state.load_time_remaining = max(TIME("00:00:00"), state.load_time_remaining-e);
                }else{/*we are not already running a load*/
                	state.load_delay_time += e;
                    if(state.amount_dirty > 0){/*starting a new load*/
                        state.load_time_remaining = load_time;
                        state.load_size = min(state.amount_dirty, max_load);
                        state.amount_dirty -= state.load_size;
                    }
                }
            }

            // confluence transition
            void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
                internal_transition();
                external_transition(e, std::move(mbs));
            }

            // output function
            typename make_message_bags<output_ports>::type output() const {
                typename make_message_bags<output_ports>::type bags;
                get_messages<typename Laundromat_cleaning_defs::clean>(bags).push_back(state.load_size);
                //cout << "-" << state.load_size << "\n";
                if(state.load_delay_time > TIME("00:00:00")){
                    ostringstream ss;
                    ss << state.load_delay_time;
                    get_messages<typename Laundromat_cleaning_defs::load_delayed>(bags).push_back(ss.str());
                }
                if(state.load_size < max_load){
                    get_messages<typename Laundromat_cleaning_defs::non_full_load>(bags).push_back(max_load - state.load_size);
                }
                return bags;
            }

            // time_advance function
            TIME time_advance() const {
              if (state.load_size > 0) {
                return max(TIME("00:00:00"), state.load_time_remaining);
              }else {
                return std::numeric_limits<TIME>::infinity();
              }
            }

            friend std::ostringstream& operator<<(std::ostringstream& os, const typename Laundromat_cleaning<TIME>::state_type& i) {
                os << "{\"amount_dirty\":"   << i.amount_dirty << ", " <<
                          "\"load_size\":"   << i.load_size << ", " <<
                "\"load_time_remaining\":\"" << i.load_time_remaining << "\", " <<
                    "\"load_delay_time\":\"" << i.load_delay_time << "\"}";
                return os;
            }
        };


#endif // __LAUNDROMAT_CLEANING_HPP__
