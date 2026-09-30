// career_dendrogram.cpp
// Career Dendrogram System - the whole program in ONE file.
//
// How to run in Dev-C++:  open this file and press F11 (Compile & Run).
// No settings are needed. It works with Dev-C++ 5.11 and newer.
//
// The file is divided into sections (Utils, Sha256, Models, FileStore,
// Auth, Quiz, Engine, CareerApp, main). Each section starts with a banner.

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

// ======================================================================
// Utils (declarations)
// ======================================================================

// Utils.h
// Small helper functions used everywhere: colours, boxes, bars, menus,
// reading input, and working with text and time.

// ---------- Colours ----------
// ANSI colour codes. They are only used when useColor is true.
extern bool useColor;
const std::string CYAN = "36";
const std::string GREEN = "32";
const std::string YELLOW = "33";
const std::string RED = "31";
const std::string GREY = "90";
const std::string BOLD = "1";
const std::string PINK = "1;35";

const int SCREEN_WIDTH = 78;

void setupConsole();
void clearScreen();
std::string color(const std::string& text, const std::string& code);

// ---------- Text helpers ----------
std::string padRight(const std::string& text, int width);
std::string padLeft(const std::string& text, int width);
std::string cutText(const std::string& text, int width);
std::vector<std::string> wrapText(const std::string& text, int width);
std::vector<std::string> splitText(const std::string& text, char separator);
std::string trimText(const std::string& text);
std::string toLowerText(std::string text);
std::string percent(double value);          // 0.87 -> "87%"
std::string signedPoints(double change);    // 0.12 -> "+12"
std::string formatNumber(double value, int decimals);
std::string toText(long long number);      // 42 -> "42"
int toInt(const std::string& text);         // "42" -> 42
double toDouble(const std::string& text);   // "0.5" -> 0.5
long long toLongLong(const std::string& text);

// ---------- Drawing ----------
std::string makeBar(double fraction, int width);                        // "#####....."
std::string colorBar(double fraction, int width, const std::string& code);
void printBanner();
void printTitle(const std::string& title, const std::string& path);
void printBox(const std::string& title, const std::vector<std::string>& lines);
void printHeading(const std::string& text);

void showInfo(const std::string& message);
void showSuccess(const std::string& message);
void showWarning(const std::string& message);
void showError(const std::string& message);

// ---------- Input ----------
struct MenuOption {
    int number;
    std::string text;
    std::string hint;
};

std::string askText(const std::string& label);
std::string askPassword(const std::string& label);
int askNumber(const std::string& label, int minValue, int maxValue);
bool askYesNo(const std::string& question);
void waitForEnter();
MenuOption makeOption(int number, const std::string& text, const std::string& hint);
int showMenu(const std::vector<MenuOption>& options);

// ---------- Files and folders ----------
void makeFolder(const std::string& path);         // creates a folder (and its parents)
void makeFolderFor(const std::string& fileName);  // creates the folder a file lives in
bool fileExists(const std::string& fileName);

// ---------- Time ----------
std::string currentTime();      // "2026-10-01 14:05:09"
long long currentSeconds();     // seconds since 1970

// ---------- A small function template ----------
// Keeps a value between low and high. Works for int, double, etc.
template <typename T>
T limitValue(T value, T low, T high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

// ======================================================================
// Sha256 (declarations)
// ======================================================================

// Sha256.h
// Turns any text into a 64-character hexadecimal SHA-256 hash.
// Used so that passwords are never saved as plain text.

std::string sha256(const std::string& text);

// ======================================================================
// Models (declarations)
// ======================================================================

// Models.h
// The main data classes of the program.
//
// A student and a career are both described by the same 12 numbers
// ("traits"), each between 0 and 1:
//   0-5  : interests  (Realistic, Investigative, Artistic, Social, Enterprising, Conventional)
//   6-11 : aptitudes  (Quantitative, Computing, Communication, Design, Life Sciences, Business)
// Because both use the same 12 numbers, we can measure how close they are.

const int TRAIT_COUNT = 12;
const int INTEREST_COUNT = 6;
extern const std::string TRAIT_NAMES[TRAIT_COUNT];

// ---------- Maths on trait lists ----------
double correlation(const std::vector<double>& a, const std::vector<double>& b);  // -1 .. 1
double traitDistance(const std::vector<double>& a, const std::vector<double>& b);     // Euclidean
std::string hollandCode(const std::vector<double>& traits);                      // e.g. "ICR"
std::string traitsToText(const std::vector<double>& traits);                     // "0.62,0.80,..."
bool textToTraits(const std::string& text, std::vector<double>& traits);

// ---------- Career ----------
class Career {
public:
    std::string id;
    std::string name;
    std::string domain;
    std::vector<double> traits;
    int salaryMin;     // lakh per annum (INR), indicative
    int salaryMax;
    int growth;        // 1 = declining ... 5 = rapid growth
    std::string exams;
    std::string summary;
    std::vector<std::string> milestones;

    Career();
    bool fromLine(const std::string& line);
    std::string salaryText() const;
    std::string growthText() const;
};

// Holds all careers loaded from careers.csv.
class CareerList {
private:
    std::vector<Career> careers;

public:
    int load(const std::string& fileName);
    std::vector<Career>& getAll();
    Career* findById(const std::string& id);
    std::vector<Career*> search(const std::string& word);
    std::vector<std::string> getDomains();
};

// ---------- Records saved with FileStore<T> ----------
class User {
public:
    std::string username;
    std::string salt;
    std::string passwordHash;
    std::string fullName;
    std::string createdAt;
    int failedAttempts;
    long long lockedUntil;   // time (in seconds) until which login is blocked

    User();
    std::string getKey() const;
    std::string toLine() const;
    bool fromLine(const std::string& line);
};

class AssessmentResult {
public:
    std::string date;
    std::vector<double> traits;
    double confidenceGap;   // self-rating minus quiz score (positive = over-confident)

    AssessmentResult();
    std::string getKey() const;
    std::string toLine() const;
    bool fromLine(const std::string& line);
};

class RoadmapProgress {
public:
    std::string careerId;
    std::string done;   // one character per milestone: '1' = done, '0' = not done

    std::string getKey() const;
    std::string toLine() const;
    bool fromLine(const std::string& line);
    int countDone() const;
};

// ======================================================================
// FileStore (declarations)
// ======================================================================

// FileStore.h
// A class template that saves a list of records to a text file and loads
// them back. One template works for users, assessments and roadmaps.
//
// The record type T must have these three member functions:
//     std::string getKey() const;               // a unique id, e.g. username
//     std::string toLine() const;               // record -> one line of text
//     bool fromLine(const std::string& line);   // one line of text -> record

template <typename T>
class FileStore {
private:
    std::string fileName;
    std::string description;   // written as a comment on the first line
    std::vector<T> items;

public:
    FileStore(const std::string& file, const std::string& about) {
        fileName = file;
        description = about;
    }

    // Reads every line of the file. Bad lines are skipped.
    void load() {
        items.clear();
        std::ifstream input(fileName.c_str());
        std::string line;
        while (std::getline(input, line)) {
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            if (line.empty() || line[0] == '#') continue;
            T item;
            if (item.fromLine(line)) items.push_back(item);
        }
    }

    // Writes every record to the file. Returns false if the file cannot be written.
    bool save() const {
        makeFolderFor(fileName);
        std::ofstream output(fileName.c_str());
        if (!output) return false;
        output << "# " << description << "\n";
        for (int i = 0; i < (int)items.size(); i++) {
            output << items[i].toLine() << "\n";
        }
        return true;
    }

    // Returns a pointer to the record with this key, or NULL if not found.
    T* find(const std::string& key) {
        for (int i = 0; i < (int)items.size(); i++) {
            if (items[i].getKey() == key) return &items[i];
        }
        return NULL;
    }

    // Replaces the record with the same key, or adds it at the end.
    void addOrUpdate(const T& item) {
        T* existing = find(item.getKey());
        if (existing != NULL) {
            *existing = item;
        } else {
            items.push_back(item);
        }
    }

    bool remove(const std::string& key) {
        for (int i = 0; i < (int)items.size(); i++) {
            if (items[i].getKey() == key) {
                items.erase(items.begin() + i);
                return true;
            }
        }
        return false;
    }

    std::vector<T>& getAll() { return items; }
    int count() const { return (int)items.size(); }
};

// ======================================================================
// Auth (declarations)
// ======================================================================

// Auth.h
// Handles user accounts: register, log in, change password.
// Passwords are stored only as salted SHA-256 hashes.

const int MAX_LOGIN_ATTEMPTS = 3;    // wrong passwords allowed before locking
const int LOCK_SECONDS = 60;         // how long the account stays locked
const int HASH_ROUNDS = 5000;        // repeat hashing to slow down guessing

class AuthManager {
private:
    FileStore<User> users;

    std::string makeSalt();
    std::string hashPassword(const std::string& salt, const std::string& password);

public:
    AuthManager(const std::string& fileName);

    // These return "" on success, or an error message.
    std::string registerUser(const std::string& username, const std::string& fullName, const std::string& password);
    std::string changePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword);

    // Returns true and fills 'user' if the login is correct.
    // Otherwise returns false and puts the reason in 'message'.
    bool login(const std::string& username, const std::string& password, User& user, std::string& message);

    int secondsLocked(const std::string& username);
    int attemptsLeft(const std::string& username);
    int userCount();

    static bool isValidUsername(const std::string& username);
    static int passwordStrength(const std::string& password);   // 0 (very weak) to 4 (strong)
    static std::string strengthName(int strength);
};

// ======================================================================
// Quiz (declarations)
// ======================================================================

// Quiz.h
// The career assessment: 28 questions in three parts.
//   Part 1: 18 interest questions (3 for each RIASEC interest)
//   Part 2:  6 self-ratings of ability
//   Part 3:  4 short test questions that check two of those self-ratings

// Runs the assessment on screen.
// Returns false if the user quits; otherwise fills in the results.
bool runAssessment(std::vector<double>& traits, double& confidenceGap, int& quizScore, int& quizTotal);

// ======================================================================
// Engine (declarations)
// ======================================================================

// Engine.h
// The "brain" of the program:
//   1. scoreCareer / rankCareers - how well a student fits each career
//   2. Dendrogram                - groups similar careers into a tree

const double SHAPE_WEIGHT = 0.65;     // weight of "same pattern of strengths"
const double GAP_LIMIT = 0.15;        // a shortfall bigger than this counts as a gap

// The result of comparing one student with one career.
struct Match {
    Career* career;
    double fit;                 // final score, 0 to 1
    double shape;               // do the strengths follow the same pattern? (0 to 1)
    double level;               // are the scores at the same level? (0 to 1)
    std::vector<int> strengths; // traits that help this match the most
    std::vector<int> gapTraits; // traits where the student is below what is needed
    std::vector<double> gapSizes;
};

Match scoreCareer(const std::vector<double>& student, Career& career);
std::vector<Match> rankCareers(const std::vector<double>& student, CareerList& careers);

// Builds a tree by repeatedly joining the two closest groups
// (hierarchical clustering with average linkage, also called UPGMA).
class Dendrogram {
private:
    struct Node {
        int left;       // child node numbers (-1 for a leaf)
        int right;
        int parent;
        double height;  // distance at which the two children were joined
        int size;       // number of leaves under this node
        std::string label;
    };

    std::vector<Node> nodes;   // leaves first, then the joined groups
    int leafCount;
    int root;

    void collectLeaves(int node, std::vector<int>& leaves);
    void cutNode(int node, double limit, std::vector<std::vector<int> >& groups);

public:
    Dendrogram();
    void build(const std::vector<std::string>& labels, const std::vector<std::vector<double> >& points);

    // Returns the tree as lines of text. The path from 'highlightLeaf' to the top is coloured.
    std::vector<std::string> draw(int highlightLeaf, int width);

    // Leaves in the smallest group that contains 'leaf' and at least 'minSize' leaves.
    std::vector<int> familyOf(int leaf, int minSize);

    // Cuts the tree at a fraction of its height and returns the groups below the cut.
    std::vector<std::vector<int> > cutTree(double fraction);
};

// ======================================================================
// CareerApp (declarations)
// ======================================================================

// CareerApp.h
// Controls the screens: the start menu, the dashboard, and every feature.

class CareerApp {
private:
    std::string dataFolder;
    CareerList careers;
    AuthManager auth;

    // Information about the signed-in user.
    bool signedIn;
    User currentUser;
    FileStore<AssessmentResult>* assessments;
    FileStore<RoadmapProgress>* roadmaps;

    // Start menu screens
    bool signIn();
    void createAccount();
    void howItWorks();

    // Dashboard screens
    void dashboard();
    void takeAssessment();
    void showRecommendations();
    void showDendrogram();
    void exploreCareers();
    void showCareer(Career& career);
    void compareCareers();
    void whatIfSimulator();
    void roadmapTracker();
    void roadmapDetail(Career& career);
    void showHistory();
    void exportReport();
    void settings();
    void changePassword();

    // Helpers
    void openUserFiles();
    void closeUserFiles();
    void loadSettings();
    void saveSettings();
    bool hasProfile();
    AssessmentResult latestProfile();
    Career* chooseCareer(const std::string& title);
    void startTracking(Career& career);
    std::vector<std::string> profileLines(const std::vector<double>& traits);
    std::string confidenceMessage(double gap);
    std::vector<std::string> dendrogramLines(const std::vector<double>& traits, bool allCareers);

public:
    CareerApp(const std::string& folder);
    ~CareerApp();
    int run();
};

// ======================================================================
// Utils
// ======================================================================

// Utils.cpp
// Implementation of the helper functions declared in Utils.h.

bool useColor = true;

// ======================= Console setup =======================

void setupConsole() {
#ifdef _WIN32
    // Ask Windows to understand ANSI colour codes.
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(output, &mode)) {
        SetConsoleMode(output, mode | 0x0004);
    }
#endif
}

void clearScreen() {
    if (useColor) {
        cout << "\033[2J\033[H";   // ANSI: clear screen and move cursor to top
    } else {
        cout << "\n\n";
    }
}

