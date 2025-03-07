#ifndef CHANNELSERVICE_H
#define CHANNELSERVICE_H

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <string>

// Forward declaration
class Task;

/**
 * @brief Abstract base class for channel control services.
 *
 * This class defines the interface for controlling the hardware channels.
 */
class ChannelCtrlService {
public:
    /**
     * @brief Constructor for the ChannelCtrlService class.
     */
    ChannelCtrlService() {}

    /**
     * @brief Virtual destructor for the ChannelCtrlService class.
     */
    virtual ~ChannelCtrlService() {}

    /**
     * @brief Performs constant current control on a channel.
     *
     * @param channel The channel number.
     * @param current The target current value.
     */
    virtual void doConstantCurrent(uint32_t channel, float current) = 0;

    /**
     * @brief Performs constant voltage control on a channel.
     *
     * @param channel The channel number.
     * @param voltage The target voltage value.
     */
    virtual void doConstantVoltage(uint32_t channel, float voltage) = 0;

    /**
     * @brief Sets the channel to a rest state (open circuit).
     *
     * @param channel The channel number.
     */
    virtual void doRest(uint32_t channel) = 0;

    /**
     * @brief Turns off the channel.
     *
     * @param channel The channel number.
     */
    virtual void doOFF(uint32_t channel) = 0;

    // ... other control functions
};

/**
 * @brief Abstract base class for channel data services.
 *
 * This class defines the interface for reading and processing data from the hardware channels.
 */
class ChannelDataService {
public:
    /**
     * @brief Type definition for callback functions.
     */
    using CallbackFunction = std::function<void(uint32_t channel, const std::map<std::string, float>& data)>;

    /**
     * @brief Constructor for the ChannelDataService class.
     */
    ChannelDataService() {}

    /**
     * @brief Virtual destructor for the ChannelDataService class.
     */
    virtual ~ChannelDataService() {}

    /**
     * @brief Subscribes to data updates for a specific channel.
     *
     * @param channel The channel number.
     */
    virtual void subscribeChannel(uint32_t channel) = 0;

    /**
     * @brief Gets the voltage value for a specific channel.
     *
     * @param channel The channel number.
     * @return The voltage value.
     */
    virtual float getVoltage(uint32_t channel) = 0;

    /**
     * @brief Gets the current value for a specific channel.
     *
     * @param channel The channel number.
     * @return The current value.
     */
    virtual float getCurrent(uint32_t channel) = 0;

    // ... other get data functions 

    /**
     * @brief Receives data from the M4 core through RPMsg.
     */
    virtual void getM4Data() = 0; // Receives data from M4 through RPMsg
    
    /**
     * @brief Registers a callback function for a specific channel.
     *
     * @param channel The channel number.
     * @param callback The callback function to register.
     */
    virtual void registerCallback(uint32_t channel, CallbackFunction callback) = 0;
    
    /**
     * @brief Processes data received from the M4 core.
     *
     * @param channel The channel number.
     * @param data The data received from the M4 core.
     */
    virtual void processM4Data(uint32_t channel, const std::map<std::string, float>& data) = 0;
};

/**
 * @brief Dummy implementation of ChannelCtrlService for testing purposes.
 *
 * This class provides a simple implementation that logs control operations to the console
 * rather than interacting with actual hardware.
 */
class DummyChannelCtrlService : public ChannelCtrlService {
public:
    /**
     * @brief Performs constant current control on a channel.
     *
     * @param channel The channel number.
     * @param current The target current value.
     */
    void doConstantCurrent(uint32_t channel, float current) override {
        std::cout << "CC on channel " << channel << ", current: " << current << std::endl;
    }
    
    /**
     * @brief Performs constant voltage control on a channel.
     *
     * @param channel The channel number.
     * @param voltage The target voltage value.
     */
    void doConstantVoltage(uint32_t channel, float voltage) override {
        std::cout << "CV on channel " << channel << ", voltage: " << voltage << std::endl;
    }
    
    /**
     * @brief Sets the channel to a rest state (open circuit).
     *
     * @param channel The channel number.
     */
    void doRest(uint32_t channel) override {
        std::cout << "Rest on channel " << channel << std::endl;
    }
    
    /**
     * @brief Turns off the channel.
     *
     * @param channel The channel number.
     */
    void doOFF(uint32_t channel) override {
        std::cout << "OFF on channel " << channel << std::endl;
    }
};

/**
 * @brief Dummy implementation of ChannelDataService for testing purposes.
 *
 * This class provides a simple implementation that logs data operations to the console
 * and returns dummy values rather than reading from actual hardware.
 */
class DummyChannelDataService : public ChannelDataService {
private:
    std::map<uint32_t, CallbackFunction> callbackMap;
    
public:
    /**
     * @brief Subscribes to data updates for a specific channel.
     *
     * @param channel The channel number.
     */
    void subscribeChannel(uint32_t channel) override {
        std::cout << "Subscribing to channel " << channel << std::endl;
    }
    
    /**
     * @brief Gets the voltage value for a specific channel.
     *
     * @param channel The channel number.
     * @return The voltage value (dummy value of 0.0).
     */
    float getVoltage(uint32_t channel) override {
        std::cout << "Getting voltage for channel " << channel << std::endl;
        return 0.0f; // Dummy value
    }
    
    /**
     * @brief Gets the current value for a specific channel.
     *
     * @param channel The channel number.
     * @return The current value (dummy value of 0.0).
     */
    float getCurrent(uint32_t channel) override {
        std::cout << "Getting current for channel " << channel << std::endl;
        return 0.0f; // Dummy value
    }
    
    /**
     * @brief Receives data from the M4 core through RPMsg.
     */
    void getM4Data() override {
        std::cout << "Getting M4 data" << std::endl;
    }
    
    /**
     * @brief Registers a callback function for a specific channel.
     *
     * @param channel The channel number.
     * @param callback The callback function to register.
     */
    void registerCallback(uint32_t channel, CallbackFunction callback) override {
        std::cout << "Registering callback for channel " << channel << std::endl;
        callbackMap[channel] = callback;
    }
    
    /**
     * @brief Processes data received from the M4 core.
     *
     * @param channel The channel number.
     * @param data The data received from the M4 core.
     */
    void processM4Data(uint32_t channel, const std::map<std::string, float>& data) override {
        std::cout << "Processing M4 data for channel " << channel << std::endl;
        // Simulate data processing and callback trigger
        if (callbackMap.count(channel) > 0) {
            callbackMap[channel](channel, data);
        }
    }
};

#endif