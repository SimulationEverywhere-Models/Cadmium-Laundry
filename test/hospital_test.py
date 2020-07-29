import os
from itertools import groupby

clean = True;

for total_in in [2, 50, 99, 1234, 10000]:
    with open("./input_data/hospital_test_1.txt", 'w') as input_file:
        input_file.write(f"00:00:00 {total_in}")
        

    os.system("./bin/hospital_test ./input_data/hospital_test_1.txt")

    total_out = 0
    with open("./simulation_results/test_hospital_messages.txt") as output_file:
        for line in output_file:
            if line.startswith("[Hospital_defs::dirty: {"):
                l = [int(''.join(i)) for is_digit, i in groupby(line, str.isdigit) if is_digit]
                if len(l) > 0:
                    total_out += l[0]
    
    print(f"{total_in} == {total_out} is {total_in == total_out}")