string color(const string& text, const string& code) {
    if (!useColor) return text;
    return "\033[" + code + "m" + text + "\033[0m";
}

// ======================= Text helpers =======================

string padRight(const string& text, int width) {
    if ((int)text.size() >= width) return text;
    return text + string(width - text.size(), ' ');
}

string padLeft(const string& text, int width) {
    if ((int)text.size() >= width) return text;
    return string(width - text.size(), ' ') + text;
}

string cutText(const string& text, int width) {
    if ((int)text.size() <= width) return text;
    return text.substr(0, width - 3) + "...";
}

vector<string> wrapText(const string& text, int width) {
    vector<string> lines;
    stringstream words(text);
    string word, line;
    while (words >> word) {
        if (!line.empty() && (int)(line.size() + 1 + word.size()) > width) {
            lines.push_back(line);
            line = "";
        }
        if (!line.empty()) line += " ";
        line += word;
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

vector<string> splitText(const string& text, char separator) {
    vector<string> parts;
    string part;
    stringstream input(text);
    while (getline(input, part, separator)) {
        parts.push_back(part);
    }
    if (!text.empty() && text[text.size() - 1] == separator) parts.push_back("");
    return parts;
}

string trimText(const string& text) {
    int start = 0;
    int end = (int)text.size() - 1;
    while (start <= end && isspace((unsigned char)text[start])) start++;
    while (end >= start && isspace((unsigned char)text[end])) end--;
    return text.substr(start, end - start + 1);
}

string toLowerText(string text) {
    for (int i = 0; i < (int)text.size(); i++) {
        text[i] = (char)tolower((unsigned char)text[i]);
    }
    return text;
}

string percent(double value) {
    return toText((int)(value * 100 + 0.5)) + "%";
}

string signedPoints(double change) {
    int points = (int)(change * 100 + (change >= 0 ? 0.5 : -0.5));
    if (points > 0) return "+" + toText(points);
    return toText(points);
}

string formatNumber(double value, int decimals) {
    stringstream out;
    out << fixed << setprecision(decimals) << value;
    return out.str();
}

string toText(long long number) {
    stringstream out;
    out << number;
    return out.str();
}

int toInt(const string& text) {
    int number = 0;
    stringstream in(text);
    in >> number;
    return number;
}

double toDouble(const string& text) {
    double number = 0;
    stringstream in(text);
    in >> number;
    return number;
}

long long toLongLong(const string& text) {
    long long number = 0;
    stringstream in(text);
    in >> number;
    return number;
}

// ======================= Drawing =======================

string makeBar(double fraction, int width) {
    fraction = limitValue(fraction, 0.0, 1.0);
    int filled = (int)(fraction * width + 0.5);
    return string(filled, '#') + string(width - filled, '.');
}

string colorBar(double fraction, int width, const string& code) {
    fraction = limitValue(fraction, 0.0, 1.0);
    int filled = (int)(fraction * width + 0.5);
    return color(string(filled, '#'), code) + color(string(width - filled, '.'), GREY);
}

void printBanner() {
    string border = "+" + string(SCREEN_WIDTH - 2, '=') + "+";
    string title = "C A R E E R    D E N D R O G R A M";
    string subtitle = "Career guidance using your profile and hierarchical clustering";
    int inner = SCREEN_WIDTH - 2;

    cout << color(border, CYAN) << "\n";
    cout << color("|", CYAN) << string(inner, ' ') << color("|", CYAN) << "\n";
    int left = (inner - (int)title.size()) / 2;
    cout << color("|", CYAN) << string(left, ' ') << color(title, BOLD)
         << string(inner - left - title.size(), ' ') << color("|", CYAN) << "\n";
    left = (inner - (int)subtitle.size()) / 2;
    cout << color("|", CYAN) << string(left, ' ') << color(subtitle, GREY)
         << string(inner - left - subtitle.size(), ' ') << color("|", CYAN) << "\n";
    cout << color("|", CYAN) << string(inner, ' ') << color("|", CYAN) << "\n";
    cout << color(border, CYAN) << "\n";
}

// Prints a screen header like:  == Dashboard ============== Home / Dashboard
void printTitle(const string& title, const string& path) {
    clearScreen();
    string left = "== " + title + " ";
    string right = " " + path;
    int fill = SCREEN_WIDTH - (int)left.size() - (int)right.size();
    if (fill < 2) fill = 2;
    cout << color(left, BOLD) << color(string(fill, '='), CYAN) << color(right, GREY) << "\n\n";
}

// Prints plain-text lines inside a box with a title.
void printBox(const string& title, const vector<string>& lines) {
    int inner = SCREEN_WIDTH - 4;
    string top = "+- " + title + " ";
    top += string(SCREEN_WIDTH - 1 - top.size(), '-') + "+";
    cout << color(top, GREY) << "\n";
    for (int i = 0; i < (int)lines.size(); i++) {
        cout << color("|", GREY) << " " << padRight(cutText(lines[i], inner), inner) << " " << color("|", GREY) << "\n";
    }
    cout << color("+" + string(SCREEN_WIDTH - 2, '-') + "+", GREY) << "\n";
}

void printHeading(const string& text) {
    cout << "\n  " << color(text, BOLD) << "\n";
    cout << "  " << color(string(text.size(), '-'), GREY) << "\n";
}

void showInfo(const string& message)    { cout << color("  [i]  ", CYAN) << message << "\n"; }
void showSuccess(const string& message) { cout << color("  [ok] ", GREEN) << message << "\n"; }
void showWarning(const string& message) { cout << color("  [!]  ", YELLOW) << message << "\n"; }
void showError(const string& message)   { cout << color("  [x]  ", RED) << message << "\n"; }

// ======================= Input =======================

// Reads one line. If the input has ended (for example Ctrl+Z / Ctrl+D),
// the program exits cleanly. Data is saved after every change, so this is safe.
static string readLine() {
    string line;
    if (!getline(cin, line)) {
        cout << "\n";
        exit(0);
    }
    if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
    return line;
}

static bool keyboardIsConnected() {
#ifdef _WIN32
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(STDIN_FILENO) != 0;
#endif
}

string askText(const string& label) {
    cout << color("  > ", CYAN) << label << ": ";
    return trimText(readLine());
}

// Reads a password and shows * instead of the real characters.
string askPassword(const string& label) {
    cout << color("  > ", CYAN) << label << ": ";
    cout.flush();

    // When input comes from a file (for testing) we cannot hide it.
    if (!keyboardIsConnected()) return readLine();

    string password;
#ifdef _WIN32
    while (true) {
        int key = _getch();
        if (key == '\r' || key == '\n') break;
        if (key == 0 || key == 224) { _getch(); continue; }   // arrow keys etc.
        if (key == '\b') {
            if (!password.empty()) { password.erase(password.size() - 1); cout << "\b \b"; }
        } else if (key >= 32 && key < 127) {
            password += (char)key;
            cout << '*';
        }
    }
#else
    termios oldSettings, newSettings;
    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~(ICANON | ECHO);   // turn off echo and line mode
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
    while (true) {
        int key = getchar();
        if (key == EOF || key == '\n' || key == '\r') break;
        if (key == 127 || key == '\b') {
            if (!password.empty()) { password.erase(password.size() - 1); cout << "\b \b"; }
        } else if (key >= 32 && key < 127) {
            password += (char)key;
            cout << '*';
        }
        cout.flush();
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
#endif
    cout << "\n";
    return password;
}

int askNumber(const string& label, int minValue, int maxValue) {
    while (true) {
        string range = color(" [" + toText(minValue) + "-" + toText(maxValue) + "]", GREY);
        string answer = askText(label + range);

        bool allDigits = !answer.empty() && answer.size() < 6;
        for (int i = 0; i < (int)answer.size(); i++) {
            if (!isdigit((unsigned char)answer[i])) allDigits = false;
        }
        if (allDigits) {
            int number = toInt(answer);
            if (number >= minValue && number <= maxValue) return number;
        }
        showError("Please enter a number from " + toText(minValue) + " to " + toText(maxValue) + ".");
    }
}

bool askYesNo(const string& question) {
    while (true) {
        string answer = toLowerText(askText(question + color(" (y/n)", GREY)));
        if (answer == "y" || answer == "yes") return true;
        if (answer == "n" || answer == "no") return false;
        showError("Please type y or n.");
    }
}

void waitForEnter() {
    cout << "\n" << color("  Press Enter to continue...", GREY);
    readLine();
}

MenuOption makeOption(int number, const string& text, const string& hint) {
    MenuOption option;
    option.number = number;
    option.text = text;
    option.hint = hint;
    return option;
}

// Shows a numbered menu and returns the number the user picked.
int showMenu(const vector<MenuOption>& options) {
    cout << "\n";
    int highest = 0;
    for (int i = 0; i < (int)options.size(); i++) {
        string number = padLeft("[" + toText(options[i].number) + "]", 4);
        cout << "  " << color(number, CYAN) << "  " << padRight(options[i].text, 30) << color(options[i].hint, GREY) << "\n";
        if (options[i].number > highest) highest = options[i].number;
    }
    cout << "\n";

    while (true) {
        int choice = askNumber("Select an option", 0, highest);
        for (int i = 0; i < (int)options.size(); i++) {
            if (options[i].number == choice) return choice;
        }
        showError("That option is not in the list.");
    }
}

// ======================= Files and folders =======================

// Creates every folder along the path, e.g. "data/profiles/yashas".
void makeFolder(const string& path) {
    string current;
    for (int i = 0; i < (int)path.size(); i++) {
        char c = path[i];
        if ((c == '/' || c == '\\') && !current.empty()) {
#ifdef _WIN32
            _mkdir(current.c_str());
#else
            mkdir(current.c_str(), 0755);
#endif
        }
        current += c;
    }
#ifdef _WIN32
    _mkdir(current.c_str());
#else
    mkdir(current.c_str(), 0755);
#endif
}

// For "data/users.db" this creates the folder "data".
void makeFolderFor(const string& fileName) {
    int lastSlash = -1;
    for (int i = 0; i < (int)fileName.size(); i++) {
        if (fileName[i] == '/' || fileName[i] == '\\') lastSlash = i;
    }
    if (lastSlash > 0) makeFolder(fileName.substr(0, lastSlash));
}

bool fileExists(const string& fileName) {
    ifstream file(fileName.c_str());
    return file.good();
}

// ======================= Time =======================

string currentTime() {
    time_t now = time(0);
    tm* local = localtime(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local);
    return buffer;
}

long long currentSeconds() {
    return (long long)time(0);
}

// ======================================================================
// Sha256
// ======================================================================

// Sha256.cpp
// A plain implementation of the SHA-256 hash (standard FIPS 180-4).
// You do not need to understand every line: it mixes the bits of the input
// in 64 rounds so that the output cannot be turned back into the input.

typedef unsigned int uint32;             // 32-bit number
typedef unsigned long long uint64;       // 64-bit number

static const uint32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

// Rotate the bits of x to the right by n places.
static uint32 rotateRight(uint32 x, int n) {
    return (x >> n) | (x << (32 - n));
}

string sha256(const string& text) {
    uint32 h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                   0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    // Step 1: padding. Add a 1 bit, then zeros, then the original length in bits.
    string data = text;
    uint64 bitLength = (uint64)text.size() * 8;
    data += (char)0x80;
    while (data.size() % 64 != 56) data += (char)0x00;
    for (int i = 7; i >= 0; i--) data += (char)((bitLength >> (i * 8)) & 0xff);

    // Step 2: process the data in blocks of 64 bytes.
    for (size_t block = 0; block < data.size(); block += 64) {
        uint32 w[64];
        for (int i = 0; i < 16; i++) {
            w[i] = ((uint32)(unsigned char)data[block + i * 4] << 24) |
                   ((uint32)(unsigned char)data[block + i * 4 + 1] << 16) |
                   ((uint32)(unsigned char)data[block + i * 4 + 2] << 8) |
                   ((uint32)(unsigned char)data[block + i * 4 + 3]);
        }
        for (int i = 16; i < 64; i++) {
            uint32 s0 = rotateRight(w[i - 15], 7) ^ rotateRight(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32 s1 = rotateRight(w[i - 2], 17) ^ rotateRight(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32 a = h[0], b = h[1], c = h[2], d = h[3];
        uint32 e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; i++) {
            uint32 s1 = rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25);
            uint32 choose = (e & f) ^ (~e & g);
            uint32 temp1 = hh + s1 + choose + K[i] + w[i];
            uint32 s0 = rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22);
            uint32 majority = (a & b) ^ (a & c) ^ (b & c);
            uint32 temp2 = s0 + majority;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    // Step 3: write the 8 numbers as hexadecimal text.
    string result;
    char piece[9];
    for (int i = 0; i < 8; i++) {
        snprintf(piece, sizeof(piece), "%08x", h[i]);
        result += piece;
    }
    return result;
}

// ======================================================================
// Models
// ======================================================================

// Models.cpp
// Code for the classes in Models.h.

const string TRAIT_NAMES[TRAIT_COUNT] = {
    "Realistic",    "Investigative", "Artistic",      "Social", "Enterprising",  "Conventional",
    "Quantitative", "Computing",     "Communication", "Design", "Life Sciences", "Business"};

// ======================= Maths =======================

// Pearson correlation: +1 means the two lists rise and fall together,
// 0 means no relation, -1 means opposite patterns.
double correlation(const vector<double>& a, const vector<double>& b) {
    double meanA = 0, meanB = 0;
    for (int i = 0; i < TRAIT_COUNT; i++) {
        meanA += a[i];
        meanB += b[i];
    }
    meanA /= TRAIT_COUNT;
    meanB /= TRAIT_COUNT;

    double top = 0, sumA = 0, sumB = 0;
    for (int i = 0; i < TRAIT_COUNT; i++) {
        double x = a[i] - meanA;
        double y = b[i] - meanB;
        top += x * y;
        sumA += x * x;
        sumB += y * y;
    }
    if (sumA == 0 || sumB == 0) return 0;
    return top / (sqrt(sumA) * sqrt(sumB));
}

// Straight-line (Euclidean) distance between two trait lists.
double traitDistance(const vector<double>& a, const vector<double>& b) {
    double sum = 0;
    for (int i = 0; i < TRAIT_COUNT; i++) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

// First letters of the three strongest interests, e.g. "ICR".
string hollandCode(const vector<double>& traits) {
    bool used[INTEREST_COUNT] = {false, false, false, false, false, false};
    string code;
    for (int round = 0; round < 3; round++) {
        int best = -1;
        for (int i = 0; i < INTEREST_COUNT; i++) {
            if (!used[i] && (best == -1 || traits[i] > traits[best])) best = i;
        }
        used[best] = true;
        code += TRAIT_NAMES[best][0];
    }
    return code;
}

string traitsToText(const vector<double>& traits) {
    string text;
    for (int i = 0; i < TRAIT_COUNT; i++) {
        if (i > 0) text += ",";
        text += formatNumber(traits[i], 3);
    }
    return text;
}

bool textToTraits(const string& text, vector<double>& traits) {
    vector<string> parts = splitText(text, ',');
    if ((int)parts.size() != TRAIT_COUNT) return false;
    traits.assign(TRAIT_COUNT, 0.0);
    for (int i = 0; i < TRAIT_COUNT; i++) traits[i] = toDouble(parts[i]);
    return true;
}

// ======================= Career =======================

Career::Career() {
    salaryMin = 0;
    salaryMax = 0;
    growth = 3;
}

// Line format: id|name|domain|12 traits (0-10)|salary min|salary max|growth|exams|summary|milestones
bool Career::fromLine(const string& line) {
    vector<string> field = splitText(line, '|');
    if (field.size() != 10) return false;

    id = trimText(field[0]);
    name = trimText(field[1]);
    domain = trimText(field[2]);
    exams = trimText(field[7]);
    summary = trimText(field[8]);

    vector<string> numbers = splitText(field[3], ',');
    if ((int)numbers.size() != TRAIT_COUNT || id.empty()) return false;
    traits.assign(TRAIT_COUNT, 0.0);
    for (int i = 0; i < TRAIT_COUNT; i++) traits[i] = toDouble(numbers[i]) / 10.0;
    salaryMin = toInt(field[4]);
    salaryMax = toInt(field[5]);
    growth = toInt(field[6]);

    milestones.clear();
    vector<string> steps = splitText(field[9], ';');
    for (int i = 0; i < (int)steps.size(); i++) {
        if (!trimText(steps[i]).empty()) milestones.push_back(trimText(steps[i]));
    }
    return true;
}

string Career::salaryText() const {
    if (salaryMin == 0) return "Highly variable";
    return "INR " + toText(salaryMin) + " - " + toText(salaryMax) + " LPA";
}

string Career::growthText() const {
    string names[5] = {"Declining", "Slow", "Stable", "Growing", "Rapid growth"};
    return names[limitValue(growth, 1, 5) - 1];
}

// ======================= CareerList =======================

// Built-in careers. Written to data/careers.csv the first time the program
// runs, so you can edit or add careers later without recompiling.
// Trait order: REA INV ART SOC ENT CON | QNT CMP COM DSN BIO BUS (each 0-10)
static const char* DEFAULT_CAREERS =
    "# Career list - id|name|domain|traits(12, 0-10)|salary min LPA|salary max LPA|growth 1-5|entry exams|summary|milestones (;)\n"
    "swe|Software Engineer|Technology|4,8,4,3,4,6,7,10,5,4,1,3|6|40|5|JEE Main / Advanced, GATE CS|Designs, builds and maintains software systems at scale.|Master one language and core data structures;Build three end-to-end projects;Contribute to an open-source project;Complete a software internship;Prepare for system design interviews\n"
    "ds|Data Scientist|Technology|2,10,3,3,4,6,10,8,6,3,2,5|8|35|5|JEE, GATE DA, IIT Madras BS in Data Science|Extracts insight and predictive models from data.|Build a statistics and probability foundation;Learn Python, SQL and pandas;Complete machine learning coursework;Publish a portfolio on real datasets;Complete an analytics internship\n"
    "mle|Machine Learning Engineer|Technology|3,10,3,2,4,5,10,10,4,3,1,3|10|50|5|JEE Advanced, GATE CS / DA|Builds, trains and deploys machine learning models in production.|Linear algebra and optimisation;Learn a deep learning framework;Reimplement two research papers;Learn MLOps and model deployment;Complete a research or industry internship\n"
    "cyber|Cybersecurity Analyst|Technology|5,8,2,3,3,8,6,9,5,2,1,3|6|30|5|GATE CS, CEH / OSCP certification|Protects systems and networks against attack.|Networking and operating systems fundamentals;Linux and scripting;Compete in capture-the-flag events;Earn a security certification;Complete a SOC or red-team internship\n"
    "cloud|Cloud / DevOps Engineer|Technology|5,7,2,3,3,8,5,9,4,2,1,3|6|35|5|AWS / Azure / GCP certifications|Automates infrastructure and keeps systems reliable.|Linux and networking;Docker and Kubernetes;Infrastructure as code;Earn a cloud certification;Complete an SRE internship\n"
    "gamedev|Game Developer|Technology|4,7,9,3,4,4,7,9,4,9,1,2|5|30|4|JEE Main, NID, portfolio-based hiring|Designs and programs interactive games.|C++ or C# fundamentals;Learn Unity or Unreal;Ship a small complete game;Take part in game jams;Complete a studio internship\n"
    "uxd|UX / Product Designer|Design|2,5,9,6,4,3,3,5,7,10,1,4|5|30|5|UCEED, CEED, NID DAT|Designs digital products around real user needs.|Design fundamentals and typography;Prototyping in Figma;User research methods;Build a case-study portfolio;Complete a design internship\n"
    "architect|Architect|Design|6,5,9,3,4,5,5,4,6,10,1,4|4|25|3|NATA, JEE Main Paper 2|Designs buildings and public spaces.|Sketching and visual composition;Complete B.Arch;AutoCAD and Revit;Practical training;Council of Architecture registration\n"
    "animator|Animator / VFX Artist|Media|3,3,10,2,3,3,2,5,4,10,1,2|3|18|4|NID, portfolio|Creates animation, motion graphics and visual effects.|Drawing and composition;Blender or Maya;Build a showreel;Complete a studio internship;Specialise in 3D or compositing\n"
    "journalist|Journalist / Content Strategist|Media|1,6,8,7,6,3,3,2,10,5,1,4|3|20|3|IIMC entrance, CUET|Investigates and communicates stories to the public.|Write every day;Build a published portfolio;Complete a media internship;Learn data journalism basics;Specialise in a beat\n"
    "civil|Civil Engineer|Engineering|9,6,3,3,4,7,7,3,4,6,1,3|4|15|3|JEE Main, GATE CE, ESE|Plans and builds infrastructure such as roads, bridges and water systems.|Mechanics and structural analysis;AutoCAD and STAAD.Pro;Complete a site internship;Prepare for GATE or ESE;Obtain professional licensure\n"
    "mech|Mechanical Engineer|Engineering|9,7,3,2,4,6,8,4,4,7,1,3|4|18|3|JEE Main / Advanced, GATE ME|Designs machines, engines and manufacturing systems.|Thermodynamics and mechanics;CAD with SolidWorks;Build a hardware project;Complete industrial training;Specialise in EV, robotics or HVAC\n"
    "ece|Electronics / VLSI Engineer|Engineering|7,9,2,2,3,6,9,7,3,5,1,2|7|35|4|JEE Advanced, GATE EC|Designs circuits, chips and embedded systems.|Circuit theory and digital logic;Verilog or VHDL;Embedded C and microcontrollers;Complete an FPGA project;Complete a semiconductor internship\n"
    "robotics|Robotics Engineer|Engineering|9,9,4,2,4,5,9,8,3,7,1,2|8|35|5|JEE Advanced, GATE ME / EC|Builds autonomous machines combining mechanics, electronics and software.|Control systems;ROS and C++;Build a working robot;Computer vision basics;Join a research lab or robotics startup\n"
    "aero|Aerospace Engineer|Engineering|8,10,3,2,3,6,10,7,4,7,1,2|7|30|4|JEE Advanced, GATE AE, ISRO recruitment|Designs aircraft, spacecraft and propulsion systems.|Fluid mechanics and aerodynamics;MATLAB and CFD;Rocketry or SAE team project;ISRO or DRDO internship;M.Tech or MS\n"
    "doctor|Medical Doctor|Healthcare|5,9,2,9,3,6,6,2,7,3,10,2|10|60|4|NEET UG, NEET PG|Diagnoses and treats patients.|Prepare for NEET UG;Complete MBBS;Complete the internship year;Clear NEET PG;Residency and specialisation\n"
    "pharma|Pharmacist / Pharma R&D|Healthcare|4,8,2,5,3,8,6,3,5,2,9,3|4|15|3|NEET / state CETs, GPAT|Develops, tests and dispenses medicines.|Pharmaceutical chemistry;Complete B.Pharm;Clear GPAT;Hospital or industry internship;Specialise in regulatory affairs or R&D\n"
    "psych|Clinical Psychologist|Healthcare|2,7,4,10,3,4,4,1,9,2,7,2|4|20|4|CUET, RCI-recognised M.Phil|Assesses and treats mental and behavioural health.|Psychology undergraduate degree;Research methods and statistics;Supervised practicum;RCI-recognised M.Phil;Licensure\n"
    "biotech|Biotechnologist|Science|5,10,2,3,3,6,7,5,4,2,10,2|5|20|4|JEE / NEET, IISER Aptitude Test, GATE BT|Engineers biological systems for medicine, agriculture and industry.|Molecular biology and genetics;Core laboratory techniques;Bioinformatics with Python;Complete a research project;MS or PhD\n"
    "research|Research Scientist|Science|3,10,4,3,3,5,9,7,6,3,5,1|8|30|4|IIT JAM, GATE, JEST, GRE|Pushes the frontier of knowledge through experiments and theory.|Strong core in mathematics and science;Summer research fellowship;Publish a paper;Clear JAM or GRE;PhD\n"
    "env|Environmental Scientist|Science|8,8,3,6,3,5,6,4,5,3,8,2|4|15|4|IIT JAM, GATE ES, IFoS (UPSC)|Studies ecosystems and designs solutions to environmental problems.|Ecology and earth science;GIS and remote sensing;Field research;Data analysis in R or Python;Policy or research role\n"
    "educator|Educator / Academic|Education|2,7,4,10,4,5,5,3,10,3,3,2|4|20|3|UGC NET, CSIR NET, B.Ed, PhD|Teaches and mentors, often combined with research.|Deep subject mastery;Teaching assistant role;Clear UGC or CSIR NET;Publish research;Secure a faculty position\n"
    "ca|Chartered Accountant|Finance|2,6,1,4,5,10,9,3,6,1,1,9|7|30|4|CA Foundation, Intermediate, Final (ICAI)|Audits accounts, advises on tax and manages financial reporting.|Clear CA Foundation;Clear CA Intermediate;Complete articleship;Clear CA Final;Specialise in audit, tax or advisory\n"
    "ib|Investment Banker|Finance|1,7,2,5,10,7,9,4,8,2,1,10|15|60|4|CAT, CFA, MBA|Advises on mergers, acquisitions and capital raising.|Accounting and corporate finance;Financial modelling in Excel;CFA Level 1;Front-office internship;MBA or analyst programme\n"
    "quant|Quantitative Analyst|Finance|1,10,2,2,5,7,10,9,4,2,1,7|20|80|5|JEE Advanced, mathematics olympiads|Builds mathematical models for trading and risk.|Probability and stochastic calculus;C++ and Python;Build an algorithmic trading project;Puzzle and probability interview preparation;Quant internship\n"
    "analyst|Business Analyst|Business|1,7,2,5,6,8,8,6,8,3,1,8|6|20|4|CAT, IIT Madras BS in Data Science, BI certifications|Translates data into business decisions.|Excel and SQL;Power BI or Tableau;Applied statistics;Case-study portfolio;Analyst internship\n"
    "pm|Product Manager|Business|2,6,4,6,9,4,6,6,9,6,1,8|12|45|5|CAT, engineering background preferred|Owns product strategy across users, engineering and business.|Understand software delivery;Analytics and product metrics;Write product specifications;Ship a side product;APM programme or internship\n"
    "consult|Management Consultant|Business|2,7,3,6,9,6,8,4,9,4,1,9|12|40|4|CAT / GMAT|Solves strategic problems for organisations.|Structured problem solving;Case interview practice;Excel and presentation skills;Hold a leadership position;MBA or business analyst role\n"
    "marketing|Marketing Manager|Business|1,5,7,7,9,4,5,3,9,7,1,8|6|30|4|CAT, MICAT|Shapes how products reach and persuade customers.|Consumer psychology;Digital marketing and analytics;Run a real campaign;Brand management internship;MBA in Marketing\n"
    "hr|Human Resources Manager|Business|1,4,3,9,7,7,3,2,9,2,1,7|5|20|3|CAT, XAT|Hires, develops and supports people in organisations.|Organisational behaviour;Communication and negotiation;HR analytics;HR internship;MBA in Human Resources\n"
    "founder|Entrepreneur / Founder|Business|4,6,6,5,10,3,6,6,8,6,2,10|0|0|4|No fixed exam; incubators and accelerators|Builds new ventures from idea to scale.|Identify a real problem worth solving;Build a minimum viable product;Acquire the first ten users;Join an incubator or accelerator;Raise funding or reach profitability\n"
    "lawyer|Lawyer|Law|1,7,4,6,8,7,4,1,10,1,1,5|5|40|3|CLAT, AILET|Represents clients and interprets the law.|Prepare for CLAT;Complete a five-year LL.B;Moot court competitions;Law-firm internships;Bar Council enrolment\n"
    "civilservice|Civil Services Officer|Public Service|3,7,2,8,8,7,5,2,9,2,2,5|8|25|3|UPSC CSE, State PSC|Administers public policy at district, state and national level.|NCERT foundation;Daily current-affairs habit;Choose an optional subject;Answer-writing practice;UPSC Prelims, Mains and Interview\n";

int CareerList::load(const string& fileName) {
    // Create the file from the built-in list if it does not exist yet.
    if (!fileExists(fileName)) {
        makeFolderFor(fileName);
        ofstream output(fileName.c_str());
        output << DEFAULT_CAREERS;
    }

    careers.clear();
    ifstream input(fileName.c_str());
    string line;
    while (getline(input, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        if (line.empty() || line[0] == '#') continue;
        Career career;
        if (career.fromLine(line) && findById(career.id) == NULL) {
            careers.push_back(career);
        }
    }
    return (int)careers.size();
}

vector<Career>& CareerList::getAll() {
    return careers;
}

Career* CareerList::findById(const string& id) {
    for (int i = 0; i < (int)careers.size(); i++) {
        if (careers[i].id == id) return &careers[i];
    }
    return NULL;
}

// Finds careers whose name, domain, summary or exams contain the word.
vector<Career*> CareerList::search(const string& word) {
    string target = toLowerText(trimText(word));
    vector<Career*> results;
    for (int i = 0; i < (int)careers.size(); i++) {
        Career& c = careers[i];
        string allText = toLowerText(c.name + " " + c.domain + " " + c.summary + " " + c.exams);
        if (target.empty() || allText.find(target) != string::npos) {
            results.push_back(&c);
        }
    }
    return results;
}

vector<string> CareerList::getDomains() {
    vector<string> domains;
    for (int i = 0; i < (int)careers.size(); i++) {
        bool alreadyAdded = false;
        for (int j = 0; j < (int)domains.size(); j++) {
            if (domains[j] == careers[i].domain) alreadyAdded = true;
        }
        if (!alreadyAdded) domains.push_back(careers[i].domain);
    }
    return domains;
}

// ======================= User =======================

User::User() {
    failedAttempts = 0;
    lockedUntil = 0;
}

string User::getKey() const {
    return username;
}

string User::toLine() const {
    return username + "|" + salt + "|" + passwordHash + "|" + fullName + "|" + createdAt + "|" +
           toText(failedAttempts) + "|" + toText(lockedUntil);
}

bool User::fromLine(const string& line) {
    vector<string> field = splitText(line, '|');
    if (field.size() != 7) return false;
    username = field[0];
    salt = field[1];
    passwordHash = field[2];
    fullName = field[3];
    createdAt = field[4];
    failedAttempts = toInt(field[5]);
    lockedUntil = toLongLong(field[6]);
    return !username.empty() && passwordHash.size() == 64;
}

// ======================= AssessmentResult =======================

AssessmentResult::AssessmentResult() {
    traits.assign(TRAIT_COUNT, 0.0);
    confidenceGap = 0;
}

string AssessmentResult::getKey() const {
    return date;
}

string AssessmentResult::toLine() const {
    return date + "|" + traitsToText(traits) + "|" + formatNumber(confidenceGap, 3);
}

bool AssessmentResult::fromLine(const string& line) {
    vector<string> field = splitText(line, '|');
    if (field.size() != 3) return false;
    date = field[0];
    if (!textToTraits(field[1], traits)) return false;
    confidenceGap = toDouble(field[2]);
    return true;
}

// ======================= RoadmapProgress =======================

string RoadmapProgress::getKey() const {
    return careerId;
}

string RoadmapProgress::toLine() const {
    return careerId + "|" + done;
}

bool RoadmapProgress::fromLine(const string& line) {
    vector<string> field = splitText(line, '|');
    if (field.size() != 2 || field[0].empty()) return false;
    careerId = field[0];
    done = field[1];
    for (int i = 0; i < (int)done.size(); i++) {
        if (done[i] != '0' && done[i] != '1') return false;
    }
    return true;
}

int RoadmapProgress::countDone() const {
    int total = 0;
    for (int i = 0; i < (int)done.size(); i++) {
        if (done[i] == '1') total++;
    }
    return total;
}

// ======================================================================
// Auth
// ======================================================================

// Auth.cpp
// Code for the AuthManager class.

AuthManager::AuthManager(const string& fileName)
    : users(fileName, "users - username|salt|password hash|full name|created|failed attempts|locked until") {
    users.load();
}

// Username rules: 3-20 characters, starts with a letter,
// only lowercase letters, digits and underscore.
bool AuthManager::isValidUsername(const string& username) {
    if (username.size() < 3 || username.size() > 20) return false;
    if (!islower((unsigned char)username[0])) return false;
    for (int i = 0; i < (int)username.size(); i++) {
        char c = username[i];
        bool ok = islower((unsigned char)c) || isdigit((unsigned char)c) || c == '_';
        if (!ok) return false;
    }
    return true;
}

// Gives a score from 0 to 4 based on length and the kinds of characters used.
int AuthManager::passwordStrength(const string& password) {
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSymbol = false;
    for (int i = 0; i < (int)password.size(); i++) {
        char c = password[i];
        if (islower((unsigned char)c)) hasLower = true;
        else if (isupper((unsigned char)c)) hasUpper = true;
        else if (isdigit((unsigned char)c)) hasDigit = true;
        else hasSymbol = true;
    }
    int kinds = hasLower + hasUpper + hasDigit + hasSymbol;

    int score = 0;
    if (password.size() >= 8) score++;
    if (password.size() >= 12) score++;
    if (kinds >= 2) score++;
    if (kinds >= 3) score++;
    if (password.size() < 8 && score > 1) score = 1;   // short passwords are always weak
    return score;
}

string AuthManager::strengthName(int strength) {
    string names[5] = {"Very weak", "Weak", "Fair", "Good", "Strong"};
    return names[limitValue(strength, 0, 4)];
}

// A salt is random text added to the password before hashing, so two users
// with the same password still get different hashes.
string AuthManager::makeSalt() {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(0));   // start the random numbers from the current time
        seeded = true;
    }
    string letters = "0123456789abcdef";
    string salt;
    for (int i = 0; i < 16; i++) {
        salt += letters[rand() % 16];
    }
    return salt;
}

string AuthManager::hashPassword(const string& salt, const string& password) {
    string hash = sha256(salt + ":" + password);
    for (int i = 0; i < HASH_ROUNDS; i++) {
        hash = sha256(hash + salt);
    }
    return hash;
}

string AuthManager::registerUser(const string& username, const string& fullName, const string& password) {
    if (!isValidUsername(username)) {
        return "Username must be 3-20 characters: lowercase letters, digits or underscore, starting with a letter.";
    }
    if (users.find(username) != NULL) {
        return "That username is already taken.";
    }
    if (password.size() < 8 || passwordStrength(password) < 2) {
        return "Password must be at least 8 characters and mix two kinds of characters (e.g. letters and digits).";
    }

    User user;
    user.username = username;
    user.fullName = fullName.empty() ? username : fullName;
    user.salt = makeSalt();
    user.passwordHash = hashPassword(user.salt, password);
    user.createdAt = currentTime();

    users.addOrUpdate(user);
    if (!users.save()) return "Could not save the user file.";
    return "";
}

bool AuthManager::login(const string& username, const string& password, User& user, string& message) {
    User* found = users.find(username);
    if (found == NULL) {
        message = "Invalid username or password.";
        return false;
    }

    if (found->lockedUntil > currentSeconds()) {
        message = "Account locked. Try again in " + toText(secondsLocked(username)) + " seconds.";
        return false;
    }

    if (hashPassword(found->salt, password) != found->passwordHash) {
        found->failedAttempts++;
        if (found->failedAttempts >= MAX_LOGIN_ATTEMPTS) {
            found->failedAttempts = 0;
            found->lockedUntil = currentSeconds() + LOCK_SECONDS;
            message = "Too many wrong attempts. Account locked for " + toText(LOCK_SECONDS) + " seconds.";
        } else {
            message = "Invalid username or password. " + toText(attemptsLeft(username)) + " attempt(s) left.";
        }
        users.save();
        return false;
    }

    // Correct password: reset the counters.
    found->failedAttempts = 0;
    found->lockedUntil = 0;
    users.save();
    user = *found;
    return true;
}

string AuthManager::changePassword(const string& username, const string& oldPassword, const string& newPassword) {
    User user;
    string message;
    if (!login(username, oldPassword, user, message)) return message;
    if (newPassword.size() < 8 || passwordStrength(newPassword) < 2) {
        return "New password is too weak.";
    }

    User* stored = users.find(username);
    stored->salt = makeSalt();
    stored->passwordHash = hashPassword(stored->salt, newPassword);
    if (!users.save()) return "Could not save the user file.";
    return "";
}

int AuthManager::secondsLocked(const string& username) {
    User* user = users.find(username);
    if (user == NULL) return 0;
    long long left = user->lockedUntil - currentSeconds();
    return left > 0 ? (int)left : 0;
}

int AuthManager::attemptsLeft(const string& username) {
    User* user = users.find(username);
    if (user == NULL) return MAX_LOGIN_ATTEMPTS;
    return MAX_LOGIN_ATTEMPTS - user->failedAttempts;
}

int AuthManager::userCount() {
    return users.count();
}

// ======================================================================
// Quiz
// ======================================================================

// Quiz.cpp
// Asks the 28 assessment questions and turns the answers into 12 trait scores.

// The three kinds of question.
const int INTEREST = 0;   // answer 1-5: how much would you enjoy it
const int SKILL = 1;      // answer 1-5: how good are you at it
const int TEST = 2;       // answer a-d: one correct option

struct Question {
    int kind;
    int trait;                  // which of the 12 traits this question measures
    string text;
    vector<string> options;     // only for TEST questions
    int correctOption;          // only for TEST questions (0 = a, 1 = b, ...)
};

static Question makeQuestion(int kind, int trait, const string& text) {
    Question q;
    q.kind = kind;
    q.trait = trait;
    q.text = text;
    q.correctOption = -1;
    return q;
}

static Question makeTest(int trait, const string& text, const string& a, const string& b,
                         const string& c, const string& d, int correct) {
    Question q = makeQuestion(TEST, trait, text);
    q.options.push_back(a);
    q.options.push_back(b);
    q.options.push_back(c);
    q.options.push_back(d);
    q.correctOption = correct;
    return q;
}

static vector<Question> buildQuestions() {
    vector<Question> list;

    // Part 1: interests (trait numbers 0-5)
    list.push_back(makeQuestion(INTEREST, 0, "Assembling, repairing or building physical things"));
    list.push_back(makeQuestion(INTEREST, 0, "Working hands-on with machines, tools or hardware"));
    list.push_back(makeQuestion(INTEREST, 0, "Seeing a tangible, physical result of my work"));
    list.push_back(makeQuestion(INTEREST, 1, "Figuring out why something works the way it does"));
    list.push_back(makeQuestion(INTEREST, 1, "Solving hard analytical problems or puzzles"));
    list.push_back(makeQuestion(INTEREST, 1, "Reading research or technical material out of curiosity"));
    list.push_back(makeQuestion(INTEREST, 2, "Creating original work: writing, music, visuals or media"));
    list.push_back(makeQuestion(INTEREST, 2, "Working without a fixed structure or rulebook"));
    list.push_back(makeQuestion(INTEREST, 2, "Expressing ideas through design or storytelling"));
    list.push_back(makeQuestion(INTEREST, 3, "Explaining concepts and helping others learn"));
    list.push_back(makeQuestion(INTEREST, 3, "Working closely with people to solve their problems"));
    list.push_back(makeQuestion(INTEREST, 3, "Contributing to community or social causes"));
    list.push_back(makeQuestion(INTEREST, 4, "Leading a team toward an ambitious goal"));
    list.push_back(makeQuestion(INTEREST, 4, "Persuading, pitching or negotiating with others"));
    list.push_back(makeQuestion(INTEREST, 4, "Starting new initiatives and taking calculated risks"));
    list.push_back(makeQuestion(INTEREST, 5, "Organising data, records or schedules precisely"));
    list.push_back(makeQuestion(INTEREST, 5, "Following well-defined procedures and standards"));
    list.push_back(makeQuestion(INTEREST, 5, "Checking work carefully for errors and consistency"));

    // Part 2: self-rated skills (trait numbers 6-11)
    list.push_back(makeQuestion(SKILL, 6, "Quantitative reasoning: algebra, statistics, estimation"));
    list.push_back(makeQuestion(SKILL, 7, "Programming and computational thinking"));
    list.push_back(makeQuestion(SKILL, 8, "Written and spoken communication"));
    list.push_back(makeQuestion(SKILL, 9, "Visual design and spatial thinking"));
    list.push_back(makeQuestion(SKILL, 10, "Biology, chemistry and the life sciences"));
    list.push_back(makeQuestion(SKILL, 11, "Business, economics and how markets work"));

    // Part 3: short tests for Quantitative (6) and Computing (7)
    list.push_back(makeTest(6, "A train covers 120 km in 1.5 hours. At the same speed, how far does it go in 2.25 hours?",
                            "160 km", "180 km", "190 km", "200 km", 1));
    list.push_back(makeTest(6, "If 3x + 7 = 25, what is the value of 2x - 1?", "11", "12", "13", "9", 0));
    list.push_back(makeTest(7, "x = 0; for i = 1 to 4: x = x + i. What is the final value of x?", "6", "10", "4", "15", 1));
    list.push_back(makeTest(7, "Which data structure serves elements in first-in, first-out order?",
                            "Stack", "Queue", "Binary tree", "Hash map", 1));
    return list;
}

static void showQuestion(const Question& q, int number, int total, int previousAnswer) {
    string part;
    if (q.kind == INTEREST) part = "Part 1 of 3: Interests";
    else if (q.kind == SKILL) part = "Part 2 of 3: Abilities";
    else part = "Part 3 of 3: Quick check";
    printTitle("Career Assessment", part);

    double progress = (double)number / total;
    cout << "  " << color("Question " + toText(number + 1) + " of " + toText(total), GREY) << "   "
         << colorBar(progress, 40, CYAN) << " " << color(percent(progress), GREY) << "\n\n";

    if (q.kind == INTEREST) cout << color("  How much would you enjoy:", GREY) << "\n";
    else if (q.kind == SKILL) cout << color("  Rate your current ability in:", GREY) << "\n";
    else cout << color("  Choose the correct answer:", GREY) << "\n";

    vector<string> lines = wrapText(q.text, SCREEN_WIDTH - 6);
    for (int i = 0; i < (int)lines.size(); i++) cout << "    " << color(lines[i], BOLD) << "\n";
    cout << "\n";

    // Print the choices. The previously chosen one (if any) is highlighted.
    if (q.kind == TEST) {
        for (int i = 0; i < (int)q.options.size(); i++) {
            string key = string("[") + (char)('a' + i) + "]";
            string code = (previousAnswer == i) ? PINK : CYAN;
            cout << "    " << color(key, code) << " " << q.options[i] << "\n";
        }
    } else {
        string interestWords[5] = {"Strongly dislike", "Dislike", "Neutral", "Enjoy", "Strongly enjoy"};
        string skillWords[5] = {"Beginner", "Basic", "Competent", "Strong", "Expert"};
        for (int i = 0; i < 5; i++) {
            string word = (q.kind == INTEREST) ? interestWords[i] : skillWords[i];
            string code = (previousAnswer == i + 1) ? PINK : CYAN;
            cout << "    " << color("[" + toText(i + 1) + "]", code) << " " << word << "\n";
        }
    }
    cout << "\n  " << color("p = previous question    q = quit without saving", GREY) << "\n\n";
}

// Converts what the user typed into an answer.
// Returns the answer, or -1 = invalid, -2 = previous, -3 = quit.
static int readAnswer(const Question& q, string typed) {
    typed = toLowerText(typed);
    if (typed == "p") return -2;
    if (typed == "q") return -3;
    if (typed.size() != 1) return -1;

    char c = typed[0];
    if (q.kind == TEST) {
        if (c >= 'a' && c <= 'd') return c - 'a';
        if (c >= '1' && c <= '4') return c - '1';
        return -1;
    }
    if (c >= '1' && c <= '5') return c - '0';
    return -1;
}

bool runAssessment(vector<double>& traits, double& confidenceGap, int& quizScore, int& quizTotal) {
    vector<Question> questions = buildQuestions();
    int total = (int)questions.size();
    vector<int> answers(total, -1);

    // Ask the questions. The user can move back with 'p'.
    int current = 0;
    while (current < total) {
        showQuestion(questions[current], current, total, answers[current]);
        int answer = readAnswer(questions[current], askText("Your answer"));

        if (answer == -1) {
            showError("Please use one of the keys shown in brackets.");
            waitForEnter();
        } else if (answer == -2) {
            if (current > 0) current--;
        } else if (answer == -3) {
            if (askYesNo("Quit and discard your answers?")) return false;
        } else {
            answers[current] = answer;
            current++;
        }
    }

    // Add up the answers for each trait.
    double selfSum[TRAIT_COUNT] = {0};
    int selfCount[TRAIT_COUNT] = {0};
    double testSum[TRAIT_COUNT] = {0};
    int testCount[TRAIT_COUNT] = {0};
    quizScore = 0;
    quizTotal = 0;

    for (int i = 0; i < total; i++) {
        Question& q = questions[i];
        if (q.kind == TEST) {
            bool correct = (answers[i] == q.correctOption);
            if (correct) {
                testSum[q.trait] += 1;
                quizScore++;
            }
            testCount[q.trait]++;
            quizTotal++;
        } else {
            selfSum[q.trait] += (answers[i] - 1) / 4.0;   // turns 1..5 into 0..1
            selfCount[q.trait]++;
        }
    }

    // Final score per trait. For tested traits we average the self-rating
    // and the test result, and remember how different they were.
    traits.assign(TRAIT_COUNT, 0.0);
    double gapTotal = 0;
    int testedTraits = 0;
    for (int t = 0; t < TRAIT_COUNT; t++) {
        double self = selfCount[t] > 0 ? selfSum[t] / selfCount[t] : 0;
        if (testCount[t] > 0) {
            double test = testSum[t] / testCount[t];
            traits[t] = 0.5 * self + 0.5 * test;
            gapTotal += self - test;
            testedTraits++;
        } else {
            traits[t] = self;
        }
    }
    confidenceGap = testedTraits > 0 ? gapTotal / testedTraits : 0;
    return true;
}

// ======================================================================
// Engine
// ======================================================================

// Engine.cpp
// Scoring careers and building the career dendrogram.

// ======================= Scoring =======================
//
// fit = 0.65 * shape + 0.35 * level
//   shape: correlation between the two trait lists (same pattern of strengths?)
//   level: how close the actual numbers are (1 = identical)
// Using both avoids two problems: shape alone rates a weak and a strong
// student the same; level alone favours "average" students.

Match scoreCareer(const vector<double>& student, Career& career) {
    Match m;
    m.career = &career;
    m.shape = (correlation(student, career.traits) + 1) / 2;     // -1..1 becomes 0..1

    double farthest = 0.5 * sqrt((double)TRAIT_COUNT);           // treat this distance as "no match"
    m.level = 1 - traitDistance(student, career.traits) / farthest;
    if (m.level < 0) m.level = 0;

    m.fit = SHAPE_WEIGHT * m.shape + (1 - SHAPE_WEIGHT) * m.level;

    // Strengths: traits the career needs (>= 0.5) where the student is also good (>= 0.4).
    // Pick the three with the largest (student x career) value.
    vector<bool> picked(TRAIT_COUNT, false);
    for (int round = 0; round < 3; round++) {
        int best = -1;
        for (int t = 0; t < TRAIT_COUNT; t++) {
            if (picked[t] || career.traits[t] < 0.5 || student[t] < 0.4) continue;
            if (best == -1 || student[t] * career.traits[t] > student[best] * career.traits[best]) best = t;
        }
        if (best == -1) break;
        picked[best] = true;
        m.strengths.push_back(best);
    }

    // Gaps: traits where the career needs clearly more than the student has.
    for (int t = 0; t < TRAIT_COUNT; t++) {
        double shortBy = career.traits[t] - student[t];
        if (shortBy > GAP_LIMIT) {
            m.gapTraits.push_back(t);
            m.gapSizes.push_back(shortBy);
        }
    }
    // Sort gaps from biggest to smallest (simple selection sort).
    for (int i = 0; i < (int)m.gapSizes.size(); i++) {
        for (int j = i + 1; j < (int)m.gapSizes.size(); j++) {
            if (m.gapSizes[j] > m.gapSizes[i]) {
                swap(m.gapSizes[i], m.gapSizes[j]);
                swap(m.gapTraits[i], m.gapTraits[j]);
            }
        }
    }
    return m;
}

static bool higherFit(const Match& a, const Match& b) {
    return a.fit > b.fit;
}

vector<Match> rankCareers(const vector<double>& student, CareerList& careers) {
    vector<Match> results;
    vector<Career>& all = careers.getAll();
    for (int i = 0; i < (int)all.size(); i++) {
        results.push_back(scoreCareer(student, all[i]));
    }
    stable_sort(results.begin(), results.end(), higherFit);
    return results;
}

// ======================= Dendrogram =======================

Dendrogram::Dendrogram() {
    leafCount = 0;
    root = -1;
}

// Steps:
//   1. Every point starts as its own group.
//   2. Find the two closest groups and join them into a new group.
//   3. The distance from the new group to any other group is the
//      average of its two parts' distances (weighted by size).
//   4. Repeat until only one group is left - that is the root.
void Dendrogram::build(const vector<string>& labels, const vector<vector<double> >& points) {
    nodes.clear();
    leafCount = (int)points.size();
    root = -1;
    if (leafCount == 0) return;

    for (int i = 0; i < leafCount; i++) {
        Node leaf;
        leaf.left = -1;
        leaf.right = -1;
        leaf.parent = -1;
        leaf.height = 0;
        leaf.size = 1;
        leaf.label = labels[i];
        nodes.push_back(leaf);
    }

    // Distance table big enough for all leaves and all future groups.
    int maxNodes = 2 * leafCount - 1;
    vector<vector<double> > dist(maxNodes, vector<double>(maxNodes, 0.0));
    for (int i = 0; i < leafCount; i++) {
        for (int j = 0; j < leafCount; j++) {
            dist[i][j] = traitDistance(points[i], points[j]);
        }
    }

    vector<int> active;   // groups not yet joined into something bigger
    for (int i = 0; i < leafCount; i++) active.push_back(i);

    while (active.size() > 1) {
        // Step 2: find the closest pair.
        int bestA = 0, bestB = 1;
        double bestDistance = dist[active[0]][active[1]];
        for (int i = 0; i < (int)active.size(); i++) {
            for (int j = i + 1; j < (int)active.size(); j++) {
                if (dist[active[i]][active[j]] < bestDistance) {
                    bestDistance = dist[active[i]][active[j]];
                    bestA = i;
                    bestB = j;
                }
            }
        }
        int a = active[bestA];
        int b = active[bestB];

        Node group;
        group.left = a;
        group.right = b;
        group.parent = -1;
        group.height = max(bestDistance, max(nodes[a].height, nodes[b].height));
        group.size = nodes[a].size + nodes[b].size;
        group.label = "";
        int newId = (int)nodes.size();
        nodes.push_back(group);
        nodes[a].parent = newId;
        nodes[b].parent = newId;

        // Step 3: distance from the new group to every other active group.
        for (int i = 0; i < (int)active.size(); i++) {
            int other = active[i];
            if (other == a || other == b) continue;
            double d = (dist[a][other] * nodes[a].size + dist[b][other] * nodes[b].size) / group.size;
            dist[newId][other] = d;
            dist[other][newId] = d;
        }

        // Replace a and b with the new group (erase the later index first).
        active.erase(active.begin() + bestB);
        active.erase(active.begin() + bestA);
        active.push_back(newId);
    }
    root = active[0];
}

// Adds the leaf numbers under 'node' to the list, from top to bottom.
void Dendrogram::collectLeaves(int node, vector<int>& leaves) {
    if (nodes[node].left == -1) {
        leaves.push_back(node);
        return;
    }
    collectLeaves(nodes[node].left, leaves);
    collectLeaves(nodes[node].right, leaves);
}

vector<int> Dendrogram::familyOf(int leaf, int minSize) {
    vector<int> leaves;
    if (root == -1) return leaves;
    int node = leaf;
    while (nodes[node].size < minSize && nodes[node].parent != -1) {
        node = nodes[node].parent;
    }
    collectLeaves(node, leaves);
    return leaves;
}

void Dendrogram::cutNode(int node, double limit, vector<vector<int> >& groups) {
    if (nodes[node].left == -1 || nodes[node].height <= limit) {
        vector<int> leaves;
        collectLeaves(node, leaves);
        groups.push_back(leaves);
        return;
    }
    cutNode(nodes[node].left, limit, groups);
    cutNode(nodes[node].right, limit, groups);
}

vector<vector<int> > Dendrogram::cutTree(double fraction) {
    vector<vector<int> > groups;
    if (root == -1) return groups;
    cutNode(root, fraction * nodes[root].height, groups);
    return groups;
}

// Drawing idea:
//   - Each leaf gets its own row (with an empty row between leaves).
//   - A group is drawn in the column that matches its joining height,
//     halfway between the rows of its two children.
//   - We draw on a grid of characters using '-', '|' and '+'.
vector<string> Dendrogram::draw(int highlightLeaf, int width) {
    vector<string> lines;
    if (root == -1) return lines;

    int total = (int)nodes.size();
    vector<int> row(total, 0);
    vector<int> col(total, 0);

    // Rows for leaves, in tree order.
    vector<int> order;
    collectLeaves(root, order);
    for (int i = 0; i < (int)order.size(); i++) row[order[i]] = 2 * i;

    // Rows and columns for groups. Children always have smaller numbers
    // than their parent, so one pass from low to high works.
    double treeHeight = nodes[root].height;
    if (treeHeight <= 0) treeHeight = 1;
    for (int n = leafCount; n < total; n++) {
        int l = nodes[n].left;
        int r = nodes[n].right;
        row[n] = (row[l] + row[r]) / 2;
        col[n] = (int)(nodes[n].height / treeHeight * (width - 4) + 0.5);
        col[n] = max(col[n], max(col[l], col[r]) + 1);   // always right of its children
    }

    int rows = 2 * leafCount - 1;
    int cols = col[root] + 3;
    vector<string> grid(rows, string(cols, ' '));
    vector<vector<bool> > highlighted(rows, vector<bool>(cols, false));

    // Mark the nodes on the path from the highlighted leaf up to the root.
    vector<bool> onPath(total, false);
    for (int n = highlightLeaf; n != -1; n = nodes[n].parent) onPath[n] = true;

    for (int n = leafCount; n < total; n++) {
        int x = col[n];
        int children[2] = {nodes[n].left, nodes[n].right};

        for (int k = 0; k < 2; k++) {
            int child = children[k];
            // Horizontal line from the child to this group.
            for (int c = col[child]; c < x; c++) {
                if (grid[row[child]][c] == ' ') grid[row[child]][c] = '-';
                if (onPath[child]) highlighted[row[child]][c] = true;
            }
            grid[row[child]][x] = '+';
            if (onPath[child]) {
                int from = min(row[child], row[n]);
                int to = max(row[child], row[n]);
                for (int y = from; y <= to; y++) highlighted[y][x] = true;
            }
        }

        // Vertical line joining the two children.
        int top = min(row[nodes[n].left], row[nodes[n].right]);
        int bottom = max(row[nodes[n].left], row[nodes[n].right]);
        for (int y = top + 1; y < bottom; y++) {
            if (grid[y][x] == ' ') grid[y][x] = '|';
        }
    }

    // Where a group's own line leaves to the right, show a junction.
    for (int n = leafCount; n < total; n++) {
        grid[row[n]][col[n]] = '+';
    }
    // Short line coming out of the root.
    for (int c = col[root] + 1; c < cols; c++) {
        grid[row[root]][c] = '-';
        highlighted[row[root]][c] = onPath[root];
    }

    // Width of the label column.
    int labelWidth = 0;
    for (int i = 0; i < leafCount; i++) {
        labelWidth = max(labelWidth, (int)nodes[i].label.size());
    }

    // Build the output lines: label + tree.
    for (int y = 0; y < rows; y++) {
        string line;
        int leafHere = -1;
        for (int i = 0; i < leafCount; i++) {
            if (row[i] == y) leafHere = i;
        }

        if (leafHere == -1) {
            line = string(labelWidth, ' ');
        } else if (leafHere == highlightLeaf) {
            line = string(labelWidth - nodes[leafHere].label.size(), ' ') + color(nodes[leafHere].label, PINK);
        } else {
            line = padLeft(nodes[leafHere].label, labelWidth);
        }
        line += " ";

        for (int x = 0; x < cols; x++) {
            string ch(1, grid[y][x]);
            if (grid[y][x] == ' ') line += ch;
            else if (highlighted[y][x]) line += color(ch, PINK);
            else line += color(ch, CYAN);
        }
        lines.push_back(line);
    }

    // Distance scale under the tree.
    string scale(cols, ' ');
    string low = "0.00";
    string middle = formatNumber(treeHeight / 2, 2);
    string high = formatNumber(treeHeight, 2);
    scale.replace(0, low.size(), low);
    if (col[root] > 16) scale.replace(col[root] / 2 - 2, middle.size(), middle);
    scale.replace(max(0, col[root] - 2), high.size(), high);
    lines.push_back(string(labelWidth + 1, ' ') + color(scale, GREY));
    lines.push_back(string(labelWidth + 1, ' ') + color("joining distance (smaller = more similar)", GREY));
    return lines;
}

// ======================================================================
// CareerApp
// ======================================================================

// CareerApp.cpp
// All the screens of the program. Every screen is one function.
// In every list, entering 0 goes back to the previous screen.

// Five small blocks showing the growth outlook, e.g. "####."
static string growthBlocks(int growth) {
    string code = growth >= 4 ? GREEN : (growth == 3 ? CYAN : YELLOW);
    return color(string(growth, '#'), code) + color(string(5 - growth, '.'), GREY);
}

// ============================================================
//  Start-up and start menu
// ============================================================

CareerApp::CareerApp(const string& folder) : auth(folder + "/users.db") {
    dataFolder = folder;
    signedIn = false;
    assessments = NULL;
    roadmaps = NULL;
    careers.load(dataFolder + "/careers.csv");
    loadSettings();
}

CareerApp::~CareerApp() {
    closeUserFiles();
}

int CareerApp::run() {
    if (careers.getAll().empty()) {
        showError("No careers found in " + dataFolder + "/careers.csv");
        return 1;
    }

    while (true) {
        clearScreen();
        printBanner();
        cout << "  " << color(toText(careers.getAll().size()) + " careers in " +
                              toText(careers.getDomains().size()) + " fields  |  " +
                              toText(auth.userCount()) + " registered users", GREY) << "\n";

        vector<MenuOption> menu;
        menu.push_back(makeOption(1, "Sign in", ""));
        menu.push_back(makeOption(2, "Create an account", "takes under a minute"));
        menu.push_back(makeOption(3, "How it works", "the method behind the results"));
        menu.push_back(makeOption(0, "Exit", ""));
        int choice = showMenu(menu);

        if (choice == 1) {
            if (signIn()) dashboard();
        } else if (choice == 2) {
            createAccount();
        } else if (choice == 3) {
            howItWorks();
        } else {
            clearScreen();
            cout << "  Goodbye.\n";
            return 0;
        }
    }
}

bool CareerApp::signIn() {
    printTitle("Sign in", "Home / Sign in");
    string username = toLowerText(askText("Username"));
    if (username.empty()) return false;

    if (auth.secondsLocked(username) > 0) {
        showError("This account is locked. Try again in " + toText(auth.secondsLocked(username)) + " seconds.");
        waitForEnter();
        return false;
    }

    string password = askPassword("Password");
    string message;
    User user;
    if (auth.login(username, password, user, message)) {
        currentUser = user;
        signedIn = true;
        openUserFiles();
        return true;
    }
    showError(message);
    waitForEnter();
    return false;
}

void CareerApp::createAccount() {
    printTitle("Create an account", "Home / Register");
    vector<string> rules;
    rules.push_back("Username : 3-20 characters, lowercase letters, digits or underscore");
    rules.push_back("Password : at least 8 characters, with two kinds (e.g. letters and digits)");
    rules.push_back("Leave the username empty to cancel.");
    printBox("Rules", rules);
    cout << "\n";

    string username;
    while (true) {
        username = toLowerText(askText("Choose a username"));
        if (username.empty()) return;
        if (AuthManager::isValidUsername(username)) break;
        showError("That username does not follow the rules above.");
    }

    // '|' separates fields in our files, so we do not allow it in names.
    string fullName = askText("Full name");
    for (int i = 0; i < (int)fullName.size(); i++) {
        if (fullName[i] == '|') fullName[i] = '/';
    }

    string password;
    while (true) {
        password = askPassword("Choose a password");
        int strength = AuthManager::passwordStrength(password);
        string code = strength >= 3 ? GREEN : (strength == 2 ? CYAN : RED);
        cout << "    Strength  " << colorBar(strength / 4.0, 20, code) << "  " << AuthManager::strengthName(strength) << "\n";

        if (password.size() < 8 || strength < 2) {
            showError("Too weak. Use at least 8 characters with two kinds of characters.");
            continue;
        }
        if (askPassword("Confirm password") != password) {
            showError("The passwords do not match. Please try again.");
            continue;
        }
        break;
    }

    string error = auth.registerUser(username, fullName, password);
    if (error.empty()) showSuccess("Account created. You can now sign in as '" + username + "'.");
    else showError(error);
    waitForEnter();
}

void CareerApp::howItWorks() {
    printTitle("How it works", "Home / Method");
    string titles[5] = {"1. Same 12 numbers for you and every career",
                        "2. A checked profile",
                        "3. Explained matches",
                        "4. The career dendrogram",
                        "5. Plan and track"};
    string texts[5] = {
        "You and every career are described by 12 scores from 0 to 1: six interests (the Holland "
        "RIASEC model) and six abilities.",
        "Self-ratings can be too high or too low. Four short test questions check your maths and "
        "programming ability. The test result is mixed with your self-rating, and the difference is shown.",
        "Fit = 0.65 x shape + 0.35 x level. Shape: does your pattern of strengths match the career's "
        "(correlation)? Level: how close are the actual scores? Each match lists what helps and what to improve.",
        "Careers and your profile are grouped step by step, always joining the two closest groups "
        "(hierarchical clustering). The tree shows which careers are similar and where you fit.",
        "The what-if simulator shows how improving one skill changes your ranking. The roadmap tracker "
        "saves your progress on the steps for a chosen career."};

    for (int i = 0; i < 5; i++) {
        cout << "  " << color(titles[i], BOLD) << "\n";
        vector<string> lines = wrapText(texts[i], SCREEN_WIDTH - 6);
        for (int j = 0; j < (int)lines.size(); j++) cout << "    " << lines[j] << "\n";
        cout << "\n";
    }
    cout << "  " << color("Salary and growth figures are rough guides for India, not exact numbers.", GREY) << "\n";
    waitForEnter();
}

// ============================================================
//  Files for the signed-in user, and settings
// ============================================================

void CareerApp::openUserFiles() {
    string folder = dataFolder + "/profiles/" + currentUser.username;
    assessments = new FileStore<AssessmentResult>(folder + "/assessments.db", "assessments - date|12 trait scores|confidence gap");
    roadmaps = new FileStore<RoadmapProgress>(folder + "/roadmap.db", "roadmaps - career id|milestones done (1) or not (0)");
    assessments->load();
    roadmaps->load();
}

void CareerApp::closeUserFiles() {
    delete assessments;
    delete roadmaps;
    assessments = NULL;
    roadmaps = NULL;
    signedIn = false;
}

void CareerApp::loadSettings() {
    ifstream input((dataFolder + "/settings.cfg").c_str());
    string line;
    while (getline(input, line)) {
        if (trimText(line) == "color=0") useColor = false;
        if (trimText(line) == "color=1") useColor = true;
    }
}

void CareerApp::saveSettings() {
    makeFolder(dataFolder);
    ofstream output((dataFolder + "/settings.cfg").c_str());
    output << "color=" << (useColor ? 1 : 0) << "\n";
}

bool CareerApp::hasProfile() {
    if (assessments != NULL && assessments->count() > 0) return true;
    showWarning("Please take the career assessment first (option 1 on the dashboard).");
    waitForEnter();
    return false;
}

// Assessments are saved in time order, so the last one is the newest.
AssessmentResult CareerApp::latestProfile() {
    return assessments->getAll().back();
}

// ============================================================
//  Dashboard
// ============================================================

void CareerApp::dashboard() {
    while (signedIn) {
        printTitle("Dashboard", "Signed in as " + currentUser.username);

        vector<string> info;
        info.push_back(currentUser.fullName + "   (member since " + currentUser.createdAt.substr(0, 10) + ")");
        info.push_back("");
        if (assessments->count() > 0) {
            AssessmentResult profile = latestProfile();
            vector<Match> ranked = rankCareers(profile.traits, careers);

            int done = 0, steps = 0;
            vector<RoadmapProgress>& list = roadmaps->getAll();
            for (int i = 0; i < (int)list.size(); i++) {
                done += list[i].countDone();
                steps += (int)list[i].done.size();
            }

            info.push_back(padRight("Latest profile", 18) + profile.date.substr(0, 16) + "  (" + toText(assessments->count()) + " saved)");
            info.push_back(padRight("Holland code", 18) + hollandCode(profile.traits));
            info.push_back(padRight("Top match", 18) + ranked[0].career->name + "  " + makeBar(ranked[0].fit, 12) + " " + percent(ranked[0].fit));
            info.push_back(padRight("Roadmaps", 18) + toText(roadmaps->count()) + " tracked, " +
                           toText(done) + "/" + toText(steps) + " steps done");
        } else {
            info.push_back("No profile yet. Start with the career assessment (about 5 minutes).");
            info.push_back("It unlocks recommendations, the dendrogram and the simulator.");
        }
        printBox("Overview", info);

        vector<MenuOption> menu;
        menu.push_back(makeOption(1, "Take career assessment", "28 questions, about 5 minutes"));
        menu.push_back(makeOption(2, "Recommendations", "ranked and explained matches"));
        menu.push_back(makeOption(3, "Career dendrogram", "see where you fit among careers"));
        menu.push_back(makeOption(4, "Explore careers", "browse or search all careers"));
        menu.push_back(makeOption(5, "Compare two careers", "side by side"));
        menu.push_back(makeOption(6, "What-if simulator", "see how skill changes affect fit"));
        menu.push_back(makeOption(7, "Roadmap tracker", "steps for chosen careers"));
        menu.push_back(makeOption(8, "Assessment history", "how your profile has changed"));
        menu.push_back(makeOption(9, "Export report", "save a text summary"));
        menu.push_back(makeOption(10, "Settings", "colours on or off"));
        menu.push_back(makeOption(11, "Change password", ""));
        menu.push_back(makeOption(0, "Sign out", ""));
        int choice = showMenu(menu);

        switch (choice) {
            case 1: takeAssessment(); break;
            case 2: showRecommendations(); break;
            case 3: showDendrogram(); break;
            case 4: exploreCareers(); break;
            case 5: compareCareers(); break;
            case 6: whatIfSimulator(); break;
            case 7: roadmapTracker(); break;
            case 8: showHistory(); break;
            case 9: exportReport(); break;
            case 10: settings(); break;
            case 11: changePassword(); break;
            case 0: closeUserFiles(); break;
        }
    }
}

// ============================================================
//  Assessment
// ============================================================

// Returns 12 lines like "Realistic     ##########.....  58%".
vector<string> CareerApp::profileLines(const vector<double>& traits) {
    vector<string> lines;
    lines.push_back(color("Interests (RIASEC)", BOLD));
    for (int t = 0; t < TRAIT_COUNT; t++) {
        if (t == INTEREST_COUNT) {
            lines.push_back("");
            lines.push_back(color("Abilities", BOLD));
        }
        string code = traits[t] >= 0.7 ? GREEN : (traits[t] >= 0.4 ? CYAN : YELLOW);
        lines.push_back("  " + padRight(TRAIT_NAMES[t], 16) + colorBar(traits[t], 36, code) + " " + padLeft(percent(traits[t]), 4));
    }
    return lines;
}

string CareerApp::confidenceMessage(double gap) {
    int points = (int)(fabs(gap) * 100 + 0.5);
    if (gap > 0.15) return "You rated yourself about " + toText(points) + " points higher than your test result.";
    if (gap < -0.15) return "You did about " + toText(points) + " points better on the test than you rated yourself.";
    return "Your self-rating matches your test result well.";
}

void CareerApp::takeAssessment() {
    printTitle("Career Assessment", "Dashboard / Assessment");
    vector<string> intro;
    intro.push_back("Part 1  Interests    18 statements - how much would you enjoy each?");
    intro.push_back("Part 2  Abilities     6 self-ratings of your current skills");
    intro.push_back("Part 3  Quick check   4 short test questions");
    intro.push_back("");
    intro.push_back("Answer for who you are now. Press 'p' to go back, 'q' to quit.");
    printBox("Before you start", intro);
    if (!askYesNo("Start the assessment now?")) return;

    AssessmentResult result;
    int quizScore = 0, quizTotal = 0;
    if (!runAssessment(result.traits, result.confidenceGap, quizScore, quizTotal)) {
        showInfo("Assessment cancelled. Nothing was saved.");
        waitForEnter();
        return;
    }
    result.date = currentTime();
    assessments->addOrUpdate(result);
    bool saved = assessments->save();

    printTitle("Your Profile", "Dashboard / Assessment / Result");
    cout << "  Holland code  " << color(hollandCode(result.traits), PINK) << color("   (your three strongest interests)", GREY) << "\n";
    cout << "  Test score    " << quizScore << " / " << quizTotal << "\n\n";
    vector<string> lines = profileLines(result.traits);
    for (int i = 0; i < (int)lines.size(); i++) cout << "  " << lines[i] << "\n";
    cout << "\n";
    if (fabs(result.confidenceGap) > 0.15) showWarning(confidenceMessage(result.confidenceGap));
    else showSuccess(confidenceMessage(result.confidenceGap));

    vector<Match> ranked = rankCareers(result.traits, careers);
    printHeading("Your top three matches");
    for (int i = 0; i < 3 && i < (int)ranked.size(); i++) {
        cout << "    " << (i + 1) << ". " << padRight(ranked[i].career->name, 34)
             << colorBar(ranked[i].fit, 16, GREEN) << " " << percent(ranked[i].fit) << "\n";
    }
    cout << "\n";
    if (saved) showSuccess("Profile saved. See Recommendations or the Dendrogram next.");
    else showError("Could not save the profile.");
    waitForEnter();
}

// ============================================================
//  Recommendations and career details
// ============================================================

void CareerApp::showRecommendations() {
    if (!hasProfile()) return;
    AssessmentResult profile = latestProfile();
    vector<Match> ranked = rankCareers(profile.traits, careers);
    int shown = 12;
    if ((int)ranked.size() < shown) shown = (int)ranked.size();

    while (true) {
        printTitle("Recommendations", "Dashboard / Recommendations");
        cout << "  " << color("Profile of " + profile.date.substr(0, 16) + "  |  Holland code " + hollandCode(profile.traits), GREY) << "\n\n";
        cout << color("   #  " + padRight("Career", 31) + padRight("Fit", 19) + padRight("Salary (LPA)", 14) + "Growth", BOLD) << "\n";
        cout << "  " << color(string(SCREEN_WIDTH - 4, '-'), GREY) << "\n";

        for (int i = 0; i < shown; i++) {
            Career* c = ranked[i].career;
            string salary = c->salaryMin == 0 ? "variable" : toText(c->salaryMin) + "-" + toText(c->salaryMax);
            cout << padLeft(toText(i + 1), 4) << "  " << padRight(cutText(c->name, 29), 31)
                 << colorBar(ranked[i].fit, 12, i < 3 ? GREEN : CYAN) << " " << padLeft(percent(ranked[i].fit), 4) << "  "
                 << padRight(salary, 14) << growthBlocks(c->growth) << "\n";
        }
        cout << "\n  " << color("Enter a number to see why it fits and what to improve. 0 = back.", GREY) << "\n";
        int pick = askNumber("Career", 0, shown);
        if (pick == 0) return;
        showCareer(*ranked[pick - 1].career);
    }
}

void CareerApp::showCareer(Career& career) {
    printTitle(career.name, "Career details");
    vector<string> facts = wrapText(career.summary, SCREEN_WIDTH - 6);
    facts.push_back("");
    facts.push_back(padRight("Field", 14) + career.domain);
    facts.push_back(padRight("Salary", 14) + career.salaryText() + "  (rough guide)");
    facts.push_back(padRight("Growth", 14) + career.growthText());
    facts.push_back(padRight("Entry routes", 14) + career.exams);
    printBox("Overview", facts);

    if (assessments->count() > 0) {
        AssessmentResult profile = latestProfile();
        Match m = scoreCareer(profile.traits, career);

        // Find this career's rank.
        vector<Match> ranked = rankCareers(profile.traits, careers);
        int rank = 1;
        while (rank < (int)ranked.size() && ranked[rank - 1].career->id != career.id) rank++;

        printHeading("How you fit");
        cout << "    Overall fit    " << colorBar(m.fit, 24, GREEN) << " " << percent(m.fit)
             << color("   rank " + toText(rank) + " of " + toText(ranked.size()), GREY) << "\n";
        cout << "    Shape          " << colorBar(m.shape, 24, CYAN) << " " << percent(m.shape)
             << color("   same pattern of strengths?", GREY) << "\n";
        cout << "    Level          " << colorBar(m.level, 24, CYAN) << " " << percent(m.level)
             << color("   scores at the same level?", GREY) << "\n";

        if (!m.strengths.empty()) {
            string list;
            for (int i = 0; i < (int)m.strengths.size(); i++) {
                if (i > 0) list += ", ";
                list += TRAIT_NAMES[m.strengths[i]];
            }
            cout << "\n    " << color("Your strengths it uses: ", GREEN) << list << "\n";
        }

        printHeading("You compared with what the career needs");
        for (int t = 0; t < TRAIT_COUNT; t++) {
            double you = profile.traits[t];
            double need = career.traits[t];
            double difference = you - need;
            string code = difference >= -0.05 ? GREEN : (difference >= -GAP_LIMIT ? CYAN : YELLOW);
            cout << "    " << padRight(TRAIT_NAMES[t], 15) << " you " << colorBar(you, 14, code)
                 << "  need " << colorBar(need, 14, GREY) << "  " << color(padLeft(signedPoints(difference), 4), code) << "\n";
        }

        // Which single ability, if raised by 20 points, improves the fit the most?
        int bestTrait = -1;
        double bestGain = 0;
        for (int t = INTEREST_COUNT; t < TRAIT_COUNT; t++) {
            vector<double> improved = profile.traits;
            improved[t] = limitValue(improved[t] + 0.2, 0.0, 1.0);
            double gain = scoreCareer(improved, career).fit - m.fit;
            if (gain > bestGain) {
                bestGain = gain;
                bestTrait = t;
            }
        }
        cout << "\n";
        if (bestTrait != -1 && bestGain > 0.001) {
            showInfo("Best skill to improve: " + TRAIT_NAMES[bestTrait] + " (+20 points gives +" +
                     formatNumber(bestGain * 100, 1) + " fit)");
        }
    } else {
        showInfo("Take the assessment to see how well you fit this career.");
    }

    printHeading("Roadmap");
    for (int i = 0; i < (int)career.milestones.size(); i++) {
        cout << "    " << color(toText(i + 1) + ".", CYAN) << " " << career.milestones[i] << "\n";
    }
    cout << "\n";

    if (roadmaps->find(career.id) != NULL) {
        showInfo("You are already tracking this roadmap (see Roadmap tracker).");
        waitForEnter();
    } else if (askYesNo("Add this roadmap to your tracker?")) {
        startTracking(career);
        showSuccess("Added to your roadmap tracker.");
        waitForEnter();
    }
}

void CareerApp::startTracking(Career& career) {
    RoadmapProgress progress;
    progress.careerId = career.id;
    progress.done = string(career.milestones.size(), '0');
    roadmaps->addOrUpdate(progress);
    roadmaps->save();
}

// ============================================================
//  Dendrogram
// ============================================================

vector<string> CareerApp::dendrogramLines(const vector<double>& traits, bool allCareers) {
    vector<Match> ranked = rankCareers(traits, careers);
    if (!allCareers && ranked.size() > 15) ranked.resize(15);

    // Leaf 0 is the student; leaf k (k >= 1) is the career ranked k.
    vector<string> labels;
    vector<vector<double> > points;
    labels.push_back("YOUR PROFILE");
    points.push_back(traits);
    for (int i = 0; i < (int)ranked.size(); i++) {
        labels.push_back(cutText(ranked[i].career->name, 26));
        points.push_back(ranked[i].career->traits);
    }

    Dendrogram tree;
    tree.build(labels, points);
    int labelWidth = 0;
    for (int i = 0; i < (int)labels.size(); i++) labelWidth = max(labelWidth, (int)labels[i].size());

    vector<string> lines = tree.draw(0, SCREEN_WIDTH - labelWidth - 6);

    // The smallest group that contains you and at least three careers.
    vector<int> family = tree.familyOf(0, 4);
    lines.push_back("");
    lines.push_back(color("Your nearest career family", BOLD));
    for (int leaf = 1; leaf <= (int)ranked.size(); leaf++) {       // in rank order
        bool inFamily = false;
        for (int j = 0; j < (int)family.size(); j++) {
            if (family[j] == leaf) inFamily = true;
        }
        if (inFamily) {
            Match& m = ranked[leaf - 1];
            lines.push_back("  " + padRight(m.career->name, 34) + colorBar(m.fit, 14, GREEN) + " " + percent(m.fit));
        }
    }

    // Cut the tree at 55% of its height to get separate career families.
    vector<vector<int> > groups = tree.cutTree(0.55);
    lines.push_back("");
    lines.push_back(color("Career families (tree cut at 55% of its height)", BOLD));
    for (int g = 0; g < (int)groups.size(); g++) {
        string members;
        bool hasYou = false;
        for (int j = 0; j < (int)groups[g].size(); j++) {
            int leaf = groups[g][j];
            if (leaf == 0) {
                hasYou = true;
                continue;
            }
            if (!members.empty()) members += ", ";
            members += ranked[leaf - 1].career->name;
        }
        if (members.empty()) members = "(no careers - your profile stands apart at this level)";

        vector<string> wrapped = wrapText(members, SCREEN_WIDTH - 16);
        for (int k = 0; k < (int)wrapped.size(); k++) {
            string head = (k == 0) ? "Family " + string(1, (char)('A' + g)) : "";
            string text = (hasYou && k == 0) ? color(wrapped[k], PINK) : wrapped[k];
            lines.push_back("  " + padRight(head, 10) + text);
        }
    }
    return lines;
}

void CareerApp::showDendrogram() {
    if (!hasProfile()) return;
    printTitle("Career Dendrogram", "Dashboard / Dendrogram");
    vector<string> help;
    help.push_back("Each line on the left is a career. Your profile is highlighted.");
    help.push_back("Branches join where careers are similar: the further left two items");
    help.push_back("join, the more alike they are. The coloured path traces your profile.");
    printBox("How to read the tree", help);

    vector<MenuOption> menu;
    menu.push_back(makeOption(1, "Top 15 matches", "shorter tree"));
    menu.push_back(makeOption(2, "All careers", "full tree"));
    menu.push_back(makeOption(0, "Back", ""));
    int choice = showMenu(menu);
    if (choice == 0) return;

    printTitle("Career Dendrogram", choice == 1 ? "Top 15 matches" : "All careers");
    vector<string> lines = dendrogramLines(latestProfile().traits, choice == 2);
    for (int i = 0; i < (int)lines.size(); i++) cout << "  " << lines[i] << "\n";
    waitForEnter();
}

// ============================================================
//  Explore, compare, simulate
// ============================================================

// Shows all careers in two columns and returns the chosen one (or NULL).
Career* CareerApp::chooseCareer(const string& title) {
    printTitle(title, "Choose a career");
    vector<Career>& all = careers.getAll();
    int half = ((int)all.size() + 1) / 2;
    for (int i = 0; i < half; i++) {
        cout << " " << color(padLeft("[" + toText(i + 1) + "]", 5), CYAN) << " " << padRight(cutText(all[i].name, 30), 31);
        int j = i + half;
        if (j < (int)all.size()) {
            cout << " " << color(padLeft("[" + toText(j + 1) + "]", 5), CYAN) << " " << cutText(all[j].name, 30);
        }
        cout << "\n";
    }
    cout << "\n";
    int number = askNumber("Career number (0 = cancel)", 0, (int)all.size());
    if (number == 0) return NULL;
    return &all[number - 1];
}

void CareerApp::exploreCareers() {
    while (true) {
        printTitle("Explore Careers", "Dashboard / Explore");
        vector<MenuOption> menu;
        menu.push_back(makeOption(1, "Browse by field", ""));
        menu.push_back(makeOption(2, "Search by keyword", "name, field, exam or summary"));
        menu.push_back(makeOption(3, "Show all careers", ""));
        menu.push_back(makeOption(0, "Back", ""));
        int choice = showMenu(menu);
        if (choice == 0) return;

        vector<Career*> results;
        string heading;
        if (choice == 1) {
            vector<string> fields = careers.getDomains();
            printTitle("Explore Careers", "Explore / Fields");
            vector<MenuOption> fieldMenu;
            for (int i = 0; i < (int)fields.size(); i++) {
                fieldMenu.push_back(makeOption(i + 1, fields[i], ""));
            }
            fieldMenu.push_back(makeOption(0, "Back", ""));
            int f = showMenu(fieldMenu);
            if (f == 0) continue;
            heading = fields[f - 1];
            vector<Career>& all = careers.getAll();
            for (int i = 0; i < (int)all.size(); i++) {
                if (all[i].domain == heading) results.push_back(&all[i]);
            }
        } else if (choice == 2) {
            string word = askText("Keyword (e.g. GATE, design, finance)");
            results = careers.search(word);
            heading = "Results for '" + word + "'";
        } else {
            results = careers.search("");
            heading = "All careers";
        }

        if (results.empty()) {
            showWarning("No careers matched.");
            waitForEnter();
            continue;
        }

        while (true) {
            printTitle("Explore Careers", heading);
            for (int i = 0; i < (int)results.size(); i++) {
                cout << padLeft("[" + toText(i + 1) + "]", 6) << "  " << padRight(cutText(results[i]->name, 32), 34)
                     << padRight(results[i]->domain, 16);
                if (assessments->count() > 0) {
                    cout << color("fit " + percent(scoreCareer(latestProfile().traits, *results[i]).fit), GREY);
                }
                cout << "\n";
            }
            cout << "\n";
            int number = askNumber("Open career (0 = back)", 0, (int)results.size());
            if (number == 0) break;
            showCareer(*results[number - 1]);
        }
    }
}

void CareerApp::compareCareers() {
    Career* first = chooseCareer("Compare - first career");
    if (first == NULL) return;
    Career* second = chooseCareer("Compare - second career");
    if (second == NULL) return;

    bool haveProfile = assessments->count() > 0;
    printTitle("Comparison", "Dashboard / Compare");
    cout << "  " << padRight("", 16) << color(padRight(cutText(first->name, 28), 30), CYAN)
         << color(cutText(second->name, 28), PINK) << "\n";
    cout << "  " << color(string(SCREEN_WIDTH - 4, '-'), GREY) << "\n";

    for (int t = 0; t < TRAIT_COUNT; t++) {
        if (t == INTEREST_COUNT) cout << "\n";
        cout << "  " << padRight(TRAIT_NAMES[t], 16) << colorBar(first->traits[t], 20, CYAN) << "          "
             << colorBar(second->traits[t], 20, PINK);
        if (haveProfile) cout << color("  you " + percent(latestProfile().traits[t]), GREY);
        cout << "\n";
    }
    cout << "\n";
    cout << "  " << padRight("Salary", 16) << padRight(first->salaryText(), 30) << second->salaryText() << "\n";
    cout << "  " << padRight("Growth", 16) << padRight(first->growthText(), 30) << second->growthText() << "\n";
    cout << "  " << padRight("Field", 16) << padRight(first->domain, 30) << second->domain << "\n";
    if (haveProfile) {
        Match a = scoreCareer(latestProfile().traits, *first);
        Match b = scoreCareer(latestProfile().traits, *second);
        cout << "  " << padRight("Your fit", 16) << padRight(percent(a.fit), 30) << percent(b.fit) << "\n";
        cout << "  " << padRight("Skills to improve", 16) << padRight(toText(a.gapTraits.size()), 30)
             << b.gapTraits.size() << "\n";
    }
    double similar = (correlation(first->traits, second->traits) + 1) / 2;
    cout << "\n  " << color("How similar the two careers are: ", GREY) << percent(similar) << "\n";
    waitForEnter();
}

void CareerApp::whatIfSimulator() {
    if (!hasProfile()) return;
    vector<double> original = latestProfile().traits;
    vector<double> changed = original;

    while (true) {
        vector<Match> before = rankCareers(original, careers);
        vector<Match> after = rankCareers(changed, careers);

        printTitle("What-if Simulator", "Dashboard / Simulator");
        cout << "  " << color("Change a score to see how your ranking moves. Your saved profile is safe.", GREY) << "\n\n";

        for (int t = 0; t < TRAIT_COUNT; t++) {
            double change = changed[t] - original[t];
            bool isChanged = fabs(change) > 0.0001;
            cout << "  " << padLeft("[" + toText(t + 1) + "]", 5) << " " << padRight(TRAIT_NAMES[t], 15)
                 << colorBar(changed[t], 20, isChanged ? PINK : CYAN) << " " << padLeft(percent(changed[t]), 4);
            if (isChanged) cout << "  " << color(signedPoints(change), change > 0 ? GREEN : RED);
            cout << "\n";
        }

        cout << "\n  " << color(padRight("Rank", 6) + padRight("Career", 30) + padRight("Fit", 20) + "Change", BOLD) << "\n";
        for (int i = 0; i < 8 && i < (int)after.size(); i++) {
            // Find the old rank of this career.
            int oldRank = 0;
            for (int j = 0; j < (int)before.size(); j++) {
                if (before[j].career == after[i].career) oldRank = j + 1;
            }
            int moved = oldRank - (i + 1);
            string movement;
            if (moved > 0) movement = color("up " + toText(moved), GREEN);
            else if (moved < 0) movement = color("down " + toText(-moved), RED);
            else movement = color("same", GREY);

            cout << "  " << padRight(toText(i + 1), 6) << padRight(cutText(after[i].career->name, 28), 30)
                 << colorBar(after[i].fit, 12, CYAN) << " " << padLeft(percent(after[i].fit), 4) << "   " << movement << "\n";
        }

        cout << "\n  " << color("Pick a score 1-12 to change, 13 to reset, 0 to leave.", GREY) << "\n";
        int t = askNumber("Score", 0, 13);
        if (t == 0) return;
        if (t == 13) {
            changed = original;
            continue;
        }
        int level = askNumber("New value for " + TRAIT_NAMES[t - 1] + " (0-100)", 0, 100);
        changed[t - 1] = level / 100.0;
    }
}

// ============================================================
//  Roadmaps
// ============================================================

void CareerApp::roadmapTracker() {
    while (true) {
        printTitle("Roadmap Tracker", "Dashboard / Roadmaps");
        vector<RoadmapProgress>& list = roadmaps->getAll();
        vector<Career*> tracked;
        vector<MenuOption> menu;

        for (int i = 0; i < (int)list.size(); i++) {
            Career* c = careers.findById(list[i].careerId);
            if (c == NULL) continue;
            tracked.push_back(c);
            double fraction = list[i].done.empty() ? 0 : (double)list[i].countDone() / list[i].done.size();
            string hint = makeBar(fraction, 16) + " " + toText(list[i].countDone()) + "/" + toText(list[i].done.size());
            menu.push_back(makeOption((int)tracked.size(), cutText(c->name, 28), hint));
        }
        if (tracked.empty()) showInfo("You are not tracking any roadmaps yet.");

        int addOption = (int)tracked.size() + 1;
        menu.push_back(makeOption(addOption, "Track another career", ""));
        menu.push_back(makeOption(0, "Back", ""));
        int choice = showMenu(menu);

        if (choice == 0) return;
        if (choice == addOption) {
            Career* c = chooseCareer("Track a career");
            if (c != NULL) {
                if (roadmaps->find(c->id) == NULL) startTracking(*c);
                roadmapDetail(*c);
            }
        } else {
            roadmapDetail(*tracked[choice - 1]);
        }
    }
}

void CareerApp::roadmapDetail(Career& career) {
    while (true) {
        RoadmapProgress* progress = roadmaps->find(career.id);
        if (progress == NULL) return;
        progress->done.resize(career.milestones.size(), '0');

        printTitle(career.name, "Roadmaps / Steps");
        int steps = (int)career.milestones.size();
        double fraction = steps == 0 ? 0 : (double)progress->countDone() / steps;
        cout << "  Progress  " << colorBar(fraction, 40, fraction >= 1 ? GREEN : CYAN) << "  " << percent(fraction) << "\n\n";

        int nextStep = -1;
        for (int i = 0; i < steps; i++) {
            bool isDone = progress->done[i] == '1';
            if (!isDone && nextStep == -1) nextStep = i;
            cout << "  " << padLeft("[" + toText(i + 1) + "]", 5) << "  "
                 << (isDone ? color("done   ", GREEN) : color("to do  ", GREY))
                 << (isDone ? color(career.milestones[i], GREY) : career.milestones[i]) << "\n";
        }
        cout << "\n";
        if (nextStep != -1) cout << "  " << color("Next step: ", CYAN) << career.milestones[nextStep] << "\n";
        else showSuccess("Roadmap complete. Well done.");

        int stopOption = steps + 1;
        cout << "\n  " << color("Enter a step number to mark done/undone, " + toText(stopOption) +
                                " to stop tracking, 0 = back.", GREY) << "\n";
        int choice = askNumber("Step", 0, stopOption);
        if (choice == 0) return;

        if (choice == stopOption) {
            if (askYesNo("Stop tracking " + career.name + "? Your progress will be removed.")) {
                roadmaps->remove(career.id);
                roadmaps->save();
                return;
            }
            continue;
        }
        char& step = progress->done[choice - 1];
        step = (step == '1') ? '0' : '1';
        roadmaps->save();
    }
}

// ============================================================
//  History, report, settings, password
// ============================================================

void CareerApp::showHistory() {
    printTitle("Assessment History", "Dashboard / History");
    vector<AssessmentResult>& list = assessments->getAll();
    if (list.empty()) {
        showInfo("No assessments saved yet.");
        waitForEnter();
        return;
    }

    cout << color("  " + padRight("Date", 20) + padRight("Code", 7) + padRight("Top match", 32) + "Self-rating", BOLD) << "\n";
    for (int i = 0; i < (int)list.size(); i++) {
        Match top = rankCareers(list[i].traits, careers)[0];
        string check = list[i].confidenceGap > 0.15 ? "too high" : (list[i].confidenceGap < -0.15 ? "too low" : "accurate");
        cout << "  " << padRight(list[i].date.substr(0, 16), 20) << padRight(hollandCode(list[i].traits), 7)
             << padRight(cutText(top.career->name, 26) + " " + percent(top.fit), 32) << color(check, GREY) << "\n";
    }

    if (list.size() >= 2) {
        vector<double>& first = list[0].traits;
        vector<double>& last = list[list.size() - 1].traits;
        printHeading("Change since your first assessment");
        for (int t = 0; t < TRAIT_COUNT; t++) {
            double change = last[t] - first[t];
            string code = change > 0.02 ? GREEN : (change < -0.02 ? RED : GREY);
            cout << "    " << padRight(TRAIT_NAMES[t], 16) << padLeft(percent(first[t]), 5) << "  ->  "
                 << padLeft(percent(last[t]), 5) << "   " << color(padLeft(signedPoints(change), 4), code) << "\n";
        }
    }
    waitForEnter();
}

void CareerApp::exportReport() {
    printTitle("Export Report", "Dashboard / Export");

    // File name like reports/yashas_20261001_140509.txt
    string stamp;
    string now = currentTime();
    for (int i = 0; i < (int)now.size(); i++) {
        if (now[i] == ' ') stamp += '_';
        else if (now[i] != '-' && now[i] != ':') stamp += now[i];
    }
    makeFolder("reports");
    string fileName = "reports/" + currentUser.username + "_" + stamp + ".txt";
    ofstream out(fileName.c_str());
    if (!out) {
        showError("Could not create " + fileName);
        waitForEnter();
        return;
    }

    // Turn colours off while writing, so the file has plain text only.
    bool oldColor = useColor;
    useColor = false;

    string line(SCREEN_WIDTH, '=');
    out << line << "\n  CAREER DENDROGRAM REPORT\n" << line << "\n\n";
    out << "  Name          " << currentUser.fullName << " (" << currentUser.username << ")\n";
    out << "  Created       " << now << "\n";

    if (assessments->count() > 0) {
        AssessmentResult profile = latestProfile();
        out << "  Profile date  " << profile.date << "\n";
        out << "  Holland code  " << hollandCode(profile.traits) << "\n\n";

        vector<string> lines = profileLines(profile.traits);
        for (int i = 0; i < (int)lines.size(); i++) out << "  " << lines[i] << "\n";
        out << "\n  " << confidenceMessage(profile.confidenceGap) << "\n";

        out << "\n  Top 10 matches\n  --------------\n";
        vector<Match> ranked = rankCareers(profile.traits, careers);
        for (int i = 0; i < 10 && i < (int)ranked.size(); i++) {
            out << "  " << padLeft(toText(i + 1), 3) << ". " << padRight(ranked[i].career->name, 34)
                << makeBar(ranked[i].fit, 16) << " " << percent(ranked[i].fit) << "\n";
            if (!ranked[i].gapTraits.empty()) {
                out << "       improve: ";
                for (int k = 0; k < (int)ranked[i].gapTraits.size() && k < 3; k++) {
                    if (k > 0) out << ", ";
                    out << TRAIT_NAMES[ranked[i].gapTraits[k]] << " (" << signedPoints(-ranked[i].gapSizes[k]) << ")";
                }
                out << "\n";
            }
        }

        out << "\n  Career dendrogram (top 15 matches)\n  ----------------------------------\n";
        vector<string> tree = dendrogramLines(profile.traits, false);
        for (int i = 0; i < (int)tree.size(); i++) out << "  " << tree[i] << "\n";
    } else {
        out << "\n  No assessment taken yet.\n";
    }

    out << "\n  Roadmap progress\n  ----------------\n";
    vector<RoadmapProgress>& list = roadmaps->getAll();
    if (list.empty()) out << "  No roadmaps tracked.\n";
    for (int i = 0; i < (int)list.size(); i++) {
        Career* c = careers.findById(list[i].careerId);
        if (c == NULL) continue;
        out << "  " << c->name << "  (" << list[i].countDone() << "/" << list[i].done.size() << ")\n";
        for (int k = 0; k < (int)c->milestones.size() && k < (int)list[i].done.size(); k++) {
            out << "    [" << (list[i].done[k] == '1' ? 'x' : ' ') << "] " << c->milestones[k] << "\n";
        }
    }
    out << "\n" << line << "\n  Salary and growth figures are rough guides. Made with Career Dendrogram System.\n";
    out.close();

    useColor = oldColor;
    showSuccess("Report saved to " + fileName);
    waitForEnter();
}

void CareerApp::settings() {
    while (true) {
        printTitle("Settings", "Dashboard / Settings");
        cout << "  Preview: " << colorBar(0.6, 20, CYAN) << "  " << color("Success", GREEN) << "  "
             << color("Warning", YELLOW) << "  " << color("Error", RED) << "\n";

        vector<MenuOption> menu;
        menu.push_back(makeOption(1, string("Colours: ") + (useColor ? "ON" : "OFF"), "turn off if you see strange symbols"));
        menu.push_back(makeOption(0, "Back", ""));
        int choice = showMenu(menu);
        if (choice == 0) return;
        useColor = !useColor;
        saveSettings();
    }
}

void CareerApp::changePassword() {
    printTitle("Change Password", "Dashboard / Account");
    string oldPassword = askPassword("Current password");
    string newPassword = askPassword("New password");
    int strength = AuthManager::passwordStrength(newPassword);
    cout << "    Strength  " << colorBar(strength / 4.0, 20, strength >= 3 ? GREEN : CYAN) << "  "
         << AuthManager::strengthName(strength) << "\n";

    if (askPassword("Confirm new password") != newPassword) {
        showError("The passwords do not match. Nothing was changed.");
        waitForEnter();
        return;
    }
    string error = auth.changePassword(currentUser.username, oldPassword, newPassword);
    if (error.empty()) showSuccess("Password changed.");
    else showError(error);
    waitForEnter();
}

// ======================================================================
// main
// ======================================================================

// main.cpp
// Career Dendrogram System - program starts here.
//
// Usage:  career_dendrogram [--data <folder>] [--no-color] [--help]

int main(int argc, char* argv[]) {
    string dataFolder = "data";
    bool noColor = false;

    // Read command-line options.
    for (int i = 1; i < argc; i++) {
        string option = argv[i];
        if (option == "--data" && i + 1 < argc) {
            dataFolder = argv[i + 1];
            i++;
        } else if (option == "--no-color") {
            noColor = true;
        } else if (option == "--help") {
            cout << "Career Dendrogram System\n\n";
            cout << "  --data <folder>   folder for data files (default: data)\n";
            cout << "  --no-color        turn off colours\n";
            cout << "  --help            show this message\n";
            return 0;
        } else {
            cout << "Unknown option: " << option << " (try --help)\n";
            return 1;
        }
    }

    setupConsole();
    CareerApp app(dataFolder);
    if (noColor) useColor = false;   // the option wins over the saved setting
    return app.run();
}
