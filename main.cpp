#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <sysinfoapi.h>
#elif __linux__
#include <fstream>
#include <sstream>
#endif

using namespace std;
using namespace chrono;

size_t getAvailableRAM()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo))
    {
        return memInfo.ullAvailPhys;
    }
    return 0;
#elif __linux__
    string line;
    ifstream file("/proc/meminfo");
    size_t freeMemory = 0;
    while (getline(file, line))
    {
        if (line.find("MemFree") == 0)
        {
            stringstream ss(line);
            string label;
            ss >> label >> freeMemory;
            break;
        }
    }
    return freeMemory * 1024;
#endif
}

void displaySystemInfo()
{
    #ifdef _WIN32
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        cout << "==============================" << endl;
        cout << "        CPU INFORMATION       " << endl;
        cout << "==============================" << endl;
        system("wmic cpu get Name");
        cout << "Number of Physical Cores: " << sysInfo.dwNumberOfProcessors << endl;
        system("wmic cpu get NumberOfCores,NumberOfLogicalProcessors");
        system("wmic cpu get MaxClockSpeed");
        cout << endl;
    #elif __linux__
        cout << "==============================" << endl;
        cout << "        CPU INFORMATION       " << endl;
        cout << "==============================" << endl;

        // Affichage des informations du CPU sur Linux
        system("lscpu");

        cout << endl;
    #endif

    // RAM
    size_t availableMemory = getAvailableRAM();
    cout << "==============================" << endl;
    cout << "        RAM INFORMATION       " << endl;
    cout << "==============================" << endl;
    cout << "Available RAM: " << fixed << setprecision(2)
         << availableMemory / (1024.0 * 1024 * 1024) << " GB" << endl;
    cout << endl;

    #ifdef _WIN32
        system("wmic memorychip get manufacturer, memorytype, capacity, speed");
    #elif __linux__
        system("sudo dmidecode -t memory");
    #endif

        // Affichage des informations SSD
        cout << "==============================" << endl;
        cout << "        SSD INFORMATION       " << endl;
        cout << "==============================" << endl;
        cout << "SSD Information:" << endl;

    #ifdef _WIN32
        system("wmic diskdrive get caption, mediaType, size"); // Commande Windows pour obtenir les infos sur le disque
    #elif __linux__
        system("lsblk -d -o name,model,size,rota"); // Commande Linux pour obtenir les infos sur le disque
    #endif
}

void cpuBenchmark(milliseconds duration, double &result, int &score)
{
    double localResult = 0.0;
    int operationsCount = 0;
    auto start = high_resolution_clock::now();

    while (high_resolution_clock::now() - start < duration)
    {
        for (int i = 1; i <= 10000; ++i)
        {
            localResult += pow(sin(i), 2.0) + pow(cos(i), 2.0) - log(i + 1) + sqrt(i);
            operationsCount++;  // Count the operations
        }
    }

    result = localResult;

    auto durationSec = duration_cast<seconds>(high_resolution_clock::now() - start).count();
    if (durationSec > 0)
    {
        score = operationsCount / durationSec;
        score /= 100;
    }
    else
    {
        score = 0;
    }
}

// GPU benchmark function (Placeholder)
void gpuBenchmark(milliseconds duration)
{
    cout << "GPU benchmark is not implemented yet." << endl;
}

// RAM benchmark function
void ramBenchmark(milliseconds duration)
{
    const size_t blockSize = 1024 * 1024 * 100; // 100 MB per block
    size_t availableMemory = getAvailableRAM(); // Free RAM detected
    size_t allocatedMemory = 0;
    vector<char*> memoryBlocks;

    cout << "Available RAM: " << fixed << setprecision(1)
         << availableMemory / (1024.0 * 1024 * 1024) << " GB" << endl;
    cout << "Allocating as much as possible..." << endl;

    // Start the timer before any operations
    auto start = high_resolution_clock::now();

    try
    {
        while (allocatedMemory + blockSize <= availableMemory)
        {
            char* block = new char[blockSize]; 
            if (!block) throw bad_alloc();
            memoryBlocks.push_back(block);
            allocatedMemory += blockSize;

            // Avoid CPU overload
            if (high_resolution_clock::now() - start >= duration)
            {
                cout << "Time limit reached. Stopping allocation." << endl;
                break;
            }
        }
    }
    catch (const bad_alloc& e)
    {
        cout << "Memory allocation stopped at: " << fixed << setprecision(1)
             << allocatedMemory / (1024.0 * 1024 * 1024) << " GB" << endl;
    }

    cout << "Allocated: " << fixed << setprecision(1)
         << allocatedMemory / (1024.0 * 1024 * 1024) << " GB" << endl;

    // Stress the memory until the end of the benchmark
    while (high_resolution_clock::now() - start < duration)
    {
        for (auto& block : memoryBlocks)
        {
            for (size_t i = 0; i < blockSize; i += 4096) // Access every 4 KB
            {
                block[i] = static_cast<char>(i % 256);
            }
        }
        this_thread::sleep_for(milliseconds(100)); // Avoid overloading the CPU
    }

    cout << "RAM benchmark completed. Releasing memory..." << endl;

    // Release allocated memory
    for (auto& block : memoryBlocks)
    {
        delete[] block;
    }

    cout << "Memory released successfully." << endl;
}

