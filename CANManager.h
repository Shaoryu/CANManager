#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include "mbed.h"
#include <vector>

// 受信側が実装するインターフェース
class CANReceiver {
public:
    virtual ~CANReceiver() {}
    // 自分に関係するIDなら処理してtrueを返す
    virtual bool handle_message(const CANMessage &msg) = 0;
};

class CANManager {
public:
    CANManager(CAN &can) : 
        _can(can), 
        _queue(32 * EVENTS_EVENT_SIZE),
        _rx_thread(osPriorityHigh) 
    {
        _rx_thread.start(callback(&_queue, &EventQueue::dispatch_forever));
        _can.attach(callback(this, &CANManager::on_can_interrupt), CAN::RxIrq);
    }

    void add_receiver(CANReceiver *receiver) {
        _receivers.push_back(receiver);
    }

private:
    void on_can_interrupt() {
        _queue.call(callback(this, &CANManager::process_can_message));
    }

    void process_can_message() {
        CANMessage msg;
        while (_can.read(msg)) {
            for (auto *receiver : _receivers) {
                if (receiver->handle_message(msg)) {
                    break;
                }
            }
        }
    }
    
    CAN &_can;
    std::vector<CANReceiver*> _receivers;
    EventQueue _queue;
    Thread _rx_thread;
};

#endif
