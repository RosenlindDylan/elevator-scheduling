#ifndef HW5_ELEVATOR_HPP
#define HW5_ELEVATOR_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <random>
#include <atomic>
#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>

using namespace std;

const int NUM_FLOORS = 50;
const int NUM_ELEVATORS = 6;
const int MAX_OCCUPANCY = 10;
const int MAX_WAIT_TIME = 500; // milliseconds

mutex mtx;
mutex global_mtx; // for information shared by both the elevator and person
mutex cout_mtx;
vector<pair<int, int>> global_queue; // global elevator queue (floor, destination) // sorry switching to vector made it so much simpler for optimizing
vector<pair<int, int>> elevator_queues[NUM_ELEVATORS]; // switching to vector made it so much simpler for optimizing
atomic<int> num_people_serviced(0);
vector<int> elevator_positions(NUM_ELEVATORS, 0);
vector<bool> elevator_directions(NUM_ELEVATORS, true);
vector<int> global_passengers_serviced(NUM_ELEVATORS, 0);
int npeople; // global variable for the number of people to be serviced

void elevator(int id) {
    int curr_floor = 0;
    int dest_floor = 0;
    int occupancy = 0;

    while (true) {
        int let_off = 0;
        
        // Check current floor: iterate over local queue and see who can get off
        for (auto it = elevator_queues[id].begin(); it != elevator_queues[id].end(); ) {
            if (it->second == curr_floor) {
                // Person gets off because it's their stop
                it = elevator_queues[id].erase(it);
                let_off++;
                occupancy--;
                num_people_serviced++;
            } else {
                ++it;
            }
        }
        
        // Increment people serviced by let_off
        // Decrement occupancy

        if (!elevator_queues[id].empty() && occupancy < MAX_OCCUPANCY) {
            dest_floor = elevator_queues[id][0].second;
            bool going_up = dest_floor > curr_floor;
            
            // Iterate over global queue and pick up people
            global_mtx.lock();
            for (auto it = global_queue.begin(); it != global_queue.end(); ) {
                if (it->first == curr_floor) {
                    if (going_up == (it->second > it->first) && occupancy < MAX_OCCUPANCY) {
                        // Take person
                        elevator_queues[id].push_back(*it);
                        it = global_queue.erase(it);
                        occupancy++;
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
            global_mtx.unlock();

            // Print movement and update floor
            cout_mtx.lock();
            std::cout << "Elevator " << id << " moving from floor " << curr_floor << " to floor " << (going_up ? ++curr_floor : --curr_floor) << std::endl;
            cout_mtx.unlock();

            // Elevator movement delay
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if (!global_queue.empty() && occupancy < MAX_OCCUPANCY) { // Local queue is empty but global isn't 
            // Find a person
            int offset = 0;
            bool found_person = false;

            global_mtx.lock();
            while (!found_person) {
                for (auto it = global_queue.begin(); it != global_queue.end(); ) {
                    if (abs(it->first - curr_floor) <= offset) {
                        // Take person
                        elevator_queues[id].push_back(*it);
                        it = global_queue.erase(it);
                        found_person = true;
                        break;
                    } else {
                        ++it;
                    }
                }
                offset++;
            }
            global_mtx.unlock();

            // Compute direction
            dest_floor = elevator_queues[id][0].second;
            bool going_up = dest_floor > curr_floor;
            
            // Print movement and update floor
            cout_mtx.lock();
            std::cout << "Elevator " << id << " moving from floor " << curr_floor << " to floor " << (going_up ? ++curr_floor : --curr_floor) << std::endl;
            cout_mtx.unlock();

            // Elevator movement delay
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if (occupancy >= MAX_OCCUPANCY) {
            // Compute direction
            dest_floor = elevator_queues[id][0].second;
            bool going_up = dest_floor > curr_floor;

            // Print movement and update floor
            cout_mtx.lock();
            std::cout << "Elevator " << id << " moving from floor " << curr_floor << " to floor " << (going_up ? ++curr_floor : --curr_floor) << std::endl;
            cout_mtx.unlock();

            // Elevator movement delay
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        if (num_people_serviced >= npeople && elevator_queues[id].empty() && occupancy == 0) {
            // Done with everything
            return;
        }
    }
}

void person(int id) {
    static atomic<int> counter(0); // Static counter for unique IDs
    int person_id = counter.fetch_add(1); // Increment and assign ID atomically

    int curr_floor = rand() % NUM_FLOORS;
    int dest_floor = rand() % NUM_FLOORS;
    while (dest_floor == curr_floor) {
        dest_floor = rand() % NUM_FLOORS;
    }

    cout_mtx.lock();
    cout << "Person " << id << " wants to go from floor " << curr_floor << " to floor " << dest_floor << endl;
    cout_mtx.unlock();

    mtx.lock();
    global_queue.push_back({curr_floor, dest_floor});
    mtx.unlock();

    cout_mtx.lock();
    cout << "Person " << id << " on floor " << curr_floor << " requested elevator service to go to floor " << dest_floor << endl;
    cout_mtx.unlock();

    // Simulate moving towards the destination floor
    while (curr_floor != dest_floor) {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    cout_mtx.lock();
    cout << "Person " << id << " arrived at floor " << curr_floor << endl << flush;
    cout_mtx.unlock();
}

#endif