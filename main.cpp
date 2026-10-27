
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <cctype>
using namespace std;

const int ROWS = 10;
const int COLS = 4;
const int MAX_PASSENGERS = ROWS * COLS;
const int QUEUE_SIZE = 50;
const int STACK_SIZE = 100;

// Seat array and ticket counters
int seats[ROWS][COLS];
int nextTicketId = 1001;
int nextGroupId = 1;

// Passenger structure
struct Passenger {
    int ticketId;
    string name;
    int age;
    int row, col;
    int groupId;
};

Passenger booked[MAX_PASSENGERS];
int passengerCount = 0;

// Circular queue for counter customers
Passenger counterQueue[QUEUE_SIZE];
int front = 0, rear = -1, queueCount = 0;

// Stack stores group IDs for undo
int history[STACK_SIZE];
int top = -1;

// Singly linked-list waiting list
struct Node {
    Passenger data;
    Node* next;
};

Node* waitFront = nullptr;
Node* waitRear = nullptr;

// ---------- INPUT HELPERS ----------

bool readInt(const string& prompt, int& value) {
    cout << prompt;

    if (cin >> value)
        return true;

    cout << "Invalid input! Please enter a number.\n";
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return false;
}

bool readName(const string& prompt, string& name) {
    cout << prompt;
    getline(cin >> ws, name);

    if (name.empty()) {
        cout << "Name cannot be empty!\n";
        return false;
    }

    return true;
}

string toLowerCase(string value) {
    for (char& ch : value) {
        ch = static_cast<char>(
            tolower(static_cast<unsigned char>(ch))
        );
    }

    return value;
}

// ---------- INITIALIZATION ----------

void initSeats() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            seats[r][c] = 0;
        }
    }
}

// ---------- VISUAL SEAT LAYOUT ----------

void displaySeats(
    const int selectedRows[] = nullptr,
    const int selectedCols[] = nullptr,
    int selectedCount = 0
) {
    cout << "\n========== BUS SEAT LAYOUT ==========\n";
    cout << "[ ] = Available    [XX] = Booked\n";
    cout << "[01], [02]... = Selected for current group\n\n";

    cout << left
         << setw(8) << "Row"
         << setw(8) << "Col 1"
         << setw(8) << "Col 2"
         << "   "
         << setw(8) << "Col 3"
         << setw(8) << "Col 4" << '\n';

    cout << string(46, '-') << '\n';

    for (int r = 0; r < ROWS; r++) {
        cout << left << setw(8)
             << ("Row " + to_string(r + 1));

        for (int c = 0; c < COLS; c++) {
            string label;

            if (seats[r][c] != 0) {
                label = "[XX]";
            } else {
                int groupSeatNumber = 0;

                for (int k = 0; k < selectedCount; k++) {
                    if (selectedRows[k] == r &&
                        selectedCols[k] == c) {
                        groupSeatNumber = k + 1;
                        break;
                    }
                }

                if (groupSeatNumber > 0) {
                    label = "[";

                    if (groupSeatNumber < 10)
                        label += "0";

                    label += to_string(groupSeatNumber) + "]";
                } else {
                    label = "[ ]";
                }
            }

            cout << setw(8) << label;

            // Gap between the two pairs of seats
            if (c == 1)
                cout << "   ";
        }

        cout << '\n';
    }

    cout << "\nAvailable seats: "
         << MAX_PASSENGERS - passengerCount << '\n';
}

// ---------- CIRCULAR QUEUE ----------

bool isQueueEmpty() {
    return queueCount == 0;
}

bool isQueueFull() {
    return queueCount == QUEUE_SIZE;
}

void enqueue(Passenger p) {
    if (isQueueFull()) {
        cout << "Counter queue is full!\n";
        return;
    }

    rear = (rear + 1) % QUEUE_SIZE;
    counterQueue[rear] = p;
    queueCount++;

    cout << "Joined counter queue successfully.\n";
    cout << "People ahead: " << queueCount - 1 << endl;
}

Passenger dequeue() {
    Passenger p = counterQueue[front];
    front = (front + 1) % QUEUE_SIZE;
    queueCount--;
    return p;
}

void joinCounterQueue() {
    if (isQueueFull()) {
        cout << "Counter queue is full!\n";
        return;
    }

    Passenger p{};
    p.ticketId = 0;
    p.row = p.col = -1;
    p.groupId = 0;

    if (!readName("Enter customer name: ", p.name))
        return;

    if (!readInt("Enter customer age: ", p.age))
        return;

    if (p.age <= 0) {
        cout << "Age must be positive!\n";
        return;
    }

    enqueue(p);
}

