# Battery Testing Service Architecture

This is a tentative architecture for a battery testing service. The general idea is to do event-driven programming, break down executing a control type into smaller tasks, and adopt a layered architecture that is hardware agnostic and with modern C++ features. So feel free to add details and change design as long as the general design principles are followed.

## Architecture

The architecture is structured around a separation of concerns, with distinct control and data planes. This design facilitates efficient task management, asynchronous communication, and real-time data processing.

### 1. Core Components

*   **BatteryTestingService:** This class serves as the primary abstraction layer, providing a high-level interface for controlling battery testing operations. It encapsulates the underlying hardware interactions and exposes only public APIs for various control types, while keeping all helper functions private for better encapsulation.

*   **Control Types:** These represent the different control types that can be executed by the service. Each control type is implemented as a separate public API within the `BatteryTestingService` class.
    *   `runCCCV`.
    *   `runCurrentRamp`.
    *   `runRest`.

*   **Tasks:** Control types are further broken down into individual tasks, representing smaller units of work. Tasks are designed to be executed asynchronously, allowing for concurrent operation and efficient resource utilization. Task classes like `CCTask` and `CVTask` are defined independently in the Task.h file for better maintainability.

*   **Low-Level Services:** These services provide the interface for interacting with the hardware.
    *   `ChannelCtrlService`: Responsible for sending control commands to the M4 core.
    *   `ChannelDataService`: Responsible for maintaining a central data table with up-to-date information for all channels. It receives data from the M4 core through the `receiveM4Data` method, updating the channel data table and triggering any registered callbacks.

### 2. Task Management

The application employs a task-based architecture, where each control type is decomposed into a series of tasks. These tasks are managed by a unified task processing system with worker threads.

*   **Task Hierarchy:**
    *   `Task`: This is the virtual base class for all tasks. It defines the basic structure for tasks, including a priority level.
    *   `ControlTask`: This class inherits from the `Task` class and serves as the base class for all control-related tasks.
    *   `DataTask`: This class inherits from the `Task` class and serves as the base class for all data-related tasks.

*   **Unified Task Queue:** A single priority queue is used to manage all tasks (both control and data tasks). This queue ensures that high-priority tasks are executed first, regardless of their type.
    *   `taskQueue`: Stores all tasks in a single priority queue.

*   **Threads:** A configurable number of worker threads process tasks from the unified task queue, along with a dedicated thread for receiving M4 data. This allows for dynamic scaling and efficient resource utilization.
    *   `workerThreads`: A vector of worker threads that process any type of task from the task queue.
    *   `m4DataThread`: Dedicated thread continuously receiving data from the M4 core and adding tasks to the task queue.
    *   The number of worker threads can be dynamically adjusted at runtime using the `setWorkerThreadCount` method.

### 3. Unified Task Processing with Worker Threads

The architecture has been updated to use a unified task processing approach with worker threads, while still maintaining the separation of concerns between control and data functionality.

* **Unified Task Processing:** All tasks (both control and data related) are processed by a pool of worker threads:
  * A single task queue holds all tasks, prioritized by their importance
  * Any worker thread can process any type of task, improving resource utilization
  * The number of worker threads can be dynamically adjusted based on system load

* **Control Functionality:** Still responsible for managing control-related operations:
  * Manages callback execution for subscribed channels through CallbackControlTask instances
  * Maintains a registry of callback functions for each subscribed channel
  * Executes callback functions in response to data events

* **Data Functionality:** Still responsible for maintaining channel information and processing data:
  * Manages a comprehensive table containing up-to-date measurements (voltage, current, dv/dt, etc.) for ALL channels
  * Continuously receives data from the M4 core through the dedicated m4DataThread
  * Processes raw data for ALL channels through various data tasks (filtering, fitting, calculations)
  * Provides access to current channel data values through getter methods
  * Notifies the system when new data is available for subscribed channels

### 4. Asynchronous Communication and Callbacks

The application utilizes a callback mechanism. This allows the A7 cores to react to data changes in real-time.

*   **Central Data Table:** The ChannelDataService maintains a comprehensive data table that stores up-to-date information for all  channels, including voltage, current, dv/dt, and other metrics. This table is continuously updated with incoming data from the M4 core.

*   **Callback Mechanism in Control Plane:** Callback functions are registered with the BatteryTestingService (not the ChannelDataService) for each channel. The control plane maintains a callbackMap that stores multiple callback functions per channel, and they are executed as CallbackControlTasks when new data is available. This separation ensures that:
    * Data processing occurs in the data plane
    * Callback execution occurs in the control plane
    
*   **Configurable Callbacks:** The callback functions are configurable at runtime, allowing for flexible adaptation to different testing scenarios. They can be registered and unregistered as needed.

*   **Data Processing Flow:**
    1. The data plane receives data from the M4 core through its `receiveM4Data` method
    2. The data plane updates the channel data table with the new values
    3. For subscribed channels, the data plane notifies the control plane about new data
    4. The control plane creates a CallbackControlTask for each callback registered for the channel
    5. Each CallbackControlTask is executed in the control thread, reading the latest data and running its callback

*   **CallbackControlTask:** This specialized task reads the current data from the ChannelDataService and executes the registered callback in the control plane context. This ensures that callbacks have access to the most up-to-date data and are executed in the appropriate thread context.

### 5. Example: Constant Current Constant Voltage (CCCV)

The CCCV control type demonstrates the unified task processing architecture:

1. The `runCCCV` function is called with the desired channel, current, and target voltage
2. The channel is subscribed in the data service
3. A control task (CCTask) is added to the unified task queue to set the channel to constant current mode
4. A callback function is registered with the BatteryTestingService to monitor voltage
5. When new data is available:
   - The data service updates the channel data table
   - The system creates a CallbackControlTask that:
     - Reads the latest data from the data service
     - Checks if the target voltage has been reached
6. When the target voltage is reached, the callback adds a CV task to the unified task queue to switch to constant voltage mode. Then the callback is unregistered, and a new callback is registered to monitor the CV phase.

#### Step Limits in CCCV
1. A vector of `StepLimit` pairs is provided when calling `runCCCV`
2. Each `StepLimit` consists of:
   - A channel variable type (voltage, current, temperature, capacity, time, etc.)
   - A target value for that variable
3. Additional callbacks are registered to monitor each step limit condition
4. When any step limit condition is met (when the specified variable reaches its target value):
   - The control type is terminated
   - The channel is set to rest or turned off
   - All callbacks are unregistered
   - The channel is unsubscribed

### 6. Worker Thread Architecture

The BatteryTestingService now employs a worker thread architecture.

### 7. Generic Control Task

The `GenericControlTask` class provides a way to group multiple `ChannelCtrlService` calls into a single task. This can be useful for complex control sequences that require multiple hardware interactions.
## Class Definitions

*   **Task.h:** Defines the base class for all tasks, as well as specific task types like CCTask and CVTask.
*   **ChannelService.h:** Defines the interfaces for the `ChannelCtrlService` and `ChannelDataService` classes, including the data processing functionality in ChannelDataService.
*   **BatteryTestingService.h:** Defines the `BatteryTestingService` class, which manages tasks with a unified worker thread pool. It provides:
    * A public API focused solely on high-level control functions
    * Dynamic control of the number of worker threads through `setWorkerThreadCount` and `getWorkerThreadCount` methods
    * A unified task queue for all task types
    * A dedicated M4 data thread for continuous data reception


