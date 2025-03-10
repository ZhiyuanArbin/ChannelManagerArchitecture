#ifndef TASK_H
#define TASK_H

#include <queue>
#include <functional>
#include <vector>
#include <map>
#include <string>

class ChannelCtrlService;
class ChannelDataService;

// Forward declaration
class Task;
class ControlTask;
class DataTask;

// Enum for task priority
enum class TaskPriority {
    HIGH,
    NORMAL,
    LOW
};

// Structure for comparing task priorities in the priority queue
struct TaskComparator {
    bool operator()(const Task* a, const Task* b) {
        return a->priority > b->priority; // Higher priority tasks come first
    }
};

/**
 * @brief Base class for all tasks.
 *
 * This class defines the basic structure for tasks that can be executed by the system.
 */
class Task {
public:
    /**
     * @brief Constructor for the Task class.
     *
     * @param priority The priority of the task.
     */
    Task(TaskPriority priority) : priority(priority) {}

    /**
     * @brief Virtual destructor for the Task class.
     */
    virtual ~Task() {}

    /**
     * @brief Executes the task.
     *
     * This is a pure virtual function that must be implemented by derived classes.
     */
    virtual void execute() = 0;

    /**
     * @brief The priority of the task.
     */
    TaskPriority priority;
};

/**
 * @brief Base class for control tasks.
 *
 * This class inherits from the Task class and provides a base for all control-related tasks.
 */
class ControlTask : public Task {
public:
    /**
     * @brief Constructor for the ControlTask class.
     *
     * @param priority The priority of the task.
     */
    ControlTask(TaskPriority priority) : Task(priority) {}

    /**
     * @brief Virtual destructor for the ControlTask class.
     */
    virtual ~ControlTask() {}
};

/**
 * @brief Base class for data tasks.
 *
 * This class inherits from the Task class and provides a base for all data-related tasks.
 */
class DataTask : public Task {
public:
    /**
     * @brief Constructor for the DataTask class.
     *
     * @param priority The priority of the task.
     */
    DataTask(TaskPriority priority) : Task(priority) {}

    /**
     * @brief Virtual destructor for the DataTask class.
     */
    virtual ~DataTask() {}
};

/**
 * @brief Generic Control Task that can hold a queue of ChannelCtrlServices.
 */
class GenericControlTask : public ControlTask {
public:
    /**
     * @brief Constructor for the GenericControlTask class.
     *
     * @param priority The priority of the task.
     */
    GenericControlTask(TaskPriority priority) : ControlTask(priority) {}

    /**
     * @brief Virtual destructor for the GenericControlTask class.
     */
    virtual ~GenericControlTask() {}

    /**
     * @brief Executes the generic control task.
     */
    void execute() override;

    /**
     * @brief Adds a ChannelCtrlService to the queue.
     *
     * @param service The ChannelCtrlService to add.
     */
    void addCtrlService(ChannelCtrlService* service);

private:
    /**
     * @brief Queue of ChannelCtrlServices.
     */
    std::queue<ChannelCtrlService*> ctrlServices;
};

/**
 * @brief Constant Current Task
 *
 * Sets a channel to constant current mode
 */
class CCTask : public ControlTask {
public:
    /**
     * @brief Constructor for the CCTask class.
     *
     * @param channel The channel number.
     * @param current The target current value.
     * @param ctrlService Pointer to the channel control service.
     */
    CCTask(uint32_t channel, float current, ChannelCtrlService* ctrlService)
        : ControlTask(TaskPriority::NORMAL), channel(channel), current(current), ctrlService(ctrlService) {}
    
    /**
     * @brief Executes the constant current task.
     */
    void execute() override;

private:
    uint32_t channel;
    float current;
    ChannelCtrlService* ctrlService;
};

/**
 * @brief Constant Voltage Task
 *
 * Sets a channel to constant voltage mode
 */
class CVTask : public ControlTask {
public:
    /**
     * @brief Constructor for the CVTask class.
     *
     * @param channel The channel number.
     * @param targetVoltage The target voltage value.
     * @param ctrlService Pointer to the channel control service.
     */
    CVTask(uint32_t channel, float targetVoltage, ChannelCtrlService* ctrlService)
        : ControlTask(TaskPriority::NORMAL), channel(channel), targetVoltage(targetVoltage), ctrlService(ctrlService) {}
    
    /**
     * @brief Executes the constant voltage task.
     */
    void execute() override;

private:
    uint32_t channel;
    float targetVoltage;
    ChannelCtrlService* ctrlService;
};

/**
 * @brief Callback Control Task to handle callback functions for subscribed channels.
 *
 * This class handles callback functions in the control plane instead of the data plane.
 * It reads data from the channel data table and executes callback logic based on the data.
 */
class CallbackControlTask : public ControlTask {
public:
    // Define callback function type
    using CallbackFunction = std::function<void(uint32_t channel, const std::map<std::string, float>& data)>;
    
    /**
     * @brief Constructor for the CallbackControlTask class.
     *
     * @param channel The channel number.
     * @param callback The callback function to execute.
     * @param dataService Pointer to the channel data service to read data from.
     */
    CallbackControlTask(uint32_t channel, CallbackFunction callback, ChannelDataService* dataService)
        : ControlTask(TaskPriority::HIGH), channel(channel), callback(callback), dataService(dataService) {}
    
    /**
     * @brief Executes the callback task.
     *
     * Reads current data for the channel from the data table and executes the callback.
     */
    void execute() override;

private:
    uint32_t channel;
    CallbackFunction callback;
    ChannelDataService* dataService;
};


/**
 * @brief Fitting data task for processing raw data
 */
class FittingDataTask : public DataTask {
public:
    /**
     * @brief Constructor for the FittingDataTask class.
     *
     * @param channel The channel number.
     * @param data The raw data to process.
     */
    FittingDataTask(uint32_t channel, const std::map<std::string, float> &data)
        : DataTask(TaskPriority::NORMAL), channel(channel), rawData(data) {}

    /**
     * @brief Executes the fitting algorithm on the raw data.
     */
    void execute() override;

private:
    uint32_t channel;
    std::map<std::string, float> rawData;
};

/**
 * @brief Filtering data task for cleaning noisy data
 */
class FilteringDataTask : public DataTask {
public:
    /**
     * @brief Constructor for the FilteringDataTask class.
     *
     * @param channel The channel number.
     * @param data The data to filter.
     */
    FilteringDataTask(uint32_t channel, const std::map<std::string, float>& data)
        : DataTask(TaskPriority::NORMAL), channel(channel), rawData(data) {}
    
    /**
     * @brief Executes the filtering algorithm on the raw data.
     */
    void execute() override;

private:
    uint32_t channel;
    std::map<std::string, float> rawData;
};


#endif