// Name: Saarah Abdelmaged
// Student Number: 18647790A1

#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <map>
#include <iomanip>

using namespace std;

struct Passenger {
    char route;
    int boardingTime;
};

struct Taxi {
    int capacity = 0;
    bool isBoarding = false;
    int remainingBoardTime = 0;
    bool justLeft = false;
};

map<int, vector<Passenger>> readPassengerData(const string& filePath) {
    ifstream file(filePath);
    map<int, vector<Passenger>> arrivals;

    if (!file.is_open()) {
        cerr << "ERROR: Could not open file at path: " << filePath << endl;
        return arrivals;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string timeStr, routeStr, boardStr;

        getline(ss, timeStr, ',');
        getline(ss, routeStr, ',');
        getline(ss, boardStr, ',');

        int time = stoi(timeStr);
        char route = routeStr[0];
        int boardTime = stoi(boardStr);

        arrivals[time].push_back({route, boardTime});
    }

    return arrivals;
}

void printHeader() {
    cout << left << setw(6) << "Time"
         << setw(20) << "New Arrivals"
         << setw(6) << "S" << setw(6) << "L" << setw(6) << "C"
         << setw(12) << "WaitQ S" << setw(12) << "WaitQ L" << setw(12) << "WaitQ C"
         << setw(10) << "Cap S" << setw(10) << "Cap L" << setw(10) << "Cap C" << endl;
    cout << string(100, '-') << endl;
}

void simulateTaxiRank(map<int, vector<Passenger>>& arrivals, int maxTime = 25) {
    queue<Passenger> queueS, queueL, queueC;
    Taxi taxiS, taxiL, taxiC;

    printHeader();

    for (int time = 0; time <= maxTime; ++time) {
        vector<Passenger> newArrivals = arrivals[time];
        stringstream arrivalStr;

        for (Passenger& p : newArrivals) {
            if (p.route == 'S') queueS.push(p);
            else if (p.route == 'L') queueL.push(p);
            else if (p.route == 'C') queueC.push(p);

            arrivalStr << p.route << "(" << p.boardingTime << ") ";
        }

        auto processTaxi = [](queue<Passenger>& q, Taxi& taxi) {
            int waitCount = max((int)q.size() - 1, 0);
        
            // Handle current boarding
            if (taxi.isBoarding) {
                taxi.remainingBoardTime--;
                if (taxi.remainingBoardTime == 0) {
                    taxi.capacity--;  // one seat is taken
                    q.pop();
                    taxi.isBoarding = false;
                }
            }
        
            // Start next boarding if not currently boarding
            if (!taxi.isBoarding && !q.empty()) {
                taxi.remainingBoardTime = q.front().boardingTime;
                taxi.isBoarding = true;
            }
        
            // Taxi has just left
            if (taxi.capacity == 0 && !taxi.justLeft) {
                taxi.justLeft = true; // mark departure
                return waitCount;     // this tick stays at 0 capacity
            }
        
            // New taxi arrives in next step
            if (taxi.justLeft) {
                taxi.capacity = 5;
                taxi.justLeft = false;
            }
        
            return waitCount;
        };
        
        
        
        int waitS = processTaxi(queueS, taxiS);
        int waitL = processTaxi(queueL, taxiL);
        int waitC = processTaxi(queueC, taxiC);

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

int main() {
    string filePath = "./taxiData.txt";
    map<int, vector<Passenger>> arrivals = readPassengerData(filePath);

    simulateTaxiRank(arrivals);
    return 0;
}