void displayCounterQueue() {
    cout << "\n===== COUNTER QUEUE =====\n";

    if (isQueueEmpty()) {
        cout << "Counter queue is empty.\n";
        return;
    }

    for (int i = 0; i < queueCount; i++) {
        int index = (front + i) % QUEUE_SIZE;

        cout << i + 1 << ". "
             << counterQueue[index].name
             << " | Age: " << counterQueue[index].age
             << '\n';
    }
}

// ---------- LINKED-LIST WAITING LIST ----------

bool isWaitingEmpty() {
    return waitFront == nullptr;
}

void addToWaiting(Passenger p) {
    Node* newNode = new Node{p, nullptr};

    if (waitRear == nullptr) {
        waitFront = waitRear = newNode;
    } else {
        waitRear->next = newNode;
        waitRear = newNode;
    }

    cout << p.name << " added to the waiting list.\n";
    cout << "Waiting-list ticket ID: "
         << p.ticketId << '\n';
}

Passenger removeFirstWaiting() {
    Node* temp = waitFront;
    Passenger p = temp->data;

    waitFront = waitFront->next;

    if (waitFront == nullptr)
        waitRear = nullptr;

    delete temp;
    return p;
}

void displayWaiting() {
    cout << "\n===== WAITING LIST =====\n";

    if (isWaitingEmpty()) {
        cout << "No passengers on the waiting list.\n";
        return;
    }

    Node* current = waitFront;
    int position = 1;

    while (current != nullptr) {
        cout << position++ << ". "
             << current->data.name
             << " | Ticket ID: " << current->data.ticketId
             << " | Age: " << current->data.age << '\n';

        current = current->next;
    }
}

// ---------- UNDO STACK ----------

bool isStackEmpty() {
    return top == -1;
}

bool isStackFull() {
    return top == STACK_SIZE - 1;
}

bool pushHistory(int groupId) {
    if (isStackFull()) {
        cout << "Warning: Undo history is full. "
             << "This group cannot be undone.\n";
        return false;
    }

    history[++top] = groupId;
    return true;
}

int popHistory() {
    return history[top--];
}

// ---------- PASSENGER DETAILS ----------

void displayPassengerDetails(const Passenger& p) {
    cout << "\nTicket ID: " << p.ticketId
         << "\nName: " << p.name
         << "\nAge: " << p.age;

    if (p.row >= 0 && p.col >= 0) {
        cout << "\nSeat: Row " << p.row + 1
             << ", Column " << p.col + 1;
    }

    cout << '\n';
}

void displayPassengers() {
    cout << "\n===== BOOKED PASSENGERS =====\n";

    if (passengerCount == 0) {
        cout << "No passengers booked yet.\n";
        return;
    }

    for (int i = 0; i < passengerCount; i++) {
        displayPassengerDetails(booked[i]);

        cout << "Group booking ID: "
             << booked[i].groupId << '\n';
    }

    cout << "\nTotal booked passengers: "
         << passengerCount << "/" << MAX_PASSENGERS << '\n';
}

// ---------- MULTIPLE-SEAT GROUP BOOKING ----------

