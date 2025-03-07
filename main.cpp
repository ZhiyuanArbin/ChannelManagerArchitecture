#include "BatteryTestingService.h"
#include <iostream>
#include <map>

/**
 * @brief This is the main entry point for the Battery Testing application.
 *
 * This application simulates a battery testing service that controls and monitors
 * battery testing hardware. It demonstrates the use of the BatteryTestingService
 * class, which manages control and data tasks, handles asynchronous communication
 * with the M4 core, and provides a callback mechanism for reacting to data changes.
 */
int main() {
    BatteryTestingService service;

    // Example usage:
    service.runCCCV(1, 2.0f, 4.2f); // Channel 1, 2.0A, target 4.2V

    // Simulate receiving data from M4
    std::map<std::string, float> data;
    data["voltage"] = 3.5f;
    service.processM4Data(1, data);

    data["voltage"] = 4.2f;
    service.processM4Data(1, data); // This should trigger the CV task

    return 0;
}