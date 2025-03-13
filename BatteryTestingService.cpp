#include "BatteryTestingService.h"
#include "ChannelService.h"
#include "Task.h"
#include <iostream>

/**
 * @brief Constructor for the BatteryTestingService class.
 *
 * Initializes the worker threads, M4 data reception thread,
 * and the low-level services.
 *
 * @param numWorkerThreads The initial number of worker threads to create.
 */
BatteryTestingService::BatteryTestingService(size_t numWorkerThreads) :
    stopThreads(false),
    m4DataThread(&BatteryTestingService::m4DataThreadFunction, this) {
    
    // Initialize channel control service
    channelCtrlService = new DummyChannelCtrlService();

    // Initialize channel data service
    channelDataService = new DummyChannelDataService();
    
    // Create worker threads
    for (size_t i = 0; i < numWorkerThreads; ++i) {
        workerThreads.emplace_back(&BatteryTestingService::workerThreadFunction, this);
    }
}

/**
 * @brief Destructor for the BatteryTestingService class.
 *
 * Cleans up the threads and low-level services.
 */
BatteryTestingService::~BatteryTestingService() {
    // Signal all threads to stop
    stopThreads = true;
    
    // Notify all worker threads to check the stop flag
    taskQueueCV.notify_all();
    
    // Join all worker threads
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Join m4DataThread
    if (m4DataThread.joinable()) {
        m4DataThread.join();
    }
    
    // Clean up services
    delete channelCtrlService;
    delete channelDataService;
    
    // Clean up any remaining tasks in the queue
    std::lock_guard<std::mutex> lock(taskQueueMutex);
    while (!taskQueue.empty()) {
        Task* task = taskQueue.top();
        taskQueue.pop();
        delete task;
    }
}


/**
 * @brief Adds a task to the task queue.
 *
 * @param task The task to add.
 */
void BatteryTestingService::addTask(Task* task) {
    std::lock_guard<std::mutex> lock(taskQueueMutex);
    taskQueue.push(task);
    taskQueueCV.notify_one();
}

/**
 * @brief Worker thread function that processes tasks from the queue.
 */
void BatteryTestingService::workerThreadFunction() {
    while (!stopThreads) {
        Task* task = nullptr;
        
        {
            std::unique_lock<std::mutex> lock(taskQueueMutex);
            taskQueueCV.wait(lock, [this] {
                return !taskQueue.empty() || stopThreads;
            });
            
            // Check if we should exit
            if (stopThreads && taskQueue.empty()) {
                break;
            }
            
            // Get the next task
            if (!taskQueue.empty()) {
                task = taskQueue.top();
                taskQueue.pop();
            }
        }
        
        // Execute the task if we got one
        if (task) {
            task->execute();
            delete task;
        }
    }
}

/**
 * @brief Dynamically adjusts the number of worker threads.
 *
 * @param numThreads The new total number of worker threads.
 */
