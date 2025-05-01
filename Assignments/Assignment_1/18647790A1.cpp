// Name: Saarah Abdelmaged
// Student Number: 18647790A1

#include <iostream>  // for standard input and output (like cout and cerr)
#include <fstream>  // for reading from files
#include <sstream> // for breaking strings into parts
#include <queue> // for using queues (FIFO structure)
#include <vector> // for using vectors (dynamic arrays)
#include <map> // for key-value pairs to store passenger arrivals by time
#include <iomanip> // for formatting output (like setw)

using namespace std;

// Structure to represent a passenger
struct Passenger {
    char route;
    int boardingTime;
};

// Structure to represent a taxi
struct Taxi {
    int capacity = 0; // number of available seats (max is 5)
    bool isBoarding = false; // is someone currently boarding this taxi?
    int remainingBoardTime = 0; // countdown for how long the current passenger takes to board
    bool justLeft = false;  // did the taxi just leave and we’re waiting for a new one?
};

// Function to read the data file and return a map of time to passengers arriving then
map<int, vector<Passenger>> readPassengerData(const string& filePath) {
    ifstream file(filePath); // try open the file
    map<int, vector<Passenger>> arrivals; // map to hold passengers by arrival time

    if (!file.is_open()) {
        cerr << "ERROR: Could not open file at path: " << filePath << endl;
        return arrivals; // return empty map if failed
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);  // used to split the line by commas
        string timeStr, routeStr, boardStr;

        // Extract time, route and boarding time
        getline(ss, timeStr, ',');
        getline(ss, routeStr, ',');
        getline(ss, boardStr, ',');

        // Convert string values to appropriate data types
        int time = stoi(timeStr); // convert to integer
        char route = routeStr[0]; // get the first char
        int boardTime = stoi(boardStr); // convert to integer

        // Add this passenger to the correct time slot
        arrivals[time].push_back({route, boardTime});
    }

    return arrivals;
}

// Function to print the table headers for each time step
void printHeader() {
    cout << left << setw(6) << "Time"
         << setw(20) << "New Arrivals"
         << setw(6) << "S" << setw(6) << "L" << setw(6) << "C"
         << setw(12) << "WaitQ S" << setw(12) << "WaitQ L" << setw(12) << "WaitQ C"
         << setw(10) << "Cap S" << setw(10) << "Cap L" << setw(10) << "Cap C" << endl;
    cout << string(100, '-') << endl; // line separator
}

// Function that simulates the entire taxi rank system over time
void simulateTaxiRank(map<int, vector<Passenger>>& arrivals, int maxTime = 25) {
    // Queues for each route
    queue<Passenger> queueS, queueL, queueC;
    Taxi taxiS, taxiL, taxiC; // taxis for each route

    printHeader(); // Print the column headers

    for (int time = 0; time <= maxTime; ++time) {
        // Get new arrivals for the current time (if any)
        vector<Passenger> newArrivals = arrivals[time];
        stringstream arrivalStr;

        // Push each new passenger into the correct queue
        for (Passenger& p : newArrivals) {
            if (p.route == 'S') queueS.push(p);
            else if (p.route == 'L') queueL.push(p);
            else if (p.route == 'C') queueC.push(p);

            arrivalStr << p.route << "(" << p.boardingTime << ") ";
        }

        // Lambda function to process a single taxi's behavior each time step
        auto processTaxi = [](queue<Passenger>& q, Taxi& taxi) {
            int waitCount = max((int)q.size() - 1, 0); // number of waiting passengers (not including current boarding)
        
            // If someone is already boarding, reduce their boarding time
            if (taxi.isBoarding) {
                taxi.remainingBoardTime--;
                if (taxi.remainingBoardTime == 0) {
                    taxi.capacity--;  // one seat is taken
                    q.pop(); // remove passenger from queue
                    taxi.isBoarding = false;
                }
            }
        
            // Start next boarding if not currently boarding
            if (!taxi.isBoarding && !q.empty()) {
                taxi.remainingBoardTime = q.front().boardingTime;
                taxi.isBoarding = true;
            }
        
            // If taxi is empty and hasn’t left yet, let it "depart"
            if (taxi.capacity == 0 && !taxi.justLeft) {
                taxi.justLeft = true; // mark departure
                return waitCount;     // this tick stays at 0 capacity
            }
        
            // If taxi just left, bring in a new one with full capacity
            if (taxi.justLeft) {
                taxi.capacity = 5;
                taxi.justLeft = false;
            }
        
            return waitCount;
        };
        
        
        // Process each taxi queue and get the number of people still waiting
        int waitS = processTaxi(queueS, taxiS);
        int waitL = processTaxi(queueL, taxiL);
        int waitC = processTaxi(queueC, taxiC);

        // Print all info for the current time step
        cout << left << setw(6) << time
             << setw(20) << arrivalStr.str()
             << setw(6) << queueS.size()
             << setw(6) << queueL.size()
             << setw(6) << queueC.size()
             << setw(12) << waitS
             << setw(12) << waitL
             << setw(12) << waitC
             << setw(10) << taxiS.capacity
             << setw(10) << taxiL.capacity
             << setw(10) << taxiC.capacity
             << endl;
    }
}

// Main function - starting point of the program
int main() {
    string filePath = "C:\\data\\taxiData.txt"; // path to the data file
    map<int, vector<Passenger>> arrivals = readPassengerData(filePath); // load all passengers from file

    simulateTaxiRank(arrivals); // start the simulation
    return 0;
}