#include <iostream>
#include <string>
#include <vector>
#include <memory>
using namespace std;

// ==================== Course Class (BST Node) ====================
class Course {
public:
    string courseCode;
    string courseName;
    int capacity;
    int enrolled;
    Course* left;
    Course* right;

    Course(string code, string name, int cap)
        : courseCode(code), courseName(name), capacity(cap),
          enrolled(0), left(nullptr), right(nullptr) {}

    bool isFull() const {
        return enrolled >= capacity;
    }
};

// ==================== Course BST Manager ====================
class CourseBST {
private:
    Course* root;

    Course* insert(Course* node, string code, string name, int capacity) {
        if (!node) return new Course(code, name, capacity);
        if (code < node->courseCode)
            node->left = insert(node->left, code, name, capacity);
        else if (code > node->courseCode)
            node->right = insert(node->right, code, name, capacity);
        return node;
    }

    Course* search(Course* node, string code) const {
        if (!node || node->courseCode == code) return node;
        if (code < node->courseCode)
            return search(node->left, code);
        return search(node->right, code);
    }

    void displayAllByDept(Course* node, const string& deptPrefix) const {
        if (!node) return;
        displayAllByDept(node->left, deptPrefix);
        if (node->courseCode.rfind(deptPrefix, 0) == 0) { // starts with deptPrefix
            cout << "  " << node->courseCode << " - " << node->courseName
                 << " (" << node->enrolled << "/" << node->capacity << ")" << endl;
        }
        displayAllByDept(node->right, deptPrefix);
    }

public:
    CourseBST() : root(nullptr) {}

    void addCourse(string code, string name, int capacity) {
        root = insert(root, code, name, capacity);
    }

    Course* findCourse(string code) const {
        return search(root, code);
    }

    bool courseExists(string code) const {
        return findCourse(code) != nullptr;
    }

    void displayCoursesByDepartment(const string& deptPrefix) const {
        cout << "\n=== Courses for Department " << deptPrefix << " ===" << endl;
        displayAllByDept(root, deptPrefix);
    }

    bool enrollStudent(string courseCode) {
        Course* course = findCourse(courseCode);
        if (!course) {
            cout << "Course " << courseCode << " not found!" << endl;
            return false;
        }
        if (course->isFull()) {
            cout << "Course " << courseCode << " is full!" << endl;
            return false;
        }
        course->enrolled++;
        return true;
    }

    bool dropStudent(string courseCode) {
        Course* course = findCourse(courseCode);
        if (!course) {
            cout << "Course " << courseCode << " not found!" << endl;
            return false;
        }
        if (course->enrolled > 0) {
            course->enrolled--;
            return true;
        }
        return false;
    }

    void showCourseDetails(string courseCode) const {
        Course* course = findCourse(courseCode);
        if (course) {
            cout << "\nCourse Code: " << course->courseCode << endl;
            cout << "Course Name: " << course->courseName << endl;
            cout << "Capacity: " << course->capacity << endl;
            cout << "Enrolled Students: " << course->enrolled << endl;
        } else {
            cout << "Course not found!" << endl;
        }
    }
};

// ==================== Registered Course Node ====================
class RegisteredCourseNode {
public:
    string courseCode;
    shared_ptr<RegisteredCourseNode> next;

    RegisteredCourseNode(string code) : courseCode(code), next(nullptr) {}
};

// ==================== Student Class ====================
class Student {
private:
    string studentID;
    string name;
    string departmentCode;
    shared_ptr<RegisteredCourseNode> registeredCoursesHead;

    bool isCourseRegistered(string courseCode) const {
        auto current = registeredCoursesHead;
        while (current) {
            if (current->courseCode == courseCode)
                return true;
            current = current->next;
        }
        return false;
    }

public:
    Student(string id, string studentName, string dept)
        : studentID(id), name(studentName), departmentCode(dept), registeredCoursesHead(nullptr) {}

    string getID() const { return studentID; }
    string getName() const { return name; }
    string getDepartment() const { return departmentCode; }

    bool registerCourse(CourseBST& courseManager) {
        courseManager.displayCoursesByDepartment(departmentCode);
        string courseCode;
        cout << "Enter Course Code to register: "; cin >> courseCode;

        if (isCourseRegistered(courseCode)) {
            cout << "Student " << studentID << " is already registered in " << courseCode << endl;
            return false;
        }

        if (!courseManager.courseExists(courseCode)) {
            cout << "Course " << courseCode << " does not exist!" << endl;
            return false;
        }

        if (courseManager.enrollStudent(courseCode)) {
            auto newNode = make_shared<RegisteredCourseNode>(courseCode);
            newNode->next = registeredCoursesHead;
            registeredCoursesHead = newNode;
            cout << "Successfully registered " << studentID << " in " << courseCode << endl;
            return true;
        }
        return false;
    }

