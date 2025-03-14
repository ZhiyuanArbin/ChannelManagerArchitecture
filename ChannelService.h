#ifndef CHANNELSERVICE_H
#define CHANNELSERVICE_H

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#define MAX_CHAN_NUM 32
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

    // virtual const 
};

/**
 * @brief Abstract base class for channel data services.
 *
 * This class defines the interface for reading and processing data from the hardware channels.
 * It maintains a table of up-to-date information for all subscribed channels.
 * The data plane focuses on data collection and processing, while callbacks are handled
 * by the control plane.
 */
class ChannelDataService {
public:
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
     * @brief Unsubscribes from data updates for a specific channel.
     *
     * @param channel The channel number.
     */
    virtual void unsubscribeChannel(uint32_t channel) = 0;

    /**
     * @brief Checks if a channel is subscribed.
     *
     * @param channel The channel number.
     * @return True if subscribed, false otherwise.
     */
    virtual bool isChannelSubscribed(uint32_t channel) const = 0;

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

    /**
     * @brief Gets the voltage derivative (dv/dt) for a specific channel.
     *
     * @param channel The channel number.
     * @return The dv/dt value.
     */
    virtual float getDvDt(uint32_t channel) = 0;

    /**
     * @brief Gets all current data for a specific channel.
     *
     * @param channel The channel number.
     * @return A reference to the channel's data map.
     */
    virtual const std::map<std::string, float>& getChannelData(uint32_t channel) const = 0;

    // ... other get data functions
    
    /**
     * @brief Receives data from the M4 core.
     *
     * @param channel The channel number.
     * @param data The data received from the M4 core.
     */
    virtual void receiveM4Data(uint32_t channel, const std::map<std::string, float>& data) = 0;
    
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
 * It maintains a data table for all subscribed channels.
 */
class DummyChannelDataService : public ChannelDataService {
private:
    // Channel data table to store up-to-date information for all channels
    std::map<uint32_t, std::map<std::string, float>> channelDataTable;
    
    // Map to track subscribed channels
    std::map<uint32_t, bool> subscribedChannels;
    
public:
    /**
     * @brief Subscribes to data updates for a specific channel.
     *
     * @param channel The channel number.
     */
    void subscribeChannel(uint32_t channel) override {
        std::cout << "Subscribing to channel " << channel << std::endl;
        subscribedChannels[channel] = true;
    }
    
    /**
     * @brief Unsubscribes from data updates for a specific channel.
     *
     * @param channel The channel number.
     */
    void unsubscribeChannel(uint32_t channel) override {
        std::cout << "Unsubscribing from channel " << channel << std::endl;
        subscribedChannels[channel] = false;
    }
    
    /**
     * @brief Checks if a channel is subscribed.
     *
     * @param channel The channel number.
     * @return True if subscribed, false otherwise.
     */
    bool isChannelSubscribed(uint32_t channel) const override {
        auto it = subscribedChannels.find(channel);
        return it != subscribedChannels.end() && it->second;
    }
    
    /**
     * @brief Gets the voltage value for a specific channel.
     *
     * @param channel The channel number.
     * @return The voltage value from the data table.
     */
    float getVoltage(uint32_t channel) override {
        std::cout << "Getting voltage for channel " << channel << std::endl;
        if (channelDataTable.count(channel) > 0) {
            return channelDataTable[channel]["voltage"];
        }
        return 0.0f; // Default value
    }
    
    /**
     * @brief Gets the current value for a specific channel.
     *
     * @param channel The channel number.
     * @return The current value from the data table.
     */
    float getCurrent(uint32_t channel) override {
        std::cout << "Getting current for channel " << channel << std::endl;
        if (channelDataTable.count(channel) > 0) {
            return channelDataTable[channel]["current"];
        }
        return 0.0f; // Default value
    }
    
    /**
     * @brief Gets the voltage derivative (dv/dt) for a specific channel.
     *
     * @param channel The channel number.
     * @return The dv/dt value from the data table.
     */
    float getDvDt(uint32_t channel) override {
        std::cout << "Getting dv/dt for channel " << channel << std::endl;
        if (channelDataTable.count(channel) > 0) {
            return channelDataTable[channel]["dvdt"];
        }
        return 0.0f; // Default value
    }
    
    /**
     * @brief Gets all current data for a specific channel.
     *
     * @param channel The channel number.
     * @return A reference to the channel's data map.
     */
    const std::map<std::string, float>& getChannelData(uint32_t channel) const override {
        static const std::map<std::string, float> emptyMap;
        if (channelDataTable.count(channel) > 0) {
            return channelDataTable.at(channel);
        }
        return emptyMap; // Return empty map if channel not found
    }
    
    /**
     * @brief Receives data from the M4 core.
     * Updates the channel data table.
     *
     * @param channel The channel number.
     * @param data The data received from the M4 core.
     */
    void receiveM4Data(uint32_t channel, const std::map<std::string, float>& data) override {
        std::cout << "Receiving M4 data for channel " << channel << std::endl;
        
        // Update the channel data table with new values
        for (const auto& pair : data) {
            channelDataTable[channel][pair.first] = pair.second;
        }
    }
    
};

#endif