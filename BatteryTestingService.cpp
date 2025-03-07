#include "BatteryTestingService.h"
#include "ChannelService.h"
#include "Task.h"
#include <iostream>

/**
 * @brief Constructor for the BatteryTestingService class.
 *
 * Initializes the control and data threads and the low-level services.
 */
BatteryTestingService::BatteryTestingService() :
    controlThread1(&BatteryTestingService::controlThreadFunction, this),
    controlThread2(&BatteryTestingService::controlThreadFunction, this),
    dataThread1(&BatteryTestingService::dataThreadFunction, this),
    dataThread2(&BatteryTestingService::dataThreadFunction, this) {
    // Initialize low-level services (replace with actual implementations)
    channelCtrlService = new class DummyChannelCtrlService : public ChannelCtrlService {
        void doConstantCurrent(uint32_t channel, float current) override {
            std::cout << "CC on channel " << channel << ", current: " << current << std::endl;
        }
        void doConstantVoltage(uint32_t channel, float voltage) override {
            std::cout << "CV on channel " << channel << ", voltage: " << voltage << std::endl;
        }
        void doRest(uint32_t channel) override {
            std::cout << "Rest on channel " << channel << std::endl;
        }
        void doOFF(uint32_t channel) override {
            std::cout << "OFF on channel " << channel << std::endl;
        }
    };

    channelDataService =  new class DummyChannelDataService : public ChannelDataService {
        void subscribeChannel(uint32_t channel) override {
            std::cout << "Subscribing to channel " << channel << std::endl;
        }
        float getVoltage(uint32_t channel) override {
            std::cout << "Getting voltage for channel " << channel << std::endl;
            return 0.0f; // Dummy value
        }
        float getCurrent(uint32_t channel) override {
            std::cout << "Getting current for channel " << channel << std::endl;
            return 0.0f; // Dummy value
        }
        void getM4Data() override {
            std::cout << "Getting M4 data" << std::endl;
        }
    };
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
    dataThread1.join();
    dataThread2.join();

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

    // 1. Add a control task to do constant current
    ControlTask* ccTask = new class CCTask : public ControlTask {
    public:
        CCTask(uint32_t channel, float current, ChannelCtrlService* ctrlService) : ControlTask(TaskPriority::NORMAL), channel(channel), current(current), ctrlService(ctrlService) {}
        void execute() override {
            ctrlService->doConstantCurrent(channel, current);
        }
    private:
        uint32_t channel;
        float current;
        ChannelCtrlService* ctrlService;
    };
    addControlTask(new CCTask(channel, current, channelCtrlService));

    // 2. Register a callback to check voltage and switch to CV
    registerCallback(channel, [this, channel, targetVoltage](uint32_t ch, const std::map<std::string, float>& data) {
        if (data.count("voltage") > 0 && data.at("voltage") >= targetVoltage) {
            std::cout << "Target voltage reached on channel " << channel << ", switching to CV" << std::endl;

            ControlTask* cvTask = new class CVTask : public ControlTask {
            public:
                CVTask(uint32_t channel, float targetVoltage, ChannelCtrlService* ctrlService) : ControlTask(TaskPriority::HIGH), channel(channel), targetVoltage(targetVoltage), ctrlService(ctrlService) {}
                void execute() override {
                    ctrlService->doConstantVoltage(channel, targetVoltage);
                }
            private:
                uint32_t channel;
                float targetVoltage;
                ChannelCtrlService* ctrlService;
            };
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
}

/**
 * @brief Sets the channel to a rest state (open circuit).
 *
 * @param channel The channel number.
 */
void BatteryTestingService::runRest(uint32_t channel) {
    std::cout << "Running Rest on channel " << channel << std::endl;
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
void BatteryTestingService::registerCallback(uint32_t channel, CallbackFunction callback) {
    callbackMap[channel] = callback;
}

/**
 * @brief Processes data received from the M4 core.
 *
 * @param channel The channel number.
 * @param data The data received from the M4 core.
 */
void BatteryTestingService::processM4Data(uint32_t channel, const std::map<std::string, float>& data) {
    std::cout << "Processing M4 data for channel " << channel << std::endl;
    // Simulate data processing and callback trigger
    if (callbackMap.count(channel) > 0) {
        callbackMap[channel](channel, data);
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
 * @brief Thread function for the data threads.
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