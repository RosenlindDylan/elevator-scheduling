#include <iostream>
using namespace std;
#include "hw5-elevator.hpp"

void elevator(int id);
void person(int id);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " npeople" << endl;
        return 1;
    }
    npeople = atoi(argv[1]); // Set the number of people to be serviced

    thread elevators[NUM_ELEVATORS];
    for (int i = 0; i < NUM_ELEVATORS; i++) {
        elevators[i] = thread(elevator, i);
    }

    default_random_engine gen;
    uniform_int_distribution<int> dist(0, MAX_WAIT_TIME);

    for (int i = 0; i < npeople; i++) // iterate over people
    {
        int wait_time = dist(gen); // wait for a random amount of time
        this_thread::sleep_for(chrono::milliseconds(wait_time));
        thread(person, i).detach(); // service person i
    }

    while (num_people_serviced < npeople) {
        this_thread::yield();
    }
    cout_mtx.lock();
    cout << "Job completed!" << endl;
    cout_mtx.unlock();
    for (int i = 0; i < NUM_ELEVATORS; i++)
    {
        elevators[i].join();
    }

    int total_passengers_serviced = 0;
    for (int i = 0; i < NUM_ELEVATORS; i++)
    {
        total_passengers_serviced += global_passengers_serviced[i];
    }
    cout << "Total passengers serviced by all elevators: " << total_passengers_serviced << endl << flush;

    return 0;
}

