#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>
#include <pocketsphinx.h>

using namespace std;

// Function to convert string to lowercase
string toLowercase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// Function to handle user queries and provide responses
string handleQuery(const string& query) {
    // Map of commands and their descriptions
    map<string, string> commands = {
        {"dir", "Lists the contents of a directory."},
        {"cd", "Changes the current directory."},
        {"mkdir", "Creates a new directory."},
        {"rmdir", "Removes a directory."},
        {"del", "Deletes one or more files."},
        {"copy", "Copies one or more files to another location."},
        {"move", "Moves one or more files from one location to another."},
        {"ren", "Renames a file or directory."},
        {"type", "Displays the contents of a text file."},
        {"cls", "Clears the Command Prompt window."},
        {"exit", "Exits the Command Prompt."},
        {"ipconfig", "Displays IP configuration information."},
        {"ping", "Sends a network request to another device to test connectivity."},
        {"tracert", "Traces the route taken by packets over an IP network."},
        {"netstat", "Displays active network connections."},
        {"tasklist", "Lists all currently running processes."},
        {"taskkill", "Terminates one or more processes."},
        {"shutdown", "Shuts down or restarts the computer."},
        {"systeminfo", "Displays detailed configuration information about a computer and its operating system."},
        {"sfc /scannow", "Scans and repairs system files."}
    };

    // Convert query to lowercase for case-insensitive matching
    string lowercaseQuery = toLowercase(query);

    // Check if query matches any predefined commands
    auto it = commands.find(lowercaseQuery);
    if (it != commands.end()) {
        return it->second;
    } else {
        return "Command not found. Type 'commands' to see the available commands.";
    }
}

// Function to recognize speech using PocketSphinx
string recognizeSpeech() {
    ps_decoder_t *ps;
    cmd_ln_t *config;
    FILE *fh;
    char const *hyp, *uttid;
    int16 buf[512];
    int rv;
    int32 score;

    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-lm", MODELDIR "/en-us/en-us.lm.bin",
            "-dict", MODELDIR "/en-us/cmudict-en-us.dict",
            NULL);

    if (config == NULL) {
        cmd_ln_free_r(config);
        return "ERROR";
    }

    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return "ERROR";
    }

    fh = fopen("recording.wav", "rb");
    if (fh == NULL) {
        ps_free(ps);
        cmd_ln_free_r(config);
        return "ERROR";
    }

    rv = ps_decode_raw(ps, fh, "recording", -1);
    if (rv < 0) {
        fclose(fh);
        ps_free(ps);
        cmd_ln_free_r(config);
        return "ERROR";
    }

    hyp = ps_get_hyp(ps, &score, &uttid);
    if (hyp == NULL) {
        fclose(fh);
        ps_free(ps);
        cmd_ln_free_r(config);
        return "ERROR";
    }

    fclose(fh);
    ps_free(ps);
    cmd_ln_free_r(config);

    return hyp;
}

int main() {
    cout << "Welcome! This is your always-active assistant. Say 'Windows' to activate voice input." << endl;

    // Main loop to handle user input
    while (true) {
        // Listen for the "windows" trigger
        string userInput;
        do {
            cout << "Listening... (Say 'Windows' to activate)" << endl;
            system("arecord -q -f cd -t wav -d 3 -r 16000 recording.wav");
            userInput = recognizeSpeech();
        } while (toLowercase(userInput) != "windows");

        // Once activated, listen for user commands
        cout << "Assistant activated. How can I assist you?" << endl;
        while (true) {
            cout << "> ";
            system("arecord -q -f cd -t wav -d 3 -r 16000 recording.wav");
            string userInput = recognizeSpeech();

            // Check if user wants to exit
            if (toLowercase(userInput) == "exit") {
                cout << "Goodbye! Have a great day." << endl;
                break;
            }

            // Handle user query
            string response = handleQuery(userInput);
            cout << response << endl;
        }

        // Wait for a moment before reactivating the assistant
        cout << "Reactivating assistant..." << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}
