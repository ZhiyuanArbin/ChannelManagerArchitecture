#ifndef BATTERYTESTINGSERVICE_H
#define BATTERYTESTINGSERVICE_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <map>
#include <vector>
#include <atomic>

#include "Task.h"
#include "ChannelService.h"

// Forward declarations
class ChannelCtrlService;
class ChannelDataService;
class Task;

typedef struct
{
    std::string var_type;
    float target_value;
} StepLimit; // Define the StepLimit struct

/**
 * @brief The BatteryTestingService class provides an abstraction layer for controlling battery testing hardware.
 *
 * This class manages tasks, handles asynchronous communication with the M4 core,
 * and provides a callback mechanism for reacting to data changes.
 */
class BatteryTestingService {
public:
    /**
     * @brief Constructor for the BatteryTestingService class.
     *
     * @param numWorkerThreads The initial number of worker threads to create.
     */
    BatteryTestingService(size_t numWorkerThreads = 3);

    /**
     * @brief Destructor for the BatteryTestingService class.
     */
    ~BatteryTestingService();

    /**
     * @brief Runs a Constant Current Constant Voltage (CCCV) test on a channel.
     *
     * @param channel The channel number.
     * @param current The target current value.
     * @param targetVoltage The target voltage value.
     * @param steplimit The step limit for the test.
     */
    void runCCCV(uint32_t channel, float current, float targetVoltage, const std::vector<StepLimit> &steplimit);

    /**
     * @brief Runs a Current Ramp test on a channel.
     *
     * @param channel The channel number.
     * @param current The target current value.
     */
    void runCurrentRamp(uint32_t channel, float current);

    /**
     * @brief Sets the channel to a rest state (open circuit).
     *
     * @param channel The channel number.
     */
    void runRest(uint32_t channel);

    /**
     * @brief Adds or removes worker threads dynamically.
     *
     * @param numThreads The new total number of worker threads.
     */
    void setWorkerThreadCount(size_t numThreads);
    
    /**
     * @brief Gets the current number of worker threads.
     *
     * @return The number of worker threads.
     */
    size_t getWorkerThreadCount() const;

private:
    /**
     * @brief Adds a task to the task queue.
     *
     * @param task The task to add.
     */
    void addTask(Task* task);
    
    /**
     * @brief Registers a callback function for a specific channel.
     *
     * @param channel The channel number.
     * @param callback The callback function to register.
     */
    void registerCallback(uint32_t channel, std::function<void(uint32_t, const std::map<std::string, float>&)> callback);
    
    /**
     * @brief Handles notifications of new data from the data plane.
     * Creates and adds a CallbackControlTask to execute the registered callback.
     *
     * @param channel The channel number with new data.
     */
    void handleCallbacks(uint32_t channel);
    
    /**
     * @brief Unregisters a callback function for a specific channel.
     *
     * @param channel The channel number.
     * @param callbackIndex Optional index of the specific callback to unregister.
     *                     If not provided, all callbacks for the channel will be unregistered.
     */
    void unregisterCallback(uint32_t channel, int callbackIndex = -1);

    // Single task queue for all tasks
    std::priority_queue<Task*, std::vector<Task*>, TaskComparator> taskQueue;

    // Worker threads and M4 data thread
    std::vector<std::thread> workerThreads;
    std::thread m4DataThread;    // Dedicated thread for receiving M4 data
    
    // Flag to signal threads to stop
    std::atomic<bool> stopThreads;

    // Mutex and condition variable for task queue
    std::mutex taskQueueMutex;
    std::condition_variable taskQueueCV;

    // Thread Functions
    void workerThreadFunction();
    void m4DataThreadFunction();

    // Low-Level Services
    ChannelCtrlService* channelCtrlService;
    ChannelDataService* channelDataService;
    
    // Callback map to store multiple callback functions for each channel
    std::map<uint32_t, std::vector<std::function<void(uint32_t, const std::map<std::string, float>&)>>> callbackMap;
};


#endif