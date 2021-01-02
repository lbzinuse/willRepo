
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

struct log {
	string time_stamp;
	string can;
	string can_id;
	string payload;
	int can_id_count=0;
}car;

//bool compareFunction (car_log a, car_log b) {return a.can_id<b.can_id;} 
//compare any way you like, here I am using the default string comparison

int main() {

	vector<log> car_log;
	int num{0};
    std::ifstream in_file;
    in_file.open("candump-2020-12-27_183804.log");
    if (!in_file) {
        std::cerr << "Problem opening file" << std::endl;
        return 1;
    }
    std::string line{};
	//vector<string> id;
	

    while (std::getline(in_file, line)) {
		//string sub = line.substr(25, 3); //get CAN_ID from string
		car.can_id = line.substr(25, 3); //get CAN_ID from string

		//find car.can_id in vector.  If not there then add it.
		if(find(car_log.begin(), car_log.end(), car_log.) == car_log.end())
			car_log.push_back(car);
    }

	cout << "This is how many unique CAN ID's are in the log." << endl;
	//sort(car_log.begin(),car_log.end(),compareFunction); // sort vector alphabeticaly
	for(auto st : car_log)
		cout << st.can_id << " ";
    
	cout << endl << "The number of CAN ID's in the vector: " << car_log.size() << endl;
    in_file.close();
    return 0;
}

// // can transmit

// //#include <stdio.h>
// //#include <stdlib.h>
// //#include <iosteam.h>
// #include <string>
// #include <vector>
// #include <iostream>
// #include <fstream>
// using namespace std;

// struct log {
// 	string time_stamp;
// 	string can;
// 	string can_id;
// 	string payload;

// }car_log;

// int main(int argc, char **argv)
// {

// 	fstream in_file("candump-2020-12-27_183804.log", ios::in);
// 	if (!in_file) {
//         cerr << "Problem opening file" << endl;
//         return 1;
//     }

// 	// while (getline(in_file, line)) {

// 	// }
// 	char line {};
// 	while (in_file.get(line)) {
// 		cout << line;
// 	}
// 	string ln;
// 	while (in_file.getline(ln, sizeof(ln)))
// 	{

// 	}
// 	// for(int x=0;x<6;x++)
// 	// {
// 	// 	in_file.get(line);
// 	// 	cout << line;
// 	// }
// 	car_log.time_stamp = 1;
// 	car_log.can = 2;
// 	car_log.can_id = 3;
// 	car_log.payload = 4;

// 	vector<log> vec_log;
// 	vec_log.push_back(car_log);

// 	in_file.close();

// 	return 0;
// }