import numpy
import os
import decimal
from datetime import datetime
import itertools
import re
import matplotlib.pyplot as plt

def cad_time(line_time):
    if line_time in [None, '', '\n']:
        return None
    time_parts = line_time.split(':')
    return int(time_parts[0])*(60*60*1000) + int(time_parts[1])*(60*1000) + int(time_parts[2])*(1000) + int(time_parts[3])


def laundry_state_output(state_path="./simulation_results/laundry_state.txt"):
    with open(state_path, "r") as state_file:
        while True:
            line_time     = state_file.readline()
            line_cleaning = state_file.readline()
            line_shipping = state_file.readline()
            line_hospital = state_file.readline()

            if not(line_time and line_cleaning and line_shipping and line_hospital):
                return

            time = cad_time(line_time)

            line_cleaning_parts = line_cleaning.split('"')
            cleaning_amount_dirty        = int(line_cleaning_parts[2][len(':'):-len(', ')])
            cleaning_load_size           = int(line_cleaning_parts[4][len(':'):-len(', ')])
            cleaning_load_time_remaining = cad_time(line_cleaning_parts[7])
            cleaning_load_delay_time     = cad_time(line_cleaning_parts[11])

            line_shipping_parts = line_shipping.split('"')
            shipping_amount_clean             = int(line_shipping_parts[2][len(':'):-len(', ')])
            shipping_time_until_next_shipment = cad_time(line_shipping_parts[5])

            line_hospital_parts = line_hospital.split('"')
            hospital_amount_clean             = int(line_hospital_parts[2][len(':'):-len(', ')])
            hospital_amount_dirty             = int(line_hospital_parts[4][len(':'):-len(', ')])
            hospital_outage                   = int(line_hospital_parts[6][len(':'):-len(', ')])
            hospital_time_until_next_shipment = cad_time(line_hospital_parts[9])

            yield { "time":time,
                    "cleaning_amount_dirty":cleaning_amount_dirty,
                    "cleaning_load_size":cleaning_load_size,
                    "cleaning_load_time_remaining":cleaning_load_time_remaining,
                    "cleaning_load_delay_time":cleaning_load_delay_time,
                    "shipping_amount_clean":shipping_amount_clean,
                    "shipping_time_until_next_shipment":shipping_time_until_next_shipment,
                    "hospital_amount_clean":hospital_amount_clean,
                    "hospital_amount_dirty":hospital_amount_dirty,
                    "hospital_outage":hospital_outage,
                    "hospital_time_until_next_shipment":hospital_time_until_next_shipment}

def laundry_message_output(message_path="./simulation_results/laundry_messages.txt"):
    with open(message_path, "r") as message_file:
        time = cad_time(message_file.readline())
        output = {'time':time}
        while True:
            line = message_file.readline()
            if line in [None, '', '\n'] or re.compile("^\d+:\d+:\d+:\d+\n$").match(line):
                yield dict(output)
                time = cad_time(line)
                if(time is None):
                    return
                output = {'time':time}
            elif line.startswith("[Laundromat_cleaning_defs::clean: {"):
                parts = re.split(r"[\{\}]", line)
                output["cleaning_clean"] = int('0'+parts[1])
                if non_full := int('0'+parts[3]):
                    output["cleaning_non_full_load"] = non_full
                if delay := cad_time(parts[5]):
                    output["cleaning_load_delayed"] = delay
            elif line.startswith("[Laundromat_shipping_defs::shipped: {"):
                parts = re.split(r"[\{\}]", line)
                output["shipping_clean"] = int('0'+parts[1])
                if short_delivery := int('0'+parts[3]):
                    output["shipping_short_delivery"] = short_delivery
            elif line.startswith("[Hospital_defs::dirty: {"):
                parts = re.split(r"[\{\}]", line)
                output["hospital_dirty"] = int('0'+parts[1])
                if outage := int('0'+parts[3]):
                    output["hospital_outage"] = outage

def laundry_output():
    return zip(laundry_state_output(), laundry_message_output())


