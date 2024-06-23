/* 
*  Armon Wilson
*  SNHU
*  CS-300 Data Structures & Algorithms
*  Project 2
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <climits> // For UINT_MAX
#include <algorithm>
#include <functional>

using namespace std;

const unsigned int DEFAULT_SIZE = 179;

// Course Structure 
struct Course {
    string courseNumber; // Unique identifier 
    string title;
    vector<string> prerequisites;

    Course() {} // Default constructor

    Course(string courseNum, string titl) : courseNumber(courseNum), title(titl) {}

    void addPrerequisite(const string& prereq) {
        prerequisites.push_back(prereq);
    }
};

/// Hash Table class definition
class HashTable {
private:
    struct Node {
        Course course;
        unsigned int key;
        Node* next;

        Node() : key(UINT_MAX), next(nullptr) {}
        Node(Course aCourse) : Node() { course = aCourse; }
        Node(Course aCourse, unsigned int aKey) : Node(aCourse) { key = aKey; }
    };

    vector<Node> nodes;
    unsigned int tableSize = DEFAULT_SIZE;
    unsigned int hash(const string& courseNumber);

public:
    HashTable();
    HashTable(unsigned int size);
    virtual ~HashTable();
    void Insert(Course course);
    void Remove(string courseId);
    Course* Search(string courseId);
    void printAll();
    size_t Size();
};
    
/**
 * Default constructor
 */
HashTable::HashTable() {
    nodes.resize(tableSize);
}

/**
 * Constructor for specifying size of the table
 * Use to improve efficiency of hashing algorithm
 * by reducing collisions without wasting memory.
 */
HashTable::HashTable(unsigned int size) {
    this->tableSize = size;
    nodes.resize(tableSize);
}

/**
 * Destructor
 */
HashTable::~HashTable() {
    for (unsigned int i = 0; i < tableSize; ++i) {
        Node* current = &nodes[i];   // Get the head of the linked list in this bucket
        while (current != nullptr) {  // Traverse the linked list
            Node* temp = current;     // Store current node to delete later
            current = current->next; // Move to the next node
            //delete temp;           // Delete the stored node *** This is causing an error ***
        }
    }
}

// Hash function (Improved)
unsigned int HashTable::hash(const string& courseNumber) {
    std::hash<std::string> hasher; // Use std::hash explicitly
    return hasher(courseNumber) % tableSize;
}

/**
 * Insert a bid
 */
void HashTable::Insert(Course course) {
    unsigned int key = hash(course.courseNumber);

    // Create a new node
    Node* newNode = new Node(course, key);

    // Insert at the head of the linked list
    newNode->next = nodes[key].next;
    nodes[key].next = newNode;
}

/**
 * Remove a bid
 */
void HashTable::Remove(string courseNumber) {
    unsigned int key = hash(courseNumber);

    // Find the course to remove
    Node* current = nodes[key].next; 
    Node* previous = &nodes[key]; 
    while (current != nullptr && current->course.courseNumber != courseNumber) {
        previous = current;
        current = current->next;
    }

    // If the course was found
    if (current != nullptr) {
        previous->next = current->next; // Remove the node
        delete current; // Free memory
    }
}

Course* HashTable::Search(string courseNumber) {
    unsigned int key = hash(courseNumber);
    Node* current = nodes[key].next; 

    while (current != nullptr) {
        if (current->course.courseNumber == courseNumber) {
            return &current->course; // Return a pointer to the Course object
        }
        current = current->next;
    }
    return nullptr; // Return nullptr if not found
}


// Print course list
void HashTable::printAll() {

    cout << endl;
    cout << "CS Course List:" << endl;

    // Create a vector to hold course pointers
    vector<Course*> allCourses;

    // Iterate through the hash table to collect courses
    for (const Node& node : nodes) {
        Node* current = node.next;
        while (current != nullptr) {
            allCourses.push_back(&current->course); // Store pointer to Course
            current = current->next;
        }
    }

    // Insertion sort the vector of course pointers
    for (size_t i = 1; i < allCourses.size(); ++i) {
        Course* key = allCourses[i];
        int j = i - 1;
        while (j >= 0 && allCourses[j]->courseNumber > key->courseNumber) {
            allCourses[j + 1] = allCourses[j];
            --j;
        }
        allCourses[j + 1] = key;
    }

    // Print the sorted courses
    for (const Course* course : allCourses) {
        cout << course->courseNumber << " | " << course->title << endl;
    }
    cout << endl;
}