// SSD benchmark function
void ssdBenchmark(milliseconds duration)
{
    const size_t fileSize = 1024 * 1024 * 500; // 500 MB
    const string filePath = "ssd_benchmark.tmp";
    vector<char> buffer(1024 * 1024, 'X'); // Buffer of 1MB filled with 'X'

    cout << "Starting SSD benchmark..." << endl;

    // Measure write speed
    auto start = high_resolution_clock::now();
    ofstream outFile(filePath, ios::binary);
    if (!outFile)
    {
        cerr << "Error: Unable to create benchmark file!" << endl;
        return;
    }

    size_t written = 0;
    while (written < fileSize && high_resolution_clock::now() - start < duration)
    {
        outFile.write(buffer.data(), buffer.size());
        written += buffer.size();
    }
    outFile.close();
    auto writeTime = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    double writeSpeed = (written / (1024.0 * 1024)) / (writeTime.count() / 1000.0);

    cout << "Write speed: " << fixed << setprecision(2) << writeSpeed << " MB/s" << endl;

    // Measure read speed
    start = high_resolution_clock::now();
    ifstream inFile(filePath, ios::binary);
    if (!inFile)
    {
        cerr << "Error: Unable to read benchmark file!" << endl;
        return;
    }

    size_t read = 0;
    while (read < fileSize && high_resolution_clock::now() - start < duration)
    {
        inFile.read(buffer.data(), buffer.size());
        read += buffer.size();
    }
    inFile.close();
    auto readTime = duration_cast<milliseconds>(high_resolution_clock::now() - start);
    double readSpeed = (read / (1024.0 * 1024)) / (readTime.count() / 1000.0);

    cout << "Read speed: " << fixed << setprecision(2) << readSpeed << " MB/s" << endl;

    // Remove temporary file
    remove(filePath.c_str());
    cout << "Benchmark completed, temporary file deleted." << endl;
}

int main()
{
    int choice;
    int durationInput;

    while (true)
    {
        cout << "==============================" << endl;
        cout << "        BENCHMARK CLI TOOL    " << endl;
        cout << "==============================" << endl;
        cout << "1. Display System Info" << endl;
        cout << "2. CPU Benchmark" << endl;
        cout << "3. GPU Benchmark (not implemented yet)" << endl;
        cout << "4. RAM Benchmark" << endl;
        cout << "5. SSD Benchmark" << endl;
        cout << "6. Exit" << endl;
        cout << "Select an option: ";
        cin >> choice;

        if (choice == 6)
        {
            cout << "Exiting program..." << endl;
            break;
        }

        if (choice == 1)
        {
            displaySystemInfo();
        }
        else
        {
            cout << "Enter the benchmark duration (in seconds): ";
            cin >> durationInput;

            if (durationInput <= 0)
            {
                cout << "Invalid duration. Please restart the program." << endl;
                continue;
            }

            milliseconds testDuration(durationInput * 1000);
            int score = 0;

            if (choice == 2) // CPU Benchmark
            {
                int numThreads = thread::hardware_concurrency();
                if (numThreads == 0) numThreads = 4;

                cout << "Detected number of threads: " << numThreads << endl;
                cout << "Starting CPU benchmark for " << durationInput << " seconds..." << endl;

                vector<thread> threads;
                vector<double> results(numThreads, 0.0);

                for (int i = 0; i < numThreads; ++i)
                {
                    threads.emplace_back(cpuBenchmark, testDuration, ref(results[i]), ref(score));
                }

                for (auto &t : threads)
                {
                    t.join();
                }

                cout << "CPU Benchmark Score: " << score << endl;
            }
            else if (choice == 3) // GPU Benchmark (to be implemented)
            {
                gpuBenchmark(testDuration);
            }
            else if (choice == 4) // RAM Benchmark
            {
                cout << "Starting RAM benchmark for " << durationInput << " seconds..." << endl;
                ramBenchmark(testDuration);
            }
            else if (choice == 5) // SSD Benchmark
            {
                cout << "Starting SSD benchmark for " << durationInput << " seconds..." << endl;
                ssdBenchmark(testDuration);
            }
        }

        cout << "\nPress Enter to return to the main menu...";
        cin.ignore();
        cin.get();
    }

    return 0;
}