void serveNextCustomer() {
    if (isQueueEmpty()) {
        cout << "Counter queue is empty!\n";
        return;
    }

    Passenger customer = counterQueue[front];

    cout << "\nServing customer: "
         << customer.name << '\n';

    int count;

    if (!readInt("How many seats do you want to book? ", count))
        return;

    if (count <= 0 || count > MAX_PASSENGERS) {
        cout << "Invalid number of seats!\n";
        return;
    }

    if (count > MAX_PASSENGERS - passengerCount) {
        cout << "Not enough available seats for this group.\n";
        cout << "Available seats: "
             << MAX_PASSENGERS - passengerCount << '\n';
        return;
    }

    // Check undo capacity before accepting the booking.
    // This prevents a successful group booking without
    // a corresponding undo entry.
    if (isStackFull()) {
        cout << "Undo history is full. Cannot create "
             << "another group booking.\n";
        return;
    }

    // Temporary storage for all group members
    Passenger group[MAX_PASSENGERS];
    int requestedRows[MAX_PASSENGERS];
    int requestedCols[MAX_PASSENGERS];

    for (int i = 0; i < count; i++) {
        Passenger p{};

        cout << "\nPassenger " << i + 1
             << " of " << count << '\n';

        if (i == 0) {
            p.name = customer.name;
            p.age = customer.age;

            cout << "Customer: " << p.name
                 << " (Age: " << p.age << ")\n";
        } else {
            if (!readName("Enter passenger name: ", p.name))
                return;

            if (!readInt("Enter passenger age: ", p.age))
                return;

            if (p.age <= 0) {
                cout << "Age must be positive! "
                     << "No seats were booked.\n";
                return;
            }
        }

        // Display earlier group selections before the next choice.
        displaySeats(requestedRows, requestedCols, i);

        int row, col;

        if (!readInt("Choose seat row (1-10): ", row))
            return;

        if (!readInt("Choose seat column (1-4): ", col))
            return;

        if (row < 1 || row > ROWS ||
            col < 1 || col > COLS) {
            cout << "Invalid seat! No seats were booked.\n";
            return;
        }

        row--;
        col--;

        if (seats[row][col] != 0) {
            cout << "That seat is already booked! "
                 << "No seats were booked.\n";
            return;
        }

        // Prevent duplicate selections within the group.
        for (int j = 0; j < i; j++) {
            if (requestedRows[j] == row &&
                requestedCols[j] == col) {
                cout << "This seat was already selected "
                     << "for this group. No seats were booked.\n";
                return;
            }
        }

        p.ticketId = 0;
        p.row = row;
        p.col = col;
        p.groupId = 0;

        group[i] = p;
        requestedRows[i] = row;
        requestedCols[i] = col;
    }

    // All seats have been validated. Commit the entire group.
    int groupId = nextGroupId++;

    for (int i = 0; i < count; i++) {
        group[i].ticketId = nextTicketId++;
        group[i].groupId = groupId;

        seats[group[i].row][group[i].col] =
            group[i].ticketId;

        booked[passengerCount++] = group[i];
    }

    // One queue entry creates one group booking.
    dequeue();
    pushHistory(groupId);

    cout << "\n===== GROUP BOOKING SUCCESSFUL =====\n";
    cout << "Group booking ID: " << groupId << '\n';
    cout << "Seats booked: " << count << '\n';

    for (int i = 0; i < count; i++)
        displayPassengerDetails(group[i]);

    cout << "\nAll passengers have been booked successfully!\n";

    displaySeats();
}

// ---------- WAITING-LIST PROMOTION ----------

void promoteWaitingPassenger(int row, int col) {
    if (isWaitingEmpty())
        return;

    Passenger p = removeFirstWaiting();

    p.row = row;
    p.col = col;
    p.groupId = 0;

    seats[row][col] = p.ticketId;
    booked[passengerCount++] = p;

    cout << "\nWaiting-list passenger promoted!\n";
    displayPassengerDetails(p);
}

// ---------- INDIVIDUAL CANCELLATION ----------

void cancelBookedIndex(int index) {
    int id = booked[index].ticketId;
    int row = booked[index].row;
    int col = booked[index].col;

    seats[row][col] = 0;

    for (int i = index; i < passengerCount - 1; i++)
        booked[i] = booked[i + 1];

    passengerCount--;

    cout << "Ticket " << id << " cancelled.\n";
    cout << "Seat Row " << row + 1
         << ", Column " << col + 1 << " released.\n";

    promoteWaitingPassenger(row, col);
}

