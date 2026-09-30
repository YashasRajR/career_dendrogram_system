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
