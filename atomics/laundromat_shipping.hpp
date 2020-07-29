
#ifndef __LAUNDROMAT_SHIPPING_HPP__
#define __LAUNDROMAT_SHIPPING_HPP__


#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>
#include <random>

using namespace cadmium;
using namespace std;

//Port definition
    struct Laundromat_shipping_defs{
        struct clean          : public in_port<int> {};
        struct shipped        : public out_port<int> {};
        struct short_delivery : public out_port<int> {};
    };

    template<typename TIME>
    class Laundromat_shipping{
        public:
            //Parameters to be overwriten when instantiating the atomic model
            int shipping_amount;
            TIME shipping_interval;
            // default constructor
            Laundromat_shipping(){}
            Laundromat_shipping(int _shipping_interval, int _int_shipping_amount) noexcept{
                shipping_interval = TIME({_shipping_interval/(60*60), (_shipping_interval/60)%60, _shipping_interval%60});
                shipping_amount  = _int_shipping_amount;

                state.amount_clean             = 0;
                state.time_until_next_shipment = TIME("00:00:00");
            }

            // state definition
            struct state_type{
                int amount_clean;
                TIME time_until_next_shipment;
            };
            state_type state;
            // ports definition
            using input_ports=std::tuple<typename Laundromat_shipping_defs::clean>;
            using output_ports=std::tuple<typename Laundromat_shipping_defs::shipped,
                                          typename Laundromat_shipping_defs::short_delivery>;

            // internal transition
            void internal_transition() {
                state.time_until_next_shipment = shipping_interval;
                state.amount_clean = max(0, state.amount_clean-shipping_amount);
            }

            // external transition
            void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
                state.time_until_next_shipment = max(TIME("00:00:00"), state.time_until_next_shipment-e);
                for(auto el : get_messages<typename Laundromat_shipping_defs::clean>(mbs)){
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
                get_messages<typename Laundromat_shipping_defs::shipped>(bags).push_back(min(shipping_amount, state.amount_clean));

                //cout << "-" << min(shipping_amount, state.amount_clean) << "\n";

                if(state.amount_clean < shipping_amount){
                    get_messages<typename Laundromat_shipping_defs::short_delivery>(bags).push_back(shipping_amount - state.amount_clean);
                }
                return bags;
            }

            // time_advance function
            TIME time_advance() const {
                return state.time_until_next_shipment;
            }

            friend std::ostringstream& operator<<(std::ostringstream& os, const typename Laundromat_shipping<TIME>::state_type& i) {
                os <<            "{\"amount_clean\":"   << i.amount_clean << ", " <<
                      "\"time_until_next_shipment\":\"" << i.time_until_next_shipment << "\"}";
                return os;
            }
        };


#endif // __LAUNDROMAT_CLEANING_HPP__
