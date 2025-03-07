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

    // 1. Create and add a control task to do constant current
    addControlTask(new CCTask(channel, current, channelCtrlService));

    // 2. Register a callback to check voltage and switch to CV
    channelDataService->registerCallback(channel, [this, channel, targetVoltage](uint32_t ch, const std::map<std::string, float>& data) {
        if (data.count("voltage") > 0 && data.at("voltage") >= targetVoltage) {
            std::cout << "Target voltage reached on channel " << channel << ", switching to CV" << std::endl;

            // Create and add a CV task
            addControlTask(new CVTask(channel, targetVoltage, channelCtrlService));
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
 * This thread continuously receives data from the M4 core and updates
 * the channel data table maintained by the ChannelDataService.
 */
void BatteryTestingService::m4DataThreadFunction() {
    // Example data for demonstration purposes - in a real implementation,
    // this would come from the hardware interface
    std::map<std::string, float> sampleData;
    
    while (true) {
        // Continuously poll for new data from M4 core
        // In a real implementation, this might:
        // 1. Wait for an interrupt or poll a hardware interface
        // 2. Process raw data into meaningful values
        // 3. Update the central data table
        for (uint32_t channel = 0; channel < 8; channel++) {  // Assuming 8 channels for example
            // In a real implementation, we would get actual data from hardware
            // Here we're just creating some dummy data for demonstration
            sampleData["voltage"] = 3.7f + (channel * 0.1f);  // Example voltage values
            sampleData["current"] = 1.0f - (channel * 0.05f); // Example current values
            sampleData["temperature"] = 25.0f + channel;      // Example temperature values
            sampleData["timestamp"] = static_cast<float>(time(nullptr));
            
            // Call receiveM4Data to update the channel data table
            channelDataService->receiveM4Data(channel, sampleData);
        }
        // Sleep to avoid excessive CPU usage
        // In a real implementation, this might synchronize with hardware timing
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