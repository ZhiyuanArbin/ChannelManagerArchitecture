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
    using CallbackFunction = std::function<void(uint32_t channel, const std::map<std::string, float>& data)>;
    void registerCallback(uint32_t channel, CallbackFunction callback);

    /**
     * @brief Processes data received from the M4 core.
     *
     * @param channel The channel number.
     * @param data The data received from the M4 core.
     */
    void processM4Data(uint32_t channel, const std::map<std::string, float>& data); // Called when data is received from M4

private:
    // Task Queues
    std::priority_queue<ControlTask*, std::vector<ControlTask*>, TaskComparator> controlTaskQueue;
    std::priority_queue<DataTask*, std::vector<DataTask*>, TaskComparator> dataTaskQueue;

    // Threads
    std::thread controlThread1;
    std::thread controlThread2;
    std::thread dataThread1;
    std::thread dataThread2;

    // Mutexes and Condition Variables
    std::mutex controlQueueMutex;
    std::mutex dataQueueMutex;
    std::condition_variable controlQueueCV;
    std::condition_variable dataQueueCV;

    // Callback Map
    std::map<uint32_t, CallbackFunction> callbackMap;

    // Thread Functions
    void controlThreadFunction();
    void dataThreadFunction();

    // Low-Level Services (Example)
    ChannelCtrlService* channelCtrlService;
    ChannelDataService* channelDataService;
};

#endif