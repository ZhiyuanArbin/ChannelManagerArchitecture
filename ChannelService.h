#ifndef CHANNELSERVICE_H
#define CHANNELSERVICE_H

#include <cstdint>

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
};

/**
 * @brief Abstract base class for channel data services.
 *
 * This class defines the interface for reading and processing data from the hardware channels.
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
     * @brief Receives data from the M4 core through RPMsg.
     */
    virtual void getM4Data() = 0; // Receives data from M4 through RPMsg
};

#endif