//Function to load data from file path into a hash table
HashTable loadCourseDataIntoHashTable(HashTable& courseTable, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return HashTable();
    }

    vector<Course> tempCourses; // Store courses temporarily

    // Load courses 
    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            cerr << "Warning: Empty line in file." << endl;
            continue;
        }

        stringstream ss(line);
        string courseNumber, courseTitle, prerequisite;
        getline(ss, courseNumber, ',');
        getline(ss, courseTitle, ',');

        if (ss.fail()) {
            cerr << "Error: Invalid format (missing course number or title) on line: " << line << endl;
            continue;
        }

        Course newCourse(courseNumber, courseTitle);

        // Store prerequisites 
        while (getline(ss, prerequisite, ',')) {
            if (!prerequisite.empty()) {  // Check for empty prerequisite
                newCourse.addPrerequisite(prerequisite);
            }
        }

        tempCourses.push_back(newCourse);
    }

    file.close();

    // Insert all courses into the hash table
    for (const Course& course : tempCourses) {
        courseTable.Insert(course);
    }

    //Validate prerequisites 
    for (const Course& course : tempCourses) {
        for (const string& prereq : course.prerequisites) {
            if (!courseTable.Search(prereq)) {
                cerr << "Error: Prerequisite " + prereq + " for course " + course.courseNumber + " does not exist." << endl;
            }
        }
    }

    return courseTable;
}
       
// Function to search and print course details
void searchCourseHashTable(HashTable& courseTable, const string& courseNumber) {
    Course* courseFound = courseTable.Search(courseNumber); // Get pointer to Course
    if (courseFound) { // Check if course was found
        cout << endl;
        cout << "Course Information:" << endl;
        if (!courseFound->prerequisites.empty()) {
            cout << courseFound->courseNumber << ", " << courseFound->title << endl << "\nPrerequisites:" << endl;
            for (const string& prereq : courseFound->prerequisites) {
                cout << prereq << endl;
            }
            cout << endl;
        }
    }
    else {
        cout << "Error: Course not found." << endl;
    }
}

size_t HashTable::Size() {
    size_t count = 0;
    for (const Node& node : nodes) {
        Node* current = node.next;
        while (current != nullptr) {
            ++count;
            current = current->next;
        }
    }
    return count;
}

int main() {
    
    HashTable courseTable;

    //TODO: Fix Filepath crash
    string filePath = "CS 300 ABCU_Advising_Program_Input.csv";

    cout << "Welcome to the Course Planner." << endl;

    bool exit = false;
    while (!exit) {
        cout << endl;
        cout << "Menu:" << endl;
        cout << "1. Load Data Structure." << endl;
        cout << "2. Print Course List." << endl;
        cout << "3. Print Course." << endl;
        cout << "9. Exit" << endl;
        cout << endl;
        cout << "What would you like to do?" << endl;

        string choice;
        cin >> choice;
        
        if (choice == "1") {
            cout << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            courseTable = loadCourseDataIntoHashTable(courseTable, filePath); // Assign the returned HashTable
            if (courseTable.Size() > 0) { // Check if loading was successful
                cout << "Course data loaded successfully." << endl;
            }
            else {
                cout << "Error loading course data." << endl;
            }
            cout << endl;
        }

        else if (choice == "2") {
            if (courseTable.Size() == 0) {
                cout << "Error: Please load data first" << endl;
            }
            else {courseTable.printAll();}
        }

        else if (choice == "3") {
            if (courseTable.Size() == 0) {
                cout << "Error: Please load data first." << endl;
            }
            else {
                cout << "Enter course number:" << endl;
                string courseNumber;
                cin >> courseNumber;
                transform(courseNumber.begin(), courseNumber.end(), courseNumber.begin(), ::toupper);
                searchCourseHashTable(courseTable, courseNumber);
            }
        }

        else if (choice == "9") { 
            exit = true; 
        }

        else {
            cout << choice << " is not a valid option." << endl;
        }
    }
}