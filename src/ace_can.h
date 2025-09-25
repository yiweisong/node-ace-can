#ifndef ACE_CAN_H
#define ACE_CAN_H

#include <napi.h>
#include <string>
#include <thread>
#include <atomic>
#include <cstdint>

class CANBus : public Napi::ObjectWrap<CANBus> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    CANBus(const Napi::CallbackInfo& info);
    ~CANBus();

    Napi::Value Send(const Napi::CallbackInfo& info);
    Napi::Value On(const Napi::CallbackInfo& info);
    Napi::Value Close(const Napi::CallbackInfo& info);

    static Napi::Value IsAvailable(const Napi::CallbackInfo& info);

private:
    std::string bustype_;
    int channel_;
    int bitrate_;
    void* handle_ = nullptr; // BM_ChannelHandle for busmust
    uint16_t pcan_handle_ = 0; // PCAN channel handle
    bool is_open_ = false;

    // --- 事件接收相关 ---
    void StartReceiveThread();
    void StopReceiveThread();
    void EmitError(int code, const std::string& message);

    std::thread recv_thread_;
    std::atomic<bool> recv_running_{false};
    Napi::ThreadSafeFunction tsfn_message_;
    Napi::ThreadSafeFunction tsfn_error_;
    Napi::ThreadSafeFunction tsfn_close_;
};

#endif // ACE_CAN_H
