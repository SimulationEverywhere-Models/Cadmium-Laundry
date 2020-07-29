This folder contains the Laundromat PDEVS model implemented in Cadmium

/**************************/
/****FILES ORGANIZATION****/
/**************************/

README.txt [This file]
hospital_laundry_pdevs.pdf
makefile

runner.py [A python3 script that can be modified to run the model sequentially with any number of sets of inputs. The output from each simulation is writen to a csv file in ./csv]
plotter.py [A python3 script that reads in data from all results stored in ./csv and can be used/modified to create any arbitrary plot of the data]

atomics [This folder contains atomic models implemented in Cadmium]
	laundromat_cleaning.hpp
	laundromat_shipping.hpp
	hospital.hpp
bin [This folder will be created automatically the first time you compile the poject.
     It will contain all the executables]
build [This folder will be created automatically the first time you compile the poject.
       It will contain all the build files (.o) generated during compilation]
input_data [This folder contains all the input data to run the tests]
	cleaner_test_1.txt
	hospital_test_1.txt
	shipping_test_1.txt
simulation_results [This folder will be created automatically the first time you compile the poject.
                    It will store the outputs from your simulations and tests in Cadmium's native format]
test [This folder contains both the cpp files used to test each atomic, and a python3 script to run each test]
	cleaner_test.cpp  
	hospital_test.cpp  
	shipping_test.cpp
	cleaner_test.py   
	hospital_test.py   
	shipping_test.py

top_model [This folder contains the Laundry top model]	
	main.cpp

csv [This folder is where runner.py puts the condensed output from runnind the model]
    {clean:load_interval}-{clean:max_load}-{shipping:clean_shipment_interval}-{shipping:clean_shipment_size}-{hospital:dirty_shipment_interval}-{hospital:usage_mean}-{hospital:usage_sd}-{hospital:amount_clean}-{run_time}.csv

/********************/
/****INSTRUCTIONS****/
/********************/

0 - hospital_laundry_pdevs.pdf contains the explanation of this model

1 - Update include path in the makefile in this folder and subfolders. You need to update the following lines:
	INCLUDECADMIUM=-I ../../cadmium/include
	INCLUDEDESTIMES=-I ../../DESTimes/include
    Update the relative path to cadmium/include from the folder where the makefile is. You need to take into account where you copied the folder during the installation process
	Example: INCLUDECADMIUM=-I ../../cadmium/include
	Do the same for the DESTimes library
    NOTE: if you follow the step by step installation guide you will not need to update these paths.
2 - Compile the project and the tests
	1 - Open the terminal (Ubuntu terminal for Linux and Cygwin for Windows) in the this folder
	3 - To compile the project and the tests, type in the terminal:
			make clean; make all
3 - Run subnet test
	1 - Open the terminal in this folder. 
	2 - To run the test, "python3 ./test/NAME_OF_YOUR_TEST.py"
	3 - The output of your test should pop out into the terminal as a tes of equalities. The test passes if all of these equalities are true.
    
5 - Run the top model manually
	1 - Open the terminal (Ubuntu terminal for Linux and Cygwin for Windows) in this folder.
	3 - To run the model, type in the terminal "./bin/NAME_OF_THE_COMPILED_FILE NAME_OF_THE_INPUT_FILE". For this test you need to type:
		./bin/Laundry (for Windows: ./bin/Laundry.exe) followed by the inputs that you intend to give
    3.1 - For instructions of how to format the input arguments, run the program with no arguments.
	4 - To check the output of the model, go to the folder simulation_results and open "laundry_messages.txt" and "laundry_state.txt"

6 - Run the top model using runner.py
	1 - movify ./runner.py's main function to call run with the integer arguments that you want
        1.1 - runner.py expects to run "./bin/Laundry " followed by the set of arguments.
        1.2 - runner.py expects the output to land in "./simulation_results/laundry_state.txt" and "./simulation_results/laundry_messages.txt".
        1.3 - runner will override past runs that have the exact same arguments as current runs, if you want to run with the same arguemnts, you will need to change this
	2 - run python3 ./runner.py in your terminal from this folder
	3 - read the output from ./csv with a filename that reflects the inputs given to the program

7 - Plot the output from runner.py using plotter.py
    1 - modify plotter.py to read values that you cair about from the output files. 
        1.1 - Note that absent optional message contents are set to 0 in these files
        1.2 - All values are integers
        1.3 - The first line in the csv is a title line, and should not be parsed
        1.4 - The implementation of plotter uses matplotlib's pyplot, but you don't have to
    2 - Run python3 ./plotter.py in this folder
    
