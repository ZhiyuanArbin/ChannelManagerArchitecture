#include "BatteryTestingService.h"
#include "ChannelService.h"
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

    // For demonstration purposes, we create a ChannelDataService to simulate data flow
    // In a real implementation, this would be managed through the system architecture
    class SimulationDataService : public ChannelDataService {
    private:
        std::map<uint32_t, CallbackFunction> callbackMap;
        
    public:
        void subscribeChannel(uint32_t channel) override {}
        float getVoltage(uint32_t channel) override { return 0.0f; }
        float getCurrent(uint32_t channel) override { return 0.0f; }
        void getM4Data() override {}
        
        void registerCallback(uint32_t channel, CallbackFunction callback) override {
            callbackMap[channel] = callback;
        }
        
        void processM4Data(uint32_t channel, const std::map<std::string, float>& data) override {
            std::cout << "Processing M4 data for channel " << channel << std::endl;
            if (callbackMap.count(channel) > 0) {
                callbackMap[channel](channel, data);
            }
        }
        
        // Helper function for the simulation
        void simulateData(uint32_t channel, float voltage) {
            std::map<std::string, float> data;
            data["voltage"] = voltage;
            processM4Data(channel, data);
        }
    };
    
    // This is just for demonstration - in the real system, data would flow through
    // the established channel data service registered with the battery testing service
    SimulationDataService dataSimulator;
    
    // Simulate receiving data from M4
    dataSimulator.simulateData(1, 3.5f);
    
    // In a real scenario, the voltage reaching the target would trigger the CV mode
    dataSimulator.simulateData(1, 4.2f);

    return 0;
}