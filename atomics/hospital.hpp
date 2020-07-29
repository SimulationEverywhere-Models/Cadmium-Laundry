/**
* Cristina Ruiz Martin
* ARSLab - Carleton University
*
* receiver:
* Cadmium implementation of CD++ Receiver atomic model
*/

#ifndef __HOSPITAL_HPP__
#define __HOSPITAL_HPP__


#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

using namespace cadmium;
using namespace std;

//Port definition
    struct Hospital_defs{
        struct clean  : public in_port<int> { };
        struct dirty  : public out_port<int> { };
        struct outage : public out_port<int> { };
    };

    template<typename TIME>
    class Hospital{
        public:
            //Parameters to be overwriten when instantiating the atomic model
            TIME   shipping_interval;

            std::random_device rd{};
            std::mt19937 gen{rd()};
            std::normal_distribution<> usage_dist;

            // default constructor
            Hospital(){}
            Hospital(int _shipping_interval, double _usage_mean, double _usage_sd, int _amount_clean) noexcept{
                shipping_interval = TIME({_shipping_interval/(60*60), (_shipping_interval/60)%60, _shipping_interval%60});
                usage_dist = std::normal_distribution<>(_usage_mean, _usage_sd);

                state.amount_clean = _amount_clean;
                state.amount_dirty = 0;
                state.outage = 0;
                state.time_until_next_shipment = TIME("00:00:00");
            }

            // state definition
            struct state_type{
                int amount_clean;
                int amount_dirty;
                int outage;
                TIME time_until_next_shipment;
            };
            state_type state;

            // ports definition
            using input_ports=std::tuple<typename Hospital_defs::clean>;
            using output_ports=std::tuple<typename Hospital_defs::dirty,
                                          typename Hospital_defs::outage>;

            // internal transition
            void internal_transition() {
                state.time_until_next_shipment = shipping_interval;

                int amount_to_dirty = max(0, (int)rint(usage_dist(gen)));
                state.amount_dirty = min(amount_to_dirty, state.amount_clean);
                state.outage = amount_to_dirty - state.amount_dirty;
                state.amount_clean -= state.amount_dirty;
            }

            // external transition
            void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
                state.time_until_next_shipment -= e;

                for(auto el : get_messages<typename Hospital_defs::clean>(mbs)){
                    state.amount_clean += el;
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

                get_messages<typename Hospital_defs::dirty>(bags).push_back(state.amount_dirty);
                if(state.outage > 0){
                    get_messages<typename Hospital_defs::outage>(bags).push_back(state.outage);
                }
                return bags;
            }

            // time_advance function
            TIME time_advance() const {
              return state.time_until_next_shipment;
            }

            friend std::ostringstream& operator<<(std::ostringstream& os, const typename Hospital<TIME>::state_type& i) {
                os << "{\"amount_clean\":"   << i.amount_clean << ", " <<
                       "\"amount_dirty\":"   << i.amount_dirty << ", " <<
                             "\"outage\":"   << i.outage << ", " <<
           "\"time_until_next_shipment\":\"" << i.time_until_next_shipment << "\"}";
                return os;
            return os;
            }
        };


#endif // __HOSPITAL_HPP__
