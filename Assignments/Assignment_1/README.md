**Public Transport Queue System**

**Objective**
Using C++ and the linear data structures of queues and stacks, this project simulates a queue-based minibus taxi rank system—a common public transport method across Africa. The simulation will involve three types of taxis:

- Short-distance (S)
- Long-distance (L)
- City routes (C)
Passengers arrive at the taxi rank and queue according to their destination. When a taxi becomes available, they board in an orderly manner based on their position in the queue. Each passenger requires some processing time to complete the onboarding process, which includes boarding the taxi and settling in. To test your implementation, download the taxiData.txt file, which contains passenger arrival data. Note that a different dataset will be used during marking.

Your implementation must incorporate linear data structures, specifically queues and stacks, to efficiently manage passenger boarding and taxi departures.

**Important: The data file is located at:**

Windows: C:\\data\\taxiData.txt
Mac/Linux: Update the path accordingly to match your system’s file structure.  However, for the purpose of marking, the path must be in the format as for Windows, suggestion add a try-catch to prevent the program from crashing.
Data Format
Each row in taxiData.txt represents a time instance at the taxi rank and contains three comma-separated values:

timecount, route (S or L or C), boarding_time

Example:
1,S,3

- At time instance 1, a passenger arrives for the Short-distance (S) taxi.
- If no one is currently boarding, the passenger takes 3 time units to board.
- If another passenger is already boarding, they must wait until the taxi is available.

**Assumptions:**

- All taxis have a capacity of five (5) passengers.
- A taxi will leave when full.
- Components
- Passenger Structure

**Each passenger is represented as a structure containing:**

Destination Type: Short-distance, Long-distance, or City route.
Processing Time: Time taken for the passenger to board and for the taxi to fill up.

**Queue and Time Simulation**
Three separate queues represent the different taxi routes.
The simulation runs for a specified duration.
At each time step:
A passenger may arrive and is assigned a queue based on their route.
If a taxi is available for that route, the next passenger boards.
The taxi departs once full.

**Output Format**
The simulation output should be displayed in a tabular format where:

Each row represents a time step.
Column Next displays the new passengers.
Columns S, L anc C display the number of passengers in each queue.
Columns waiting Q S, waiting Q L, waiting Q C display the passengers in the waiting queue that has to wait for the passenger infront of him/her. 
Columns Taxi Capacity for S, L and C indicating the current capacity for the taxis.
