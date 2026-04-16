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
    CANManager(CAN &can) : _can(can) {
        // 受信割り込みを設定
        _can.attach(callback(this, &CANManager::on_can_interrupt), CAN::RxIrq);
    }

    // 各ライブラリのインスタンスを登録
    void add_receiver(CANReceiver *receiver) {
        _receivers.push_back(receiver);
    }

    // メインループや専用スレッドから定期的に呼び出す
    void dispatch_all() {
        CANMessage msg;
        // キューに溜まっているメッセージを全て処理
        while (_queue.pop(msg)) {
            for (auto *receiver : _receivers) {
                if (receiver->handle_message(msg)) {
                    break; // 一つのライブラリが処理したら次へ（必要に応じて継続も可）
                }
            }
        }
    }

private:
    // 割り込みハンドラ（ここではキューに入れるだけにするのが安全）
    void on_can_interrupt() {
        CANMessage msg;
        while (_can.read(msg)) {
            _queue.push(msg);
        }
    }

    CAN &_can;
    std::vector<CANReceiver*> _receivers;
    CircularBuffer<CANMessage, 32> _queue; // 割り込み安全なバッファ
};

#endif