void BatteryTestingService::setWorkerThreadCount(size_t numThreads) {
    size_t currentThreadCount = workerThreads.size();
    
    if (numThreads > currentThreadCount) {
        // Add more threads
        for (size_t i = currentThreadCount; i < numThreads; ++i) {
            workerThreads.emplace_back(&BatteryTestingService::workerThreadFunction, this);
        }
    }
    else if (numThreads < currentThreadCount) {
        // Signal threads to stop
        stopThreads = true;
        taskQueueCV.notify_all();
        
        // Wait for threads to finish
        for (auto& thread : workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        // Clear the thread vector
        workerThreads.clear();
        
        // Reset stop flag
        stopThreads = false;
        
        // Create new threads
        for (size_t i = 0; i < numThreads; ++i) {
            workerThreads.emplace_back(&BatteryTestingService::workerThreadFunction, this);
        }
    }
}

/**
 * @brief Gets the current number of worker threads.
 *
 * @return The number of worker threads.
 */
size_t BatteryTestingService::getWorkerThreadCount() const {
    return workerThreads.size();
}

/**
 * @brief Runs a Constant Current Constant Voltage (CCCV) test on a channel.
 *
 * @param channel The channel number.
 * @param current The target current value.
 * @param targetVoltage The target voltage value.
 * @param steplimit The step limit for the test.
 */
void BatteryTestingService::runCCCV(uint32_t channel, float current, float targetVoltage,
    const std::vector<StepLimit>& steplimit) {
    std::cout << "Running CCCV on channel " << channel << ", current: " << current << ", target voltage: " << targetVoltage << std::endl;

    // 1. Subscribe to the channel data
    channelDataService->subscribeChannel(channel);
    
    // 2. Create and add a task to do constant current
    addTask(new CCTask(channel, current, channelCtrlService));

    // 3. Register a callback to check voltage and switch to CV
    registerCallback(channel, [this, channel, targetVoltage](uint32_t ch, const std::map<std::string, float>& data) {
        if (data.count("voltage") > 0 && data.at("voltage") >= targetVoltage) {
            std::cout << "Target voltage reached on channel " << channel << ", switching to CV" << std::endl;

            // Create and add a CV task
            addTask(new CVTask(channel, targetVoltage, channelCtrlService));
            
            // Unregister the callback once we've switched to CV
            unregisterCallback(channel, 0);

            registerCallback(channel, [this, channel](uint32_t ch, const std::map<std::string, float>& data) {
                // Implement CV check logic here
            });
        }
    });

    // 4. Register a callback to check step limits and end the test
    registerCallback(channel, [this, channel, steplimit](uint32_t ch, const std::map<std::string, float>& data) {
        //Implement limit check logic here
        if (isLimitReached(data, steplimit)) {
            std::cout << "Step limit reached on channel " << channel << ", ending test" << std::endl;
            unregisterCallback(channel, -1);
            channelDataService->unsubscribeChannel(channel);
            terminateTest(channel);
        }
    });
}

/**
 * @brief Runs a Current Ramp test on a channel.
 *
 * @param channel The channel number.
 * @param current The target current value.
 */
void BatteryTestingService::runCurrentRamp(uint32_t channel, float current) {
    std::cout << "Running Current Ramp on channel " << channel << ", current: " << current << std::endl;
    // Implementation goes here
}

/**
 * @brief Sets the channel to a rest state (open circuit).
 *
 * @param channel The channel number.
 */
void BatteryTestingService::runRest(uint32_t channel) {
    std::cout << "Running Rest on channel " << channel << std::endl;
    // Implementation would go here
}

/**
 * @brief Registers a callback function for a specific channel.
 *
 * @param channel The channel number.
 * @param callback The callback function to register.
 */
void BatteryTestingService::registerCallback(uint32_t channel, std::function<void(uint32_t, const std::map<std::string, float>&)> callback) {
    std::cout << "Registering callback for channel " << channel << std::endl;
    
    // Create vector for the channel if it doesn't exist
    if (callbackMap.find(channel) == callbackMap.end()) {
        callbackMap[channel] = std::vector<std::function<void(uint32_t, const std::map<std::string, float>&)>>();
    }
    
    // Add the callback to the vector
    callbackMap[channel].push_back(callback);
}

/**
 * @brief Handles notifications of new data from the data plane.
 * Creates and adds a CallbackControlTask to execute the registered callbacks.
 *
 * @param channel The channel number with new data.
 */
void BatteryTestingService::handleCallbacks(uint32_t channel) {
    std::cout << "Handling new data notification for channel " << channel << std::endl;
    
    // Check if there are callbacks registered for this channel
    if (callbackMap.find(channel) != callbackMap.end() && !callbackMap[channel].empty()) {
        // Create a CallbackControlTask for each callback and add it to the task queue
        for (const auto& callback : callbackMap[channel]) {
            CallbackControlTask* task = new CallbackControlTask(channel, callback, channelDataService);
            addTask(task);
        }
    }
}

/**
 * @brief Unregisters a callback function for a specific channel.
 *
 * @param channel The channel number.
 * @param callbackIndex Optional index of the specific callback to unregister.
 *                     If negative, all callbacks for the channel will be unregistered.
 */
void BatteryTestingService::unregisterCallback(uint32_t channel, int callbackIndex) {
    std::cout << "Unregistering callback(s) for channel " << channel << std::endl;
    
    // Check if there are callbacks registered for this channel
    auto it = callbackMap.find(channel);
    if (it != callbackMap.end()) {
        if (callbackIndex < 0) {
            // Remove all callbacks for this channel
            callbackMap.erase(channel);
        } else if (callbackIndex < static_cast<int>(it->second.size())) {
            // Remove the specific callback at the given index
            it->second.erase(it->second.begin() + callbackIndex);
            
            // If no callbacks remain, remove the channel entry
            if (it->second.empty()) {
                callbackMap.erase(channel);
            }
        }
    }
}

/**
 * @brief Thread function for continuously receiving M4 data.
 *
 * This thread continuously receives data from the M4 core and processes it.
 * Worker threads handle data processing (filtering, fitting, etc.) and callbacks.
 */
void BatteryTestingService::m4DataThreadFunction() {
    // Example data for demonstration purposes
    std::map<std::string, float> sampleData[MAX_CHAN_NUM];
    
    while (!stopThreads) {
        // In a real implementation, we would read from the M4 core
        // Something like:
        ReadFromM4("/dev/ttyRPMSG0", &sampleData);
        
        // For simulation purposes, iterate over all channels
        for (uint32_t channel = 0; channel < MAX_CHAN_NUM; channel++) {
            // Create data processing tasks (filtering, fitting, etc.)
            channelDataService->receiveM4Data(channel, sampleData[channel]);
            addTask(new FilteringDataTask(channel, sampleData[channel]));
            addTask(new FittingDataTask(channel, sampleData[channel]));
            
            if (channelDataService->isChannelSubscribed(channel)) {
                // Execute callbacks for subscribed channels
                handleCallbacks(channel);
            }
        }
        
        // Sleep to avoid excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Helper function to check if a step limit has been reached
// This would need to be properly implemented in a real application
bool isLimitReached(const std::map<std::string, float>& data, const std::vector<StepLimit>& limits) {
    // Example implementation
    for (const auto& limit : limits) {
        if (data.count(limit.var_type) > 0 && data.at(limit.var_type) >= limit.target_value) {
            return true;
        }
    }
    return false;
}

// Helper function to terminate a test
// This would need to be properly implemented in a real application
void terminateTest(uint32_t channel) {
    std::cout << "Terminating test on channel " << channel << std::endl;
    // Implementation would go here
}

/**
 * @brief Executes the constant current task.
 */
void CCTask::execute() {
    ctrlService->doConstantCurrent(channel, current);
}

/**
 * @brief Executes the constant voltage task.
 */
void CVTask::execute() {
    ctrlService->doConstantVoltage(channel, targetVoltage);
}

/**
 * @brief Executes the generic control task.
 */
void GenericControlTask::execute() {
    while (!ctrlServices.empty()) {
        ChannelCtrlService* service = ctrlServices.front();
        ctrlServices.pop();
        // Clean up the service after execution
        delete service;
    }
}

/**
 * @brief Adds a ChannelCtrlService to the queue.
 *
 * @param service The ChannelCtrlService to add.
 */
void GenericControlTask::addCtrlService(ChannelCtrlService* service) {
    ctrlServices.push(service);
}

/**
 * @brief Executes the callback control task.
 *
 * Reads the latest data for the channel from the data service and
 * executes the registered callback function.
 */
void CallbackControlTask::execute() {
    // Get the latest data for this channel from the data service
    const std::map<std::string, float>& channelData = dataService->getChannelData(channel);
    
    // Execute the callback with the channel data
    if (callback) {
        callback(channel, channelData);
    }
}
