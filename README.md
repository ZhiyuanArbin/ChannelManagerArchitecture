# Battery Testing Service Architecture

## Overview

The Battery Testing Service is designed for controlling and monitoring battery testing equipment. It is specifically tailored for an embedded Linux environment running on an STM32MP157 System on Chip (SoC), which features dual A7 cores and an M4 core. The application leverages the A7 cores for high-level control and data processing, while the M4 core handles low-level hardware interactions.

## Architecture

The architecture is structured around a separation of concerns, with distinct control and data planes. This design facilitates efficient task management, asynchronous communication, and real-time data processing.

### 1. Core Components

*   **BatteryTestingService:** This class serves as the primary abstraction layer, providing a high-level interface for controlling battery testing operations. It encapsulates the underlying hardware interactions and exposes only public APIs for various control types, while keeping all helper functions private for better encapsulation.

*   **Control Types:** These represent the different testing modes that can be executed by the service. Each control type is implemented as a separate public API within the `BatteryTestingService` class.
    *   `runCCCV`: Executes a Constant Current Constant Voltage test.
    *   `runDCIM`: Executes a Direct Current Internal Measurement test.
    *   `runRest`: Sets the channel to a rest state (open circuit).

*   **Tasks:** Control types are further broken down into individual tasks, representing smaller units of work. Tasks are designed to be executed asynchronously, allowing for concurrent operation and efficient resource utilization. Task classes like `CCTask` and `CVTask` are defined independently in the Task.h file for better maintainability.

*   **Low-Level Services:** These services provide the interface for interacting with the hardware.
    *   `ChannelCtrlService`: Responsible for sending control commands to the M4 core.
    *   `ChannelDataService`: Responsible for reading and processing data received from the M4 core, including data processing functionality through the `processM4Data` method.

### 2. Task Management

The application employs a task-based architecture, where each control type is decomposed into a series of tasks. These tasks are then managed by separate control and data planes.

*   **Task Hierarchy:**
    *   `Task`: This is the virtual base class for all tasks. It defines the basic structure for tasks, including a priority level.
    *   `ControlTask`: This class inherits from the `Task` class and serves as the base class for all control-related tasks.
    *   `DataTask`: This class inherits from the `Task` class and serves as the base class for all data-related tasks.

*   **Task Queues:** Separate priority queues are used to manage control and data tasks. These queues ensure that high-priority tasks are executed first.
    *   `controlTaskQueue`: Stores control tasks.
    *   `dataTaskQueue`: Stores data tasks.

*   **Threads:** Dedicated threads are used to process tasks from the control and data queues. This allows for concurrent execution of tasks, improving overall system performance.
    *   `controlThread1` and `controlThread2`: Process control tasks.
    *   `dataThread1` and `dataThread2`: Process data tasks.

### 3. Control and Data Planes

The architecture is divided into distinct control and data planes to ensure separation of concerns and efficient resource utilization.

*   **Control Plane:** Responsible for managing control-related tasks, such as setting hardware parameters and initiating testing sequences.
*   **Data Plane:** Responsible for reading and processing data received from the hardware, such as voltage, current, and temperature measurements.

### 4. Asynchronous Communication and Callbacks

The application utilizes a callback mechanism to handle asynchronous communication between the A7 and M4 cores. This allows the A7 cores to react to data changes in real-time.

*   **Callback Mechanism:** A callback function is registered with the ChannelDataService for each channel, which is triggered when new data is received from the M4 core. The callback function can then perform specific actions based on the received data, such as switching to a different control mode.

*   **Configurable Callbacks:** The callback functions are configurable at runtime, allowing for flexible adaptation to different testing scenarios.

*   **Data Processing:** The ChannelDataService is responsible for processing incoming data from the M4 core through its `processM4Data` method, which invokes the appropriate callbacks based on the channel and data received.

### 5. Example: Constant Current Constant Voltage (CCCV)

The CCCV control type demonstrates the use of the callback mechanism.

1.  The `runCCCV` function is called with the desired channel, current, and target voltage.
2.  A control task is added to the control task queue to set the channel to constant current mode.
3.  A callback function is registered for the channel, which checks the voltage level.
4.  When the voltage reaches the target voltage, the callback function adds a control task to the control task queue to switch the channel to constant voltage mode.

### 6. Generic Control Task

The `GenericControlTask` class provides a way to group multiple `ChannelCtrlService` calls into a single task. This can be useful for complex control sequences that require multiple hardware interactions.

## Class Definitions

*   **Task.h:** Defines the base class for all tasks, as well as specific task types like CCTask and CVTask.
*   **ChannelService.h:** Defines the interfaces for the `ChannelCtrlService` and `ChannelDataService` classes, including the data processing functionality in ChannelDataService.
*   **BatteryTestingService.h:** Defines the `BatteryTestingService` class, which manages control and data tasks, with a public API focused solely on high-level control functions.

## Usage

The `main.cpp` file provides an example of how to use the `BatteryTestingService` class.

1.  Create an instance of the `BatteryTestingService` class.
2.  Call the desired control type function, such as `runCCCV`.
3.  Data from the M4 core is processed through the ChannelDataService's `processM4Data` function.

## Conclusion

The Battery Testing Service provides a flexible and efficient architecture for controlling and monitoring battery testing equipment. Its modular design, separation of concerns, and asynchronous communication capabilities make it well-suited for real-time embedded Linux environments.


### Future Recommendations

Future improvements could include:

1. **Memory Management**: Replace raw pointers with smart pointers (std::unique_ptr, std::shared_ptr) to prevent memory leaks and improve code safety.

2. **Thread Safety**: Enhance thread safety around the callback mechanism, particularly for registering and invoking callbacks.

3. **Error Handling**: Implement a more robust error handling strategy, either using exceptions or error codes, to manage failures gracefully.