    bool dropCourse(string courseCode, CourseBST& courseManager) {
        if (!isCourseRegistered(courseCode)) {
            cout << "Student " << studentID << " is not registered in " << courseCode << endl;
            return false;
        }

        shared_ptr<RegisteredCourseNode> current = registeredCoursesHead;
        shared_ptr<RegisteredCourseNode> prev = nullptr;

        while (current && current->courseCode != courseCode) {
            prev = current;
            current = current->next;
        }

        if (current) {
            if (prev)
                prev->next = current->next;
            else
                registeredCoursesHead = current->next;

            courseManager.dropStudent(courseCode);
            cout << "Successfully dropped " << courseCode << " for student " << studentID << endl;
            return true;
        }
        return false;
    }

    void viewRegisteredCourses(const CourseBST& courseManager) const {
        cout << "\n=== Registered Courses for " << studentID << " (" << name << ") ===" << endl;
        if (!registeredCoursesHead) {
            cout << "No courses registered." << endl;
            return;
        }

        auto current = registeredCoursesHead;
        int count = 1;
        while (current) {
            Course* c = courseManager.findCourse(current->courseCode);
            if (c)
                cout << count << ". " << c->courseCode << " : " << c->courseName << endl;
            else
                cout << count << ". " << current->courseCode << " : [Course not found]" << endl;
            current = current->next;
            count++;
        }
    }
};

// ==================== Department Hash Table ====================
class DepartmentHashTable {
private:
    struct Department {
        string code;
        string name;
        Department* next;
        Department(string c, string n) : code(c), name(n), next(nullptr) {}
    };

    static const int TABLE_SIZE = 10;
    vector<Department*> table;

    int hashFunction(string key) const {
        int hash = 0;
        for (char c : key) hash = (hash * 31 + c) % TABLE_SIZE;
        return hash;
    }

public:
    DepartmentHashTable() : table(TABLE_SIZE, nullptr) {
        insertDepartment("CS", "Computer Science");
        insertDepartment("EE", "Electrical Engineering");
        insertDepartment("ME", "Mechanical Engineering");
        insertDepartment("CE", "Civil Engineering");
        insertDepartment("MATH", "Mathematics");
        insertDepartment("PHYS", "Physics");
        insertDepartment("CHEM", "Chemistry");
        insertDepartment("BIO", "Biology");
        insertDepartment("ECON", "Economics");
        insertDepartment("ENG", "English");
    }

    void insertDepartment(string code, string name) {
        int index = hashFunction(code);
        Department* newDept = new Department(code, name);
        newDept->next = table[index];
        table[index] = newDept;
    }

    void displayAllDepartments() const {
        cout << "\n=== Department Directory ===" << endl;
        for (int i = 0; i < TABLE_SIZE; i++) {
            Department* current = table[i];
            while (current) {
                cout << "  " << current->code << " -> " << current->name << endl;
                current = current->next;
            }
        }
    }

    string searchDepartment(string code) const {
        int index = hashFunction(code);
        Department* current = table[index];
        while (current) {
            if (current->code == code)
                return current->name;
            current = current->next;
        }
        return "Department not found";
    }
};

// ==================== University System ====================
class UniversitySystem {
private:
    vector<Student> students;
    CourseBST courseManager;
    DepartmentHashTable deptTable;

    void initializeSampleData() {
        courseManager.addCourse("CS101", "Introduction to Programming", 30);
        courseManager.addCourse("CS201", "Data Structures", 25);
        courseManager.addCourse("CS301", "Algorithms", 20);
        courseManager.addCourse("MATH101", "Calculus I", 40);
        courseManager.addCourse("PHYS101", "Physics I", 35);
        courseManager.addCourse("ENG101", "Composition I", 30);
        courseManager.addCourse("EE101", "Circuit Analysis", 25);

        students.push_back(Student("S1001", "Alice Johnson", "CS"));
        students.push_back(Student("S1002", "Bob Smith", "EE"));
        students.push_back(Student("S1003", "Charlie Brown", "MATH"));
    }

    Student* findStudent(string studentID) {
        for (auto& s : students)
            if (s.getID() == studentID) return &s;
        return nullptr;
    }

    void addNewStudent() {
        string id, name, dept;
        cout << "\n=== Add New Student ===" << endl;
        cout << "Enter Student ID: "; cin >> id; cin.ignore();
        if (findStudent(id)) { cout << "Student already exists!" << endl; return; }
        cout << "Enter Student Name: "; getline(cin, name);

        cout << "Available Departments:" << endl;
        deptTable.displayAllDepartments();
        cout << "Enter Department Code for the student: "; cin >> dept;

        students.push_back(Student(id, name, dept));
        cout << "Student added successfully!" << endl;
    }

    void registerStudentInCourse(Student* s) {
        s->registerCourse(courseManager);
    }

    void dropStudentFromCourse(Student* s) {
        string code;
        cout << "Enter Course Code to drop: "; cin >> code;
        s->dropCourse(code, courseManager);
    }

    void viewStudentCourses(Student* s) {
        s->viewRegisteredCourses(courseManager);
    }

