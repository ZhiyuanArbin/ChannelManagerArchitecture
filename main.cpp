#include "BatteryTestingService.h"
#include "ChannelService.h"
#include <iostream>
#include <map>

/**
 * @brief This is the main entry point for the Battery Testing application.
 *
 */
int main() {
    BatteryTestingService service;

    // Example usage:
    service.runCCCV(1, 2.0f, 4.2f); // Channel 1, 2.0A, target 4.2V

    return 0;
}