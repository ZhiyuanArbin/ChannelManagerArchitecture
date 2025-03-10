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
 */
void BatteryTestingService::runCCCV(uint32_t channel, float current, float targetVoltage) {
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
            
            // Optionally, unregister the callback once we've switched to CV
            // unregisterCallback(channel);
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
    callbackMap[channel] = callback;
}

/**
 * @brief Handles notifications of new data from the data plane.
 * Creates and adds a CallbackControlTask to execute the registered callback.
 *
 * @param channel The channel number with new data.
 */
void BatteryTestingService::handleNewDataNotification(uint32_t channel) {
    std::cout << "Handling new data notification for channel " << channel << std::endl;
    
    // Check if there's a callback registered for this channel
    if (callbackMap.find(channel) != callbackMap.end()) {
        // Create a CallbackControlTask and add it to the control task queue
        CallbackControlTask* task = new CallbackControlTask(channel, callbackMap[channel], channelDataService);
        addControlTask(task);
    }
}

/**
 * @brief Unregisters a callback function for a specific channel.
 *
 * @param channel The channel number.
 */
void BatteryTestingService::unregisterCallback(uint32_t channel) {
    std::cout << "Unregistering callback for channel " << channel << std::endl;
    callbackMap.erase(channel);
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
    std::map<std::string, float> sampleData;
    
    // Connect the data plane notification to the control plane
    // This is pseudo-code - in a real implementation, there would be a proper way
    // to set up the notification callback
    DummyChannelDataService* dummyDataService = static_cast<DummyChannelDataService*>(channelDataService);
    dummyDataService->setNotifyCallback([this](uint32_t channel) {
        // When the data service notifies about new data, handle it in the control plane
        this->handleNewDataNotification(channel);
    });
    
    while (true) {
        // In a real implementation, we would read from the M4 core
        // Something like:
        // ReadFromM4Core(&channel, &rawData);
        
        // For simulation purposes, iterate over all channels
        for (uint32_t channel = 0; channel < MAX_CHAN_NUM; channel++) {
            // Simulate getting data
            sampleData["voltage"] = 3.7f + (channel * 0.1f);
            sampleData["current"] = 1.0f - (channel * 0.05f);
            sampleData["temperature"] = 25.0f + channel;
            sampleData["timestamp"] = static_cast<float>(time(nullptr));
            
            // Process data in the data plane for ALL channels
            // Create data processing tasks (filtering, fitting, etc.)
            addDataTask(new FilteringDataTask(channel, sampleData));
            addDataTask(new FittingDataTask(channel, sampleData));
            
            // Update the channel data table with processed data for ALL channels
            bool updated = channelDataService->receiveM4Data(channel, sampleData);
            
            // Note: The notification to the control plane is only triggered
            // for subscribed channels within receiveM4Data method
            // The control plane then executes the registered callbacks for those channels only
        }
        
        // Sleep to avoid excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
        // Assuming all services are the same, but you can add logic to handle different service types
        // service->execute(); // Execute the service (assuming a common execute method)
        delete service; // Clean up the service after execution
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

/**
 * @brief Executes the fitting data task.
 *
 * In a real implementation, this would apply a mathematical fitting algorithm
 * to the raw data to extract patterns or trends.
 */
void FittingDataTask::execute() {
    std::cout << "Executing fitting algorithm on data for channel " << channel << std::endl;
    
    // In a real implementation, this would:
    // 1. Apply mathematical fitting to the raw data
    // 2. Store the results in the channel data table
    // 3. Calculate derived metrics
    
    // For demonstration, we just log the action
}

/**
 * @brief Executes the filtering data task.
 *
 * In a real implementation, this would apply a filtering algorithm
 * to clean the raw data by removing noise or outliers.
 */
void FilteringDataTask::execute() {
    std::cout << "Executing filtering algorithm on data for channel " << channel << std::endl;
    
    // In a real implementation, this would:
    // 1. Apply filters to clean the raw data
    // 2. Store the filtered data in the channel data table
    
    // For demonstration, we just log the action
}

/**
 * @brief Executes the notification task.
 *
 * This is a data plane task that notifies the control plane about
 * new data availability for a channel.
 */
void NotifyCallbackTask::execute() {
    std::cout << "Notifying control plane about new data for channel " << channel << std::endl;
    
    // In a real implementation, this would communicate with the BatteryTestingService
    // to trigger callback handling in the control plane
}