void cancelTicket() {
    int id;

    if (!readInt("Enter ticket ID to cancel: ", id))
        return;

    int index = -1;

    for (int i = 0; i < passengerCount; i++) {
        if (booked[i].ticketId == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        cout << "Booked ticket not found!\n";
        return;
    }

    cancelBookedIndex(index);
}

// ---------- GROUP CANCELLATION FOR UNDO ----------

void cancelGroup(int groupId) {
    int freedRows[MAX_PASSENGERS];
    int freedCols[MAX_PASSENGERS];
    int freedCount = 0;
    int writeIndex = 0;

    for (int i = 0; i < passengerCount; i++) {
        if (booked[i].groupId == groupId) {
            int row = booked[i].row;
            int col = booked[i].col;

            if (row >= 0 && col >= 0) {
                seats[row][col] = 0;
                freedRows[freedCount] = row;
                freedCols[freedCount] = col;
                freedCount++;
            }

            cout << "Cancelled ticket "
                 << booked[i].ticketId
                 << " (" << booked[i].name << ").\n";
        } else {
            booked[writeIndex++] = booked[i];
        }
    }

    passengerCount = writeIndex;

    if (freedCount == 0) {
        cout << "No active tickets remain in this group.\n";
        return;
    }

    cout << "Group booking " << groupId
         << " cancelled. Seats released: "
         << freedCount << '\n';

    // Promote waiting passengers into released seats.
    for (int i = 0; i < freedCount; i++)
        promoteWaitingPassenger(freedRows[i], freedCols[i]);

    displaySeats();
}

// ---------- UNDO ENTIRE GROUP BOOKING ----------

void undoLastBooking() {
    while (!isStackEmpty()) {
        int groupId = popHistory();
        bool found = false;

        for (int i = 0; i < passengerCount; i++) {
            if (booked[i].groupId == groupId) {
                found = true;
                break;
            }
        }

        // Skip groups whose tickets were already cancelled.
        if (!found)
            continue;

        cout << "\nUndoing group booking "
             << groupId << "...\n";

        cancelGroup(groupId);
        cout << "Undo completed.\n";
        return;
    }

    cout << "Nothing to undo!\n";
}

// ---------- LINEAR SEARCH ----------

void searchByTicketId() {
    int id;

    if (!readInt("Enter ticket ID: ", id))
        return;

    for (int i = 0; i < passengerCount; i++) {
        if (booked[i].ticketId == id) {
            cout << "\nPassenger found!\n";
            displayPassengerDetails(booked[i]);
            cout << "Group booking ID: "
                 << booked[i].groupId << '\n';
            return;
        }
    }

    cout << "Booked passenger not found!\n";
}

void searchByName() {
    string name;

    if (!readName("Enter passenger name: ", name))
        return;

    string target = toLowerCase(name);
    bool found = false;

    for (int i = 0; i < passengerCount; i++) {
        if (toLowerCase(booked[i].name) == target) {
            cout << "\nPassenger found!\n";
            displayPassengerDetails(booked[i]);
            cout << "Group booking ID: "
                 << booked[i].groupId << '\n';
            found = true;
        }
    }

    if (!found)
        cout << "Booked passenger not found!\n";
}

void searchPassenger() {
    int choice;

    cout << "\n===== SEARCH PASSENGER =====\n";
    cout << "1. Search by ticket ID\n";
    cout << "2. Search by name\n";

    if (!readInt("Enter choice: ", choice))
        return;

    switch (choice) {
        case 1:
            searchByTicketId();
            break;
        case 2:
            searchByName();
            break;
        default:
            cout << "Invalid search choice!\n";
    }
}

// ---------- INSERTION SORT ----------

void sortPassengersByName() {
    if (passengerCount == 0) {
        cout << "No passengers to sort.\n";
        return;
    }

    // Sort a copy; actual booking order is unchanged.
    Passenger sorted[MAX_PASSENGERS];

    for (int i = 0; i < passengerCount; i++)
        sorted[i] = booked[i];

    for (int i = 1; i < passengerCount; i++) {
        Passenger key = sorted[i];
        string keyName = toLowerCase(key.name);
        int j = i - 1;

        while (j >= 0 &&
               toLowerCase(sorted[j].name) > keyName) {
            sorted[j + 1] = sorted[j];
            j--;
        }

        sorted[j + 1] = key;
    }

    cout << "\n===== PASSENGERS SORTED BY NAME =====\n";

    for (int i = 0; i < passengerCount; i++) {
        cout << "\n" << i + 1 << ". ";
        displayPassengerDetails(sorted[i]);

        cout << "Group booking ID: "
             << sorted[i].groupId << '\n';
    }
}

// ---------- MEMORY CLEANUP ----------

void cleanupWaitingList() {
    while (!isWaitingEmpty())
        removeFirstWaiting();
}

// ---------- MAIN MENU ----------

int main() {
    int choice;
    initSeats();

    cout << "========================================\n";
    cout << "       BUS TICKET BOOKING SYSTEM\n";
    cout << "========================================\n";

    do {
        cout << "\n============== MAIN MENU ==============\n";
        cout << "1. Join counter queue\n";
        cout << "2. Serve customer and book seat(s)\n";
        cout << "3. Show visual seat layout\n";
        cout << "4. Show booked passengers\n";
        cout << "5. Cancel individual ticket\n";
        cout << "6. Show counter queue\n";
        cout << "7. Show waiting list\n";
        cout << "8. Undo last group booking\n";
        cout << "9. Search passenger\n";
        cout << "10. Sort passengers by name\n";
        cout << "11. Exit\n";

        if (!readInt("Enter your choice: ", choice))
            continue;

        switch (choice) {
            case 1:
                joinCounterQueue();
                break;

            case 2:
                serveNextCustomer();
                break;

            case 3:
                displaySeats();
                break;

            case 4:
                displayPassengers();
                break;

            case 5:
                cancelTicket();
                break;

            case 6:
                displayCounterQueue();
                break;

            case 7:
                displayWaiting();
                break;

            case 8:
                undoLastBooking();
                break;

            case 9:
                searchPassenger();
                break;

            case 10:
                sortPassengersByName();
                break;

            case 11:
                cout << "Thank you for using our system!\n";
                break;

            default:
                cout << "Invalid menu choice!\n";
        }

    } while (choice != 11);

    cleanupWaitingList();
    return 0;
}
