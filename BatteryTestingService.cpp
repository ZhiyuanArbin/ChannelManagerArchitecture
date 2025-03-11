#include "BatteryTestingService.h"
#include "ChannelService.h"
#include "Task.h"
#include <iostream>

/**
 * @brief Constructor for the BatteryTestingService class.
 *
 * Initializes the control threads, data thread, M4 data reception thread,
 * and the low-level services.
 */
BatteryTestingService::BatteryTestingService() :
    controlThread1(&BatteryTestingService::controlThreadFunction, this),
    controlThread2(&BatteryTestingService::controlThreadFunction, this),
    dataThread(&BatteryTestingService::dataThreadFunction, this),
    m4DataThread(&BatteryTestingService::m4DataThreadFunction, this) {
    
    // Initialize channel control service
    channelCtrlService = new DummyChannelCtrlService();

    // Initialize channel data service
    channelDataService = new DummyChannelDataService();
}

/**
 * @brief Destructor for the BatteryTestingService class.
 *
 * Cleans up the threads and low-level services.
 */
BatteryTestingService::~BatteryTestingService() {
    // Clean up threads
    controlThread1.join();
    controlThread2.join();
    dataThread.join();
    m4DataThread.join();

    delete channelCtrlService;
    delete channelDataService;
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
    
    // 2. Create and add a control task to do constant current
    addControlTask(new CCTask(channel, current, channelCtrlService));

    // 3. Register a callback to check voltage and switch to CV
    registerCallback(channel, [this, channel, targetVoltage](uint32_t ch, const std::map<std::string, float>& data) {
        if (data.count("voltage") > 0 && data.at("voltage") >= targetVoltage) {
            std::cout << "Target voltage reached on channel " << channel << ", switching to CV" << std::endl;

            // Create and add a CV task
            addControlTask(new CVTask(channel, targetVoltage, channelCtrlService));
            
            // Unregister the callback once we've switched to CV
            unregisterCallback(channel, 0);

            // register CV callback check
            registerCallback(channel, [this, channel, checkCV_data](uint32_t ch, const std::map<std::string, float>& data) {
                this->checkCV(channel, data, checkCV_data); 
            } );
        }
    });

    registerCallback(channel, [this, channel, steplimit](uint32_t ch, const std::map<std::string, float>& data) {
        if (isLimitReached(data, steplimit)) {
            std::cout << "Step limit reached on channel " << channel << ", ending test" << std::endl;
            unregisterCallback(channel, -1);
            channelDataService->unsubscribeChannel(channel);
            terminateTest(channel);  
        }
    });
}

/**
 * @brief Runs a Direct Current Internal Measurement (DCIM) test on a channel.
 *
 * @param channel The channel number.
 * @param current The target current value.
 */
void BatteryTestingService::runDCIM(uint32_t channel, float current) {
    std::cout << "Running DCIM on channel " << channel << ", current: " << current << std::endl;
    // Implementation would go here
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
 * @brief Adds a control task to the control task queue.
 *
 * @param task The control task to add.
 */
void BatteryTestingService::addControlTask(ControlTask* task) {
    std::lock_guard<std::mutex> lock(controlQueueMutex);
    controlTaskQueue.push(task);
    controlQueueCV.notify_one();
}

/**
 * @brief Adds a data task to the data task queue.
 *
 * @param task The data task to add.
 */
void BatteryTestingService::addDataTask(DataTask* task) {
    std::lock_guard<std::mutex> lock(dataQueueMutex);
    dataTaskQueue.push(task);
    dataQueueCV.notify_one();
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
        // Create a CallbackControlTask for each callback and add it to the control task queue
        for (const auto& callback : callbackMap[channel]) {
            CallbackControlTask* task = new CallbackControlTask(channel, callback, channelDataService);
            addControlTask(task);
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
 * @brief Thread function for the control threads.
 *
 * Executes control tasks from the control task queue.
 */
void BatteryTestingService::controlThreadFunction() {
    while (true) {
        std::unique_lock<std::mutex> lock(controlQueueMutex);
        controlQueueCV.wait(lock, [this]{ return !controlTaskQueue.empty(); });

        ControlTask* task = controlTaskQueue.top();
        controlTaskQueue.pop();
        lock.unlock();

        task->execute();
        delete task; // Clean up task after execution
    }
}

/**
 * @brief Thread function for the data thread.
 *
 * Executes data tasks from the data task queue.
 */
void BatteryTestingService::dataThreadFunction() {
    while (true) {
        std::unique_lock<std::mutex> lock(dataQueueMutex);
        dataQueueCV.wait(lock, [this]{ return !dataTaskQueue.empty(); });

        DataTask* task = dataTaskQueue.top();
        dataTaskQueue.pop();
        lock.unlock();

        task->execute();
        delete task; // Clean up task after execution
    }
}

/**
 * @brief Thread function for continuously receiving M4 data.
 *
 * This thread continuously receives data from the M4 core and processes it.
 * The data plane handles data processing (filtering, fitting, etc.), while
 * the control plane handles callbacks through CallbackControlTasks.
 */
void BatteryTestingService::m4DataThreadFunction() {
    // Example data for demonstration purposes
    std::map<std::string, float> sampleData[MAX_CHAN_NUM];
    while (true) {
        // In a real implementation, we would read from the M4 core
        // Something like:

        ReadFromM4("/dev/ttyRPMSG0", &sampleData);
        

        // For simulation purposes, iterate over all channels
        for (uint32_t channel = 0; channel < MAX_CHAN_NUM; channel++) {
            // Create data processing tasks (filtering, fitting, etc.)
            channelDataService->receiveM4Data(channel, sampleData[channel]);
            addDataTask(new FilteringDataTask(channel, sampleData[channel]));
            addDataTask(new FittingDataTask(channel, sampleData[channel]));
            if (channelDataService->isChannelSubscribed(channel)) {
                // Execute callbacks for subscribed channels
                handleCallbacks(channel);
            }
        }
        
        // Sleep to avoid excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
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
