// can transmit

#include <stdio.h>
#include <stdlib.h>
//#include <iosteam.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

struct log {
	string time_stamp;
	string can;
	string can_id;
	string payload;

}car_log;

int main(int argc, char **argv)
{

	fstream in_file("candump-2020-12-27_183804.log", ios::in);
	if (!in_file) {
        cerr << "Problem opening file" << endl;
        return 1;
    }

	// while (getline(in_file, line)) {

	// }
	char line {};
	while (in_file.get(line)) {
		cout << line;
	}

	car_log.time_stamp = 1;
	car_log.can = 2;
	car_log.can_id = 3;
	car_log.payload = 4;

	vector<log> vec_log;
	vec_log.push_back(car_log);

	in_file.close();

	return 0;
}