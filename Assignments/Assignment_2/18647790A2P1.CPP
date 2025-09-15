//18647790
//Saarah Abdelmaged
 
// COS2611_2024_As2_P1.cpp : This file contains the 'main' function. Program execution begins and ends there

#include <iostream> // for input and output operations
#include <vector> // for using std::vector
#include <ctime> // for clock() in time measurement functions
#include <cmath> // for log2() in time complexity calculation


using namespace std;

// Function: bubbleSort
// Purpose: Sorts a vector of integers in ascending order using the bubble sort algorithm
void bubbleSort(vector<int>& arr) {
    int n = arr.size(); // get size of the array
    // Outer loop for passes over the array
    for (int i = 0; i < n - 1; ++i) {
        // Inner loop to compare adjacent elements
        for (int j = 0; j < n - i - 1; ++j) {
            // If current element is greater than the next, swap them
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]); 
            }
        }
    }
}               

// Function: merge
// Purpose: Helper function for merge sort that merges two sorted subarrays
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1; // size of left subarray
    int n2 = right - mid; // size of right subarray
    vector<int> L(n1), R(n2); // temporary vectors to hold subarrays

    // Copy data into temporary arrays
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left; // index counters
    // Merge the two arrays back into the original
    while (i < n1 && j <n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];

    // Copy any remaining elements of left subarray
    while (i < n1) arr[k++] = L[i++];
    // Copy any remaining elements of right subarray
    while (j < n2) arr[k++] = R[j++];

}

// Function: mergeSort
// Purpose: Recursively sorts a vector of integers using merge sort algorithm
void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2; // find the midpoint
        // Sort first half
        mergeSort(arr, left, mid);
        // Sort second half
        mergeSort(arr, mid + 1, right);
        // Merge the sorted halves
        merge(arr, left, mid, right);
    }
}

// Function: generateRandomArray
// Purpose: Creates a vector of given size filled with random integers [0,999]
vector<int> generateRandomArray(int size) {

    vector<int> arr(size);
    srand(time(0));
    for (int i = 0; i < size; ++i) {
        arr[i] = rand() % 1000; // assign random value in range [0,999]
    }
           
    return arr;

}

// Function: measureSortingTimeFirstSortingAlgorithm
// Purpose: Measures execution time and theoretical time complexity of bubble sort
void measureSortingTimeFirstSortingAlgorithm(vector<int>& arr, double& timeExecuted, double& timeComplexity) {

    //measure start time
    clock_t startTime = clock();
    
    //Call the first sorting algorithm
    bubbleSort(arr);

    //measure end time
    clock_t endTime = clock();

    //calculate executation time in secods for first sorting algorithm
    timeExecuted = (double)(endTime - startTime) / CLOCKS_PER_SEC;

    //time complexity for bubble sort: O(n^2)
    timeComplexity = arr.size() * arr.size(); //this will then be the time complexity - you must provide the code

} //measureSortingTimeFirstSortingAlgorithm


// Function: measureSortingTimeSecondSortingAlgorithm
// Purpose: Measures execution time and theoretical time complexity of merge sort
void measureSortingTimeSecondSortingAlgorithm(vector<int>& arr , double  &timeExecuted, double &timeComplexity   ) {

    //measure start time
    clock_t startTime = clock();

    //Call the second algortihm
    mergeSort(arr, 0, arr.size() - 1);

    //measure end time
    clock_t endTime = clock();

    //calculate executation time in secods for first sorting algorithm
    timeExecuted = (endTime - startTime) / CLOCKS_PER_SEC;
    
    //time complexity for merge sort: O(n log n)
    timeComplexity = arr.size() * log2(arr.size()); //this will then be the time complexity - you must provide the code

} //measureSortingTimeSecondSortingAlgorithm


int main()
{

    // cout << "COS2611 Assessment 2 Project 1 - Skeleton \n";
    // cout << "ARRAYS \n";

    //generate an array with 100 random elements
    // Variables to store measured times and complexities
    double timeExecuted1, timeExecuted2;
    double timeComplexity1, timeComplexity2;

    // Test input sizes for performance comparison
    vector<int> inputSizes = { 100,1000,5000 };

    for (int size : inputSizes) {
        // Generate random array and clone for fair comparison
        vector<int> arr = generateRandomArray(size);
        vector<int> arrCopy = arr;  // Clone for second algorithm

        //measure the sorting time for sorting algorithm
        measureSortingTimeFirstSortingAlgorithm(arr,timeExecuted1,timeComplexity1);
                

        measureSortingTimeSecondSortingAlgorithm(arrCopy,timeExecuted2,timeComplexity2);

        
        //The display of your results will go here.
        cout << "\nInput Size: " << size << endl;

        cout << "Bubble Sort:\n";
        cout << "Execution Time: " << timeExecuted1 << " seconds\n";

        cout << "Merge Sort:\n";
        cout << "Execution Time: " << timeExecuted2 << " seconds\n";

    }

    //the above may not be the best way to code it - it is just to give you a headstart.
    cout << "\nThe time complexity formula for Bubble Sort: O(n^2)" << endl;
    cout << "The time complexity formula for Merge Sort: O(n log n)" << endl;


    return 0;
} //main