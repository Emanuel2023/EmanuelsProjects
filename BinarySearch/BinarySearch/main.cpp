//
//  main.cpp
//  BinarySearch
//
//  Created by Emanuel Contreras on 1/2/25.
//
#include <iostream>
#include <cmath>

using namespace std;

int BinarySearchLogicFunction(int* arrayList, int key, int size) {
    int low = 0;
    int high = size - 1;

    while (high >= low) {
        int mid = (high + low) / 2; // Calculate the middle index
        if (arrayList[mid] == key) {
            return mid; // Key found, return its index
        } else if (arrayList[mid] < key) {
            low = mid + 1; // Search in the right half
        } else {
            high = mid - 1; // Search in the left half
        }
    }
    return -1; // Key not found
}

int main() {
    int List[] = {1, 4, 7, 11, 15, 23, 39, 87};
    int size = sizeof(List) / sizeof(List[0]);

    // Print the values of the array
    std::cout << "The list is as follows:" << endl;
    for (int i = 0; i < size; ++i) {
        std::cout << List[i] << " ";
    }
    std::cout << endl;

    // Prompt the user to enter the key
    std::cout << "Enter the key you would wish to search for: ";
    int key;
    cin >> key;

    // Call the Binary Search function
    int keyIndex = BinarySearchLogicFunction(&List[0], key, size);

    // Print the result
    if (keyIndex != -1) {
        std::cout << "Found " << key << " at index " << keyIndex << "." << endl;
    } else {
        std::cout << key << " was not found." << endl;
    }

    return 0;
}
