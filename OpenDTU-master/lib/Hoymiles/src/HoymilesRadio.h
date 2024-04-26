// // SPDX-License-Identifier: GPL-2.0-or-later
// #pragma once

// #include "commands/CommandAbstract.h"
// #include "types.h"
// #include <ThreadSafeQueue.h>
// #include <TimeoutHelper.h>
// #include <memory>

// class HoymilesRadio {
// public:
//     serial_u DtuSerial() const;
//     virtual void setDtuSerial(const uint64_t serial);

//     bool isIdle() const;
//     bool isQueueEmpty() const;
//     bool isInitialized() const;

//     void enqueCommand(std::shared_ptr<CommandAbstract> cmd)
//     {
//         _commandQueue.push(cmd);
//     }

//     template <typename T>
//     std::shared_ptr<T> prepareCommand()
//     {
//         return std::make_shared<T>();
//     }

// protected:
//     static serial_u convertSerialToRadioId(const serial_u serial);
//     static void dumpBuf(const uint8_t buf[], const uint8_t len, const bool appendNewline = true);

//     bool checkFragmentCrc(const fragment_t& fragment) const;
//     virtual void sendEsbPacket(CommandAbstract& cmd) = 0;
//     void sendRetransmitPacket(const uint8_t fragment_id);
//     void sendLastPacketAgain();
//     void handleReceivedPackage();

//     serial_u _dtuSerial;
//     ThreadSafeQueue<std::shared_ptr<CommandAbstract>> _commandQueue;
//     bool _isInitialized = false;
//     bool _busyFlag = false;

//     TimeoutHelper _rxTimeout;
// };

// New HoymilesRadio.h without any dependcy

// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "commands/CommandAbstract.h"
#include "types.h"
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdint.h>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue<T>&) = delete;

    ThreadSafeQueue(ThreadSafeQueue<T>&& other) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue = std::move(other._queue);
    }

    virtual ~ThreadSafeQueue() {}

    unsigned long size() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return {};
        }
        T tmp = _queue.front();
        _queue.pop();
        return tmp;
    }

    void push(const T& item) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(item);
    }

    T front() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.front();
    }

private:
    std::queue<T> _queue;
    mutable std::mutex _mutex;
};

class TimeoutHelper {
public:
    TimeoutHelper() : startMillis(0), timeout(0) {}
    void set(const uint32_t ms) { timeout = ms; startMillis = getCurrentMillis(); }
    void extend(const uint32_t ms) { timeout += ms; }
    void reset() { startMillis = getCurrentMillis(); }
    bool occured() const { return (getCurrentMillis() - startMillis) >= timeout; }

private:
    uint32_t getCurrentMillis() const; // Placeholder for actual time function
    uint32_t startMillis;
    uint32_t timeout;
};

class HoymilesRadio {
public:
    serial_u DtuSerial() const;
    virtual void setDtuSerial(const uint64_t serial);

    bool isIdle() const;
    bool isQueueEmpty() const;
    bool isInitialized() const;

    void enqueCommand(std::shared_ptr<CommandAbstract> cmd) {
        _commandQueue.push(cmd);
    }

    template <typename T>
    std::shared_ptr<T> prepareCommand() {
        return std::make_shared<T>();
    }

protected:
    static serial_u convertSerialToRadioId(const serial_u serial);
    static void dumpBuf(const uint8_t buf[], const uint8_t len, const bool appendNewline = true);

    bool checkFragmentCrc(const fragment_t& fragment) const;
    virtual void sendEsbPacket(CommandAbstract& cmd) = 0;
    void sendRetransmitPacket(const uint8_t fragment_id);
    void sendLastPacketAgain();
    void handleReceivedPackage();

    serial_u _dtuSerial;
    ThreadSafeQueue<std::shared_ptr<CommandAbstract>> _commandQueue;
    bool _isInitialized = false;
    bool _busyFlag = false;

    TimeoutHelper _rxTimeout;
};