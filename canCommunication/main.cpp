
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <regex>
using namespace std;

struct log {
	string time_stamp{"0"};
	string can{"0"};
	string can_id{"0"};
	string payload{"0"};
	int can_id_count=0;
};

struct obd2 {
	string name{"0"};
	string bit_start{"0"};
	string bit_len{"0"};
	string little_endian{"0"};
	string scale{"0"};
	string offset{"0"};
	string minimum{"0"};
	string maxx{"0"};
	string units{"0"};
	string pid{"0"};
};

void reg(regex reg, string str)
{
	smatch m;
	regex_search(str, m, reg);
	cout << m.str(0) << endl;

}
int setup_obd2(vector<obd2> &obd2_info)
{
	
	std::ifstream in_file;    
	in_file.open("C:\\Users\\wlutz\\Documents\\repo\\willRepo\\canCommunication\\OBD2v1.4.dbc");
	if (!in_file) {
        std::cerr << "Problem opening file" << std::endl;
        return 1;
    }
	std::string line{};
	while (std::getline(in_file, line)) {
		
		obd2 temp_obd2;
		cout << line << endl;
		cout << endl;
		smatch m; // flag type for determing the matching behavior 
		regex regexa("[\\.0-9-]+(?=\\])");
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.maxx = m.str(0);
		cout << "max is: " << m.str(0) << endl; // m.str(0) this will show a string representing entire matched expression

		regexa = R"(\[([.0-9-]+))";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.minimum = m.str(1);
		cout << "min is: " << m.str(1) << endl; 	

		regexa = "[\\.0-9-]+(?=\\))";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.offset = m.str(0);
		cout << "offset is: " << m.str(0) << endl;

		regexa = R"(\(([.0-9-]+))";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.scale = m.str(1);
		cout << "scale is: " << m.str(1) << endl; 	

		regexa = R"(@([\.0-9\+]+))";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.little_endian = m.str(1);
		cout << "little_indian is: " << m.str(1) << endl;
		
		regexa = "[0-9]+(?=\\|)";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.bit_len = m.str(0);
		cout << "bit_len is: " << m.str(0) << endl;

		regexa = R"((([0-9]+)|[0-9]+)@)";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.bit_start = m.str(1);
		cout << "bit_start is: " << m.str(1)  << endl;

		regexa = R"(\"(.*)\")";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.units = m.str(1);
		cout << "Units is: " << m.str(1) << endl; 

		regexa = "SG_\\sS1_PID_(..)_([\\S]+)";
		regex_search(line, m, regexa); // searches pattern regexp in the string "line"
		temp_obd2.pid = m.str(1);
		temp_obd2.name = m.str(2);
		cout << "PID is: " << m.str(1) << " Name is: " << m.str(2) << endl; 

		obd2_info.push_back(temp_obd2);
	}


		
	in_file.close();
	return 0;
}

//bool compareFunction (car_log a, car_log b) {return a.can_id<b.can_id;} 
//compare any way you like, here I am using the default string comparison

int main() {
	vector<obd2> obd2_info;
	setup_obd2(obd2_info);
	vector<log> car_log;
	
	// I created the first element in vector and give it a can_id of 000
	car_log.push_back(log());


	log temp_struct;

	std::ifstream in_file;    
    in_file.open("C:\\Users\\wlutz\\Documents\\repo\\willRepo\\canCommunication\\candump-2021-02-13_154133.log");
    if (!in_file) {
        std::cerr << "Problem opening file" << std::endl;
        return 1;
    }
    std::string line{};
	bool flag{false};

    while (std::getline(in_file, line)) {
		//string sub = line.substr(25, 3); //get CAN_ID from string
		temp_struct.can = line.substr(20, 4);
		temp_struct.can_id = line.substr(25, 3); //get CAN_ID from string
		temp_struct.time_stamp = line.substr(1, 17);
		temp_struct.payload = line.substr(30, 16);
		temp_struct.can_id_count = 0;

		//find car.can_id in vector.  If not there then add it.
		// for(vector<log>::iterator it = car_log.begin(); it != car_log.end(); it++)
		// {
		// 	// int num {3};
		// 	// if(it->can_id==temp_struct.can_id) {
		// 	//  	it->can_id_count++;
		// 	// } 
		// 	if (it == car_log.end()) {
		// 		car_log.push_back(temp_struct);
		// 	}
		// }

		for(auto & it:car_log)
			if(it.can_id == temp_struct.can_id) {
				flag = true;
				it.can_id_count++;
				break;
			} else flag = false;
			
		// add temp_struct to car_log if it is not in car_log
		if(!flag) {
			car_log.push_back(temp_struct);
			flag=false;
		}
    }

	cout << "This is how many unique CAN ID's are in the log." << endl;
	//sort(car_log.begin(),car_log.end(),compareFunction); // sort vector alphabeticaly
	for(auto st : car_log)
		cout << st.can_id << " " << st.can_id_count << endl;
    
	cout << endl << "The number of CAN ID's in the vector: " << car_log.size() << endl;
    in_file.close();
    return 0;
}
