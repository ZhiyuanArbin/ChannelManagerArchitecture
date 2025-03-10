#ifndef BATTERYTESTINGSERVICE_H
#define BATTERYTESTINGSERVICE_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <map>

#include "Task.h"
#include "ChannelService.h"

// Forward declarations
class ChannelCtrlService;
class ChannelDataService;
class Task;
class ControlTask;
class DataTask;

/**
 * @brief The BatteryTestingService class provides an abstraction layer for controlling battery testing hardware.
 *
 * This class manages control and data tasks, handles asynchronous communication with the M4 core,
 * and provides a callback mechanism for reacting to data changes.
 */
class BatteryTestingService {
public:
    /**
     * @brief Constructor for the BatteryTestingService class.
     */
    BatteryTestingService();

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
     */
    void runCCCV(uint32_t channel, float current, float targetVoltage);

    /**
     * @brief Runs a Direct Current Internal Measurement (DCIM) test on a channel.
     *
     * @param channel The channel number.
     * @param current The target current value.
     */
    void runDCIM(uint32_t channel, float current);

    /**
     * @brief Sets the channel to a rest state (open circuit).
     *
     * @param channel The channel number.
     */
    void runRest(uint32_t channel);

private:
    /**
     * @brief Adds a control task to the control task queue.
     *
     * @param task The control task to add.
     */
    void addControlTask(ControlTask* task);

    /**
     * @brief Adds a data task to the data task queue.
     *
     * @param task The data task to add.
     */
    void addDataTask(DataTask* task);
    
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
    // void unregisterCallback(uint32_t channel);

    // Task Queues
    std::priority_queue<ControlTask*, std::vector<ControlTask*>, TaskComparator> controlTaskQueue;
    std::priority_queue<DataTask*, std::vector<DataTask*>, TaskComparator> dataTaskQueue;

    // Threads
    std::thread controlThread1;
    std::thread controlThread2;
    std::thread dataThread;      // For processing data tasks
    std::thread m4DataThread;    // Dedicated thread for receiving M4 data

    // Mutexes and Condition Variables
    std::mutex controlQueueMutex;
    std::mutex dataQueueMutex;
    std::condition_variable controlQueueCV;
    std::condition_variable dataQueueCV;

    // Thread Functions
    void controlThreadFunction();
    void dataThreadFunction();
    void m4DataThreadFunction(); // New function for continuously receiving M4 data

    // Low-Level Services
    ChannelCtrlService* channelCtrlService;
    ChannelDataService* channelDataService;
    
    // Callback map to store multiple callback functions for each channel
    std::map<uint32_t, std::vector<std::function<void(uint32_t, const std::map<std::string, float>&)>>> callbackMap;
};

#endif