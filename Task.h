#ifndef TASK_H
#define TASK_H

#include <queue>
#include <functional>
#include <vector>

class ChannelCtrlService;

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

#endif