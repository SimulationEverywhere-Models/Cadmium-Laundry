import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

#import csv
import os

def main():
    i = 0
    total_files = len(os.listdir("./csv"))

    ax = plt.figure(figsize=(10,10)).add_subplot(111, projection='3d')
    ax.set_xlabel('Consumption mean / Consumption standard deviation')
    ax.set_ylabel('Production rate / Consumption mean')
    ax.set_zlabel('Minimum stockpile')

    x = [[],[],[],[]]
    y = [[],[],[],[]]
    z = [[],[],[],[]]
    #plt.title("What to call this plot")

    for root, dirs, files in os.walk("./csv"):
        for name in files:
            #print(os.path.join(root, name))

            #comment out values that you do not need to speed up the plotter
            name_parts = name[:-len(".csv")].split('-')

            load_interval           = int(name_parts[0])
            load_size               = int(name_parts[1])
            clean_shipment_interval = int(name_parts[2])
            clean_shipment_size     = int(name_parts[3])
            dirty_shipment_interval = int(name_parts[4])
            usage_mean              = int(name_parts[5])
            usage_sd                = int(name_parts[6])
            stockpile               = int(name_parts[7])
            #run_time                = int(name_parts[8])

            min_clean_pile = stockpile
            max_outage = 0

            with open(os.path.join(root, name)) as csv:
                csv.readline()
                for line in csv:
                    #comment out values that you do not need to speed up the plotter
                    line_parts = line.split(', ')

                    #time                              = int(line_parts[0])
                    #cleaning_amount_dirty             = int(line_parts[1])
                    #cleaning_load_size                = int(line_parts[2])
                    #cleaning_load_time_remaining      = int(line_parts[3])
                    #cleaning_load_delay_time          = int(line_parts[4])
                    #shipping_amount_clean             = int(line_parts[5])
                    #shipping_time_until_next_shipment = int(line_parts[6])
                    hospital_amount_clean             = int(line_parts[7])
                    #hospital_amount_dirty             = int(line_parts[8])
                    #hospital_outage                   = int(line_parts[9])
                    #hospital_time_until_next_shipment = int(line_parts[10])
                    #m_cleaning_clean                  = int(line_parts[11])
                    #m_cleaning_non_full_load          = int(line_parts[12])
                    #m_cleaning_load_delayed           = int(line_parts[13])
                    #m_shipping_clean                  = int(line_parts[14])
                    #m_shipping_short_delivery         = int(line_parts[15])
                    #m_hospital_dirty                  = int(line_parts[16])
                    m_hospital_outage                 = int(line_parts[17])

                    min_clean_pile = min(hospital_amount_clean, min_clean_pile)
                    max_outage     = max(m_hospital_outage, max_outage)
            t_x = usage_sd/usage_mean
            t_y = min((load_size/load_interval),(clean_shipment_size/clean_shipment_interval)) / (usage_mean/dirty_shipment_interval)
            key = (0, 2)[t_y >= 1]
            if max_outage > 0:
                x[key].append(t_x)
                y[key].append(t_y)
                z[key].append(-max_outage)
            else:
                x[key+1].append(t_x)
                y[key+1].append(t_y)
                z[key+1].append(min_clean_pile)


            if (i := i+1)%1000 == 0: #working in batches because appending to very long lists is slow in python
                print(i, total_files)#A crude progress indicator
                ax.scatter(x[0], y[0], z[0], marker='o', color=(1, 0, 0))# <1 production, with outage
                ax.scatter(x[1], y[1], z[1], marker='o', color=(1, 1, 0))# <1 production, with no outage
                ax.scatter(x[2], y[2], z[2], marker='o', color=(1, 0, 1))# >1 production, with outage
                ax.scatter(x[3], y[3], z[3], marker='o', color=(0, 1, 0))# >1 production, with no outage
                x = [[],[],[],[]]
                y = [[],[],[],[]]
                z = [[],[],[],[]]

    ax.scatter(x[0], y[0], z[0], marker='o', color=(1, 0, 0))# <1 production, with outage
    ax.scatter(x[1], y[1], z[1], marker='o', color=(1, 1, 0))# <1 production, with no outage
    ax.scatter(x[2], y[2], z[2], marker='o', color=(1, 0, 1))# >1 production, with outage
    ax.scatter(x[3], y[3], z[3], marker='o', color=(0, 1, 0))# >1 production, with no outage
    plt.show()
    return

main()
