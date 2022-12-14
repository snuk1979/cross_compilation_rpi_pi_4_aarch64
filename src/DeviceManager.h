#ifndef __IDEVICE_MANAGER_H__
#define __IDEVICE_MANAGER_H__

#include <stdio.h>
#include <utility>
#include <mutex>

#include <SoapySDR/Types.hpp>
#include <SoapySDR/Constants.h>
#include <SoapySDR/Formats.h>

namespace device_manager
{
    class IDeviceManager
    {
    public:
        /**
         * @brief Founds devices and creates devices instance
         * @return true if devices are found and created, otherwise false.
         */
        virtual bool DeviceSearch() = 0;
        /**
         * @brief Set the baseband sample rate of the chain.
         * @param deviceNumber number device
         * @param direction the channel direction RX or TX
         * @param channel an available channel on the device
         * @param rate the sample rate in samples per second
         * @return true on success, otherwise false
         */
        virtual bool SetSampleRate(
            const int deviceNumber = 0,
            const int direction = SOAPY_SDR_RX,
            const size_t channel = 0u,
            const double rate = kMinSampleRate) = 0;
        /**
         * @brief Tune the center frequency of the specified element.
         *  - For RX, this specifies the down-conversion frequency.
         *  - For TX, this specifies the up-conversion frequency.
         *
         * Recommended names used to represent tunable components:
         *  - "CORR" - freq error correction in PPM
         *  - "RF" - frequency of the RF frontend
         *  - "BB" - frequency of the baseband DSP
         * @param deviceNumber number device
         * @param direction the channel direction RX or TX
         * @param channel an available channel on the device
         * @param name the name of a tunable element
         * @param frequency the center frequency in Hz
         * @param args optional tuner arguments
         * @return true on success, otherwise false
         */
        virtual bool SetFrequency(
            const int deviceNumber = 0,
            const int direction = SOAPY_SDR_RX,
            const size_t channel = 0u,
            const double value = kDefFrequency,
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs()) = 0;
        /**
         * @brief Setup, activate a stream, and start receiving data
         * @param deviceNumber number device
         * @param direction the channel direction (`SOAPY_SDR_RX` or `SOAPY_SDR_TX`)
         * @param format A string representing the desired buffer format in read/writeStream()
         * @parblock
         *
         * The first character selects the number type:
         *   - "C" means complex
         *   - "F" means floating point
         *   - "S" means signed integer
         *   - "U" means unsigned integer
         *
         * The type character is followed by the number of bits per number (complex is 2x this size per sample)
         *
         *  Example format strings:
         *   - "CF32" -  complex float32 (8 bytes per element)
         *   - "CS16" -  complex int16 (4 bytes per element)
         *   - "CS12" -  complex int12 (3 bytes per element)
         *   - "CS4" -  complex int4 (1 byte per element)
         *   - "S32" -  int32 (4 bytes per element)
         *   - "U8" -  uint8 (1 byte per element)
         *
         * @endparblock
         * @param channels a list of channels or empty for automatic.
         * @param args stream args or empty for defaults.
         */
        virtual bool StartStream(
            const int deviceNumber = 0,
            const int direction = SOAPY_SDR_RX,
            const std::string &format = SOAPY_SDR_CF32,
            const std::vector<size_t> &channels = std::vector<size_t>(),
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs()) = 0;
        /**
         * @brief Shutdown all streams
         */
        virtual void StopStreams() = 0;
        /**
         * @brief Returns the number of devices created of the
         * DeviceSearch function
         */
        virtual std::size_t GetCountDevice() const = 0;
        /**
         * @brief Query a dictionary of available device information.
         * This dictionary can any number of values like
         * vendor name, product name, revisions, serials...
         * This information can be displayed to the user
         * to help identify the instantiated device.
         * @param deviceNumber number device
         * @return SoapySDR::Kwargs
         */
        virtual SoapySDR::Kwargs GetHardwareInfo(const int deviceNumber = 0) const = 0;
        /**
         * @brief Prints like vendor name, product name, revisions, serials...
         * This information can be displayed to the user
         * to help identify the instantiated device.
         * @param deviceNumber number device
         */
        virtual void PrintDeviceInfo(const int deviceNumber) const = 0;
        /**
         * @brief Prints like antennas, gains, frequency ranges info
         * @param deviceNumber number device
         */
        virtual void PrintDeviceSettings(const int deviceNumber) const = 0;
        /**
         * @brief Dtor
         */
        virtual ~IDeviceManager() {}

        template <typename Fn, class IDMO, typename... Arg, class Lock>
        auto CallThreadSafe(Lock &lock, IDMO obj, Fn fn, Arg &&...args) const -> decltype((obj->*fn)(std::forward<Arg>(args)...))
        {
            std::lock_guard guard(lock);
            return (obj->*fn)(std::forward<Arg>(args)...);
        }

        static constexpr auto kMinSampleRate = 30.72e6;
        static constexpr auto kDefFrequency = 433e6;
    };

} // device_manager

#endif // __IDEVICE_MANAGER_H__