    void searchDepartmentInfo() {
        string code;
        cout << "Enter Department Code: "; cin >> code;
        string deptName = deptTable.searchDepartment(code);
        cout << "Department: " << deptName << endl;

        char choice;
        cout << "Do you want to search for a course in this department? (Y/N): "; cin >> choice;
        if (choice == 'Y' || choice == 'y') {
            courseManager.displayCoursesByDepartment(code);
            string courseCode;
            cout << "Enter Course Code to view details: "; cin >> courseCode;
            courseManager.showCourseDetails(courseCode);
        }
    }

    void displayAllStudents() {
        cout << "\n=== All Students ===" << endl;
        for (auto& s : students)
            cout << "  " << s.getID() << " - " << s.getName() << " (" << s.getDepartment() << ")" << endl;
    }

public:
    UniversitySystem() { initializeSampleData(); }

    void studentMenu() {
        string id;
        cout << "\n=== Student Login ===" << endl;
        cout << "Enter Student ID: "; cin >> id;
        Student* s = findStudent(id);
        if (!s) { cout << "Student not found!" << endl; return; }

        int choice;
        do {
            cout << "\n===== STUDENT MENU =====" << endl;
            cout << "1. View All Courses" << endl;
            cout << "2. Register in Course" << endl;
            cout << "3. Drop a Course" << endl;
            cout << "4. View Registered Courses" << endl;
            cout << "0. Back" << endl;
            cout << "Enter choice: "; cin >> choice;

            switch (choice) {
                case 1:
                    courseManager.displayCoursesByDepartment(s->getDepartment());
                    break;
                case 2: registerStudentInCourse(s); break;
                case 3: dropStudentFromCourse(s); break;
                case 4: viewStudentCourses(s); break;
                case 0: break;
                default: cout << "Invalid!" << endl;
            }
        } while (choice != 0);
    }

    void adminMenu() {
        int choice;
        do {
            cout << "\n===== ADMIN MENU =====" << endl;
            cout << "1. Display All Courses (by Department)" << endl;
            cout << "2. Display All Students" << endl;
            cout << "3. Display All Departments" << endl;
            cout << "4. Add New Student" << endl;
            cout << "5. Register Student in Course" << endl;
            cout << "6. Drop Student from Course" << endl;
            cout << "7. View Student's Courses" << endl;
            cout << "8. Search Department" << endl;
            cout << "9. Add New Course" << endl;
            cout << "0. Back" << endl;
            cout << "Enter choice: "; cin >> choice;

            switch (choice) {
                case 1: {
                    cout << "\nAvailable Departments:" << endl;
                    deptTable.displayAllDepartments();
                    string dept; cout << "Enter Department Code to view courses: "; cin >> dept;
                    courseManager.displayCoursesByDepartment(dept);
                    break;
                }
                case 2: displayAllStudents(); break;
                case 3: deptTable.displayAllDepartments(); break;
                case 4: addNewStudent(); break;
                case 5: {
                    string studentID;
                    cout << "Enter Student ID: "; cin >> studentID;
                    Student* s = findStudent(studentID);
                    if (!s) { cout << "Student not found!" << endl; break; }
                    registerStudentInCourse(s);
                    break;
                }
                case 6: {
                    string studentID;
                    cout << "Enter Student ID: "; cin >> studentID;
                    Student* s = findStudent(studentID);
                    if (!s) { cout << "Student not found!" << endl; break; }
                    dropStudentFromCourse(s);
                    break;
                }
                case 7: {
                    string studentID;
                    cout << "Enter Student ID: "; cin >> studentID;
                    Student* s = findStudent(studentID);
                    if (!s) { cout << "Student not found!" << endl; break; }
                    viewStudentCourses(s);
                    break;
                }
                case 8: searchDepartmentInfo(); break;
                case 9: {
                    string code, name, dept; int cap;
                    cout << "Enter Course Department Code: "; cin >> dept;
                    cout << "Enter Course Code (without department prefix): "; cin >> code; cin.ignore();
                    code = dept + code; // prepend department code automatically
                    cout << "Enter Course Name: "; getline(cin, name);
                    cout << "Enter Capacity: "; cin >> cap;
                    courseManager.addCourse(code, name, cap);
                    cout << "Course added successfully as " << code << endl;
                    break;
                }
                case 0: break;
                default: cout << "Invalid!" << endl;
            }
        } while (choice != 0);
    }

    void run() {
        int role;
        while (true) {
            cout << "\n=============================" << endl;
            cout << "   UNIVERSITY LOGIN SYSTEM   " << endl;
            cout << "=============================" << endl;
            cout << "1. Admin" << endl;
            cout << "2. Student" << endl;
            cout << "0. Exit" << endl;
            cout << "Enter choice: "; cin >> role;

            if (role == 1) adminMenu();
            else if (role == 2) studentMenu();
            else if (role == 0) { cout << "Exiting system..." << endl; break; }
            else cout << "Invalid selection!" << endl;
        }
    }
};

// ==================== Main Function ====================
int main() {
    UniversitySystem system;
    system.run();
    return 0;
}