def run(load_interval, load_size, clean_shipment_interval, clean_shipment_size, dirty_shipment_interval, usage_mean, usage_sd, stockpile, run_time):
    os.system(f"./bin/Laundry {load_interval} {load_size} {clean_shipment_interval} {clean_shipment_size} {dirty_shipment_interval} {usage_mean} {usage_sd} {stockpile} {run_time}")
    os.system("mkdir -p csv")
    with open(f"./csv/{load_interval}-{load_size}-{clean_shipment_interval}-{clean_shipment_size}-{dirty_shipment_interval}-{usage_mean}-{usage_sd}-{stockpile}-{run_time}.csv", 'w') as o_file:
        o_file.write("time, cleaning_amount_dirty, cleaning_load_size, cleaning_load_time_remaining, cleaning_load_delay_time, shipping_amount_clean, shipping_time_until_next_shipment, hospital_amount_clean, hospital_amount_dirty, hospital_outage, hospital_time_until_next_shipment, m_cleaning_clean, m_cleaning_non_full_load, m_cleaning_load_delayed, m_shipping_clean, m_shipping_short_delivery, m_hospital_dirty, m_hospital_outage\n");
        for s, m in laundry_output():
            o_file.write(f"{s['time']}, {s['cleaning_amount_dirty']}, {s['cleaning_load_size']}, {s['cleaning_load_time_remaining']}, {s['cleaning_load_delay_time']}, {s['shipping_amount_clean']}, {s['shipping_time_until_next_shipment']}, {s['hospital_amount_clean']}, {s['hospital_amount_dirty']}, {s['hospital_outage']}, {s['hospital_time_until_next_shipment']}, {m.get('cleaning_clean', 0)}, {m.get('cleaning_non_full_load', 0)}, {m.get('cleaning_load_delayed', 0)}, {m.get('shipping_clean', 0)}, {m.get('shipping_short_delivery', 0)}, {m.get('hospital_dirty', 0)}, {m.get('hospital_outage', 0)}\n")
            '''
            o_file.write(f"{s['time']}, ")
            o_file.write(f"{s['cleaning_amount_dirty']}, ")
            o_file.write(f"{s['cleaning_load_size']}, ")
            o_file.write(f"{s['cleaning_load_time_remaining']}, ")
            o_file.write(f"{s['cleaning_load_delay_time']}, ")
            o_file.write(f"{s['shipping_amount_clean']}, ")
            o_file.write(f"{s['shipping_time_until_next_shipment']}, ")
            o_file.write(f"{s['hospital_amount_clean']}, ")
            o_file.write(f"{s['hospital_amount_dirty']}, ")
            o_file.write(f"{s['hospital_outage']}, ")
            o_file.write(f"{s['hospital_time_until_next_shipment']}, ")
            o_file.write(f"{m.get('cleaning_clean', 0)}, ")
            o_file.write(f"{m.get('cleaning_non_full_load', 0)}, ")
            o_file.write(f"{m.get('cleaning_load_delayed', 0)}, ")
            o_file.write(f"{m.get('shipping_clean', 0)}, ")
            o_file.write(f"{m.get('shipping_short_delivery', 0)}, ")
            o_file.write(f"{m.get('hospital_dirty', 0)}, ")
            o_file.write(f"{m.get('hospital_outage', 0)}\n")
            '''

def main():
    set_load_interval           = [60]    #1m
    set_loadsize_scale          = [110]   #list(range(80, 201, 10))
    set_clean_shipment_interval = [60*60] #1h
    set_clean_size_scale        = list(range(80, 201, 10))
    set_dirty_shipment_interval = [90*60] #1.5h
    set_mean                    = list(range(500, 901, 50))
    set_sd                      = list(itertools.chain(range(5), range(5, 100, 5), range(100, 300, 20), range(300, 1000, 75)))
    set_stockpile               = [5000]
    set_run_time                = [7*24*60*60] #1 week

    #count and total_count are used by the progress indicator
    count = 0
    total_count = sum(1 for _ in itertools.product(set_load_interval, set_loadsize_scale, set_clean_shipment_interval, set_clean_size_scale, set_dirty_shipment_interval, set_mean, set_sd, set_stockpile, set_run_time))



    for params in itertools.product(set_load_interval, set_loadsize_scale, set_clean_shipment_interval, set_clean_size_scale, set_dirty_shipment_interval, set_mean, set_sd, set_stockpile, set_run_time):
        load_interval, load_size_scale, clean_shipment_interval, clean_size_scale, dirty_shipment_interval, usage_mean, usage_sd, stockpile, run_time = params
        clean_shipment_size = int((usage_mean*clean_shipment_interval/dirty_shipment_interval)*(clean_size_scale/100))
        load_size = int((clean_shipment_size*load_interval/clean_shipment_interval)*(load_size_scale/100))
        print(f"{count}/{total_count} : {count/total_count}")
        run(load_interval, load_size, clean_shipment_interval, clean_shipment_size, dirty_shipment_interval, usage_mean, usage_sd, stockpile, run_time)
        count += 1

main()
