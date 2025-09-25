#include "ace_can.h"
#include <napi.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <thread>

extern "C" {
#include "bmapi.h"
}

#if defined(__linux__) || defined(__APPLE__)
#ifndef __stdcall
#define __stdcall
#endif
#ifndef BYTE
using BYTE = unsigned char;
#endif
#ifndef WORD
using WORD = unsigned short;
#endif
#ifndef DWORD
using DWORD = unsigned int;
#endif
#ifndef UINT64
using UINT64 = std::uint64_t;
#endif
#ifndef LPSTR
using LPSTR = char*;
#endif
#endif

#include "PCANBasic.h"

namespace {

constexpr WORD kPcanLanguageEnglish = 0x09;

TPCANBaudrate MapPcanBaudrate(int bitrate) {
    switch (bitrate) {
        case 1000000: return PCAN_BAUD_1M;
        case 800000: return PCAN_BAUD_800K;
        case 500000: return PCAN_BAUD_500K;
        case 250000: return PCAN_BAUD_250K;
        case 125000: return PCAN_BAUD_125K;
        case 100000: return PCAN_BAUD_100K;
        case 95000: return PCAN_BAUD_95K;
        case 83333: return PCAN_BAUD_83K;
        case 50000: return PCAN_BAUD_50K;
        case 47619: return PCAN_BAUD_47K;
        case 33333: return PCAN_BAUD_33K;
        case 20000: return PCAN_BAUD_20K;
        case 10000: return PCAN_BAUD_10K;
        case 5000: return PCAN_BAUD_5K;
        default: return 0;
    }
}

TPCANHandle ResolvePcanChannelHandle(int channel) {
    if (channel >= 0x20) {
        return static_cast<TPCANHandle>(channel);
    }
    if (channel >= 1 && channel <= 16) {
        return static_cast<TPCANHandle>(PCAN_USBBUS1 + (channel - 1));
    }
    return PCAN_NONEBUS;
}

std::string PcanStatusToString(TPCANStatus status) {
    char buffer[256] = {0};
    if (CAN_GetErrorText(status, kPcanLanguageEnglish, buffer) == PCAN_ERROR_OK) {
        return std::string(buffer);
    }
    std::ostringstream oss;
    oss << "PCAN error 0x" << std::hex << std::uppercase << status;
    return oss.str();
}

} // namespace

Napi::Object CANBus::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "CANBus", {
        InstanceMethod("send", &CANBus::Send),
        InstanceMethod("on", &CANBus::On),
        InstanceMethod("close", &CANBus::Close),
        StaticMethod("isAvailable", &CANBus::IsAvailable)
    });
    exports.Set("CANBus", func);
    return exports;
}

CANBus::CANBus(const Napi::CallbackInfo& info) : Napi::ObjectWrap<CANBus>(info) {
    Napi::Env env = info.Env();
    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Expected channel, bustype, bitrate").ThrowAsJavaScriptException();
        return;
    }

    channel_ = info[0].As<Napi::Number>().Int32Value();
    bustype_ = info[1].As<Napi::String>().Utf8Value();
    bitrate_ = info[2].As<Napi::Number>().Int32Value();
    handle_ = nullptr;
    pcan_handle_ = PCAN_NONEBUS;
    is_open_ = false;

    if (bustype_ == "busust") {
        BM_StatusTypeDef status = BM_Init();
        if (status != 0) {
            Napi::Error::New(env, "BM_Init failed").ThrowAsJavaScriptException();
            return;
        }
        handle_ = BM_OpenCan(static_cast<uint16_t>(channel_));
        if (!handle_) {
            BM_UnInit();
            Napi::Error::New(env, "BM_OpenCan failed").ThrowAsJavaScriptException();
            return;
        }
        is_open_ = true;
    } else if (bustype_ == "pcan") {
        TPCANHandle resolved = ResolvePcanChannelHandle(channel_);
        if (resolved == PCAN_NONEBUS) {
            Napi::Error::New(env, "Invalid PCAN channel").ThrowAsJavaScriptException();
            return;
        }
        TPCANBaudrate baud = MapPcanBaudrate(bitrate_);
        if (baud == 0) {
            Napi::Error::New(env, "Unsupported PCAN bitrate").ThrowAsJavaScriptException();
            return;
        }
        TPCANStatus status = CAN_Initialize(resolved, baud, 0, 0, 0);
        if (status != PCAN_ERROR_OK) {
            Napi::Error::New(env, "CAN_Initialize failed: " + PcanStatusToString(status)).ThrowAsJavaScriptException();
            return;
        }
        pcan_handle_ = resolved;
        is_open_ = true;
    } else {
        Napi::Error::New(env, "Unsupported bustype: " + bustype_).ThrowAsJavaScriptException();
    }
}

CANBus::~CANBus() {
    StopReceiveThread();
    if (!is_open_) {
        return;
    }
    if (bustype_ == "busust") {
        if (handle_) {
            BM_Close(handle_);
            handle_ = nullptr;
        }
        BM_UnInit();
    } else if (bustype_ == "pcan") {
        if (pcan_handle_ != PCAN_NONEBUS) {
            CAN_Uninitialize(pcan_handle_);
            pcan_handle_ = PCAN_NONEBUS;
        }
    }
    is_open_ = false;
}

Napi::Value CANBus::Send(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!is_open_) {
        Napi::Error::New(env, "CANBus not open").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected message object").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    Napi::Object msgObj = info[0].As<Napi::Object>();
    if (!msgObj.Has("id") || !msgObj.Get("id").IsNumber()) {
        Napi::TypeError::New(env, "Message.id must be a number").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    if (!msgObj.Has("data") || !msgObj.Get("data").IsBuffer()) {
        Napi::TypeError::New(env, "Message.data must be a Buffer").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    uint32_t id = msgObj.Get("id").As<Napi::Number>().Uint32Value();
    Napi::Buffer<uint8_t> dataBuf = msgObj.Get("data").As<Napi::Buffer<uint8_t>>();

    if (bustype_ == "busust") {
        if (!handle_) {
            Napi::Error::New(env, "busust handle not open").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        size_t dlc = std::min<size_t>(dataBuf.Length(), 64);
        BM_CanMessageTypeDef msg = {};
        if (id <= 0x7FF) {
            BM_SET_STD_MSG_ID(msg.id, id);
            msg.ctrl.tx.IDE = 0;
        } else {
            BM_SET_EXT_MSG_ID(msg.id, id);
            msg.ctrl.tx.IDE = 1;
        }
        msg.ctrl.tx.DLC = dlc;
        msg.ctrl.tx.RTR = 0;
        msg.ctrl.tx.FDF = 0;
        msg.ctrl.tx.BRS = 0;
        msg.ctrl.tx.ESI = 0;
        std::memcpy(msg.payload, dataBuf.Data(), dlc);

        uint32_t timestamp = 0;
        BM_StatusTypeDef status = BM_WriteCanMessage(handle_, &msg, 0, 100, &timestamp);
        if (status != 0) {
            EmitError(static_cast<int>(status), "BM_WriteCanMessage failed");
            Napi::Error::New(env, "BM_WriteCanMessage failed").ThrowAsJavaScriptException();
        }
    } else if (bustype_ == "pcan") {
        if (pcan_handle_ == PCAN_NONEBUS) {
            Napi::Error::New(env, "PCAN channel not open").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        size_t dlc = std::min<size_t>(dataBuf.Length(), 8);
        TPCANMsg msg = {};
        msg.ID = id;
        msg.MSGTYPE = (id > 0x7FF) ? PCAN_MESSAGE_EXTENDED : PCAN_MESSAGE_STANDARD;
        msg.LEN = static_cast<BYTE>(dlc);
        std::memcpy(msg.DATA, dataBuf.Data(), dlc);

        TPCANStatus status = CAN_Write(pcan_handle_, &msg);
        if (status != PCAN_ERROR_OK) {
            std::string reason = PcanStatusToString(status);
            EmitError(static_cast<int>(status), reason);
            Napi::Error::New(env, "CAN_Write failed: " + reason).ThrowAsJavaScriptException();
        }
    } else {
        Napi::Error::New(env, "Unsupported bustype: " + bustype_).ThrowAsJavaScriptException();
    }

    return env.Undefined();
}

Napi::Value CANBus::On(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsFunction()) {
        Napi::TypeError::New(env, "Expected (event, callback)").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string event = info[0].As<Napi::String>();
    Napi::Function cb = info[1].As<Napi::Function>();

    if (event == "message") {
        if (tsfn_message_) {
            Napi::Error::New(env, "Already listening for messages").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        tsfn_message_ = Napi::ThreadSafeFunction::New(env, cb, "CANBusOnMessage", 0, 1);
        StartReceiveThread();
    } else if (event == "error") {
        if (tsfn_error_) {
            Napi::Error::New(env, "Already listening for errors").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        tsfn_error_ = Napi::ThreadSafeFunction::New(env, cb, "CANBusOnError", 0, 1);
    } else if (event == "close") {
        if (tsfn_close_) {
            Napi::Error::New(env, "Already listening for close").ThrowAsJavaScriptException();
            return env.Undefined();
        }
        tsfn_close_ = Napi::ThreadSafeFunction::New(env, cb, "CANBusOnClose", 0, 1);
    } else {
        Napi::Error::New(env, "Only 'message', 'error', 'close' events supported").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    return env.Undefined();
}

void CANBus::StartReceiveThread() {
    if (recv_running_ || !is_open_) {
        return;
    }
    recv_running_ = true;
    recv_thread_ = std::thread([this]() {
        while (recv_running_) {
            if (!is_open_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            if (bustype_ == "busust") {
                if (!handle_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    continue;
                }
                BM_CanMessageTypeDef msg = {};
                uint32_t channel = 0;
                uint32_t timestamp = 0;
                BM_StatusTypeDef status = BM_ReadCanMessage(handle_, &msg, &channel, &timestamp);
                if (status == 0) {
                    if (tsfn_message_) {
                        auto callback = [msg](Napi::Env env, Napi::Function jsCallback) {
                            Napi::Object jsMsg = Napi::Object::New(env);
                            bool extended = msg.ctrl.rx.IDE != 0;
                            uint32_t canid = extended ? BM_GET_EXT_MSG_ID(msg.id) : BM_GET_STD_MSG_ID(msg.id);
                            size_t dlc = std::min<size_t>(msg.ctrl.rx.DLC, 64);
                            jsMsg.Set("id", Napi::Number::New(env, canid));
                            jsMsg.Set("data", Napi::Buffer<uint8_t>::Copy(env, msg.payload, dlc));
                            jsCallback.Call({jsMsg});
                        };
                        if (tsfn_message_.BlockingCall(callback) != napi_ok) {
                            break;
                        }
                    }
                } else if (status == BM_ERROR_QRCVEMPTY) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                } else {
                    EmitError(static_cast<int>(status), "BM_ReadCanMessage error");
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            } else if (bustype_ == "pcan") {
                if (pcan_handle_ == PCAN_NONEBUS) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    continue;
                }
                TPCANMsg msg = {};
                TPCANStatus status = CAN_Read(pcan_handle_, &msg, nullptr);
                if (status == PCAN_ERROR_OK) {
                    if (tsfn_message_) {
                        auto callback = [msg](Napi::Env env, Napi::Function jsCallback) {
                            Napi::Object jsMsg = Napi::Object::New(env);
                            bool extended = (msg.MSGTYPE & PCAN_MESSAGE_EXTENDED) != 0;
                            uint32_t canid = msg.ID;
                            if (!extended) {
                                canid &= 0x7FF;
                            }
                            size_t dlc = std::min<size_t>(msg.LEN, 8);
                            jsMsg.Set("id", Napi::Number::New(env, canid));
                            jsMsg.Set("data", Napi::Buffer<uint8_t>::Copy(env, msg.DATA, dlc));
                            jsCallback.Call({jsMsg});
                        };
                        if (tsfn_message_.BlockingCall(callback) != napi_ok) {
                            break;
                        }
                    }
                } else if (status == PCAN_ERROR_QRCVEMPTY) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                } else {
                    EmitError(static_cast<int>(status), PcanStatusToString(status));
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    });
}

void CANBus::StopReceiveThread() {
    recv_running_ = false;
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
    if (tsfn_message_) {
        tsfn_message_.Release();
        tsfn_message_ = nullptr;
    }
    if (tsfn_error_) {
        tsfn_error_.Release();
        tsfn_error_ = nullptr;
    }
    if (tsfn_close_) {
        tsfn_close_.BlockingCall([](Napi::Env env, Napi::Function jsCallback) {
            jsCallback.Call({});
        });
        tsfn_close_.Release();
        tsfn_close_ = nullptr;
    }
}

void CANBus::EmitError(int code, const std::string& message) {
    if (!tsfn_error_) {
        return;
    }
    auto callback = [code, message](Napi::Env env, Napi::Function jsCallback) {
        Napi::Object errObj = Napi::Object::New(env);
        errObj.Set("code", Napi::Number::New(env, code));
        errObj.Set("message", Napi::String::New(env, message));
        jsCallback.Call({errObj});
    };
    tsfn_error_.BlockingCall(callback);
}

Napi::Value CANBus::Close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    StopReceiveThread();
    if (!is_open_) {
        return env.Undefined();
    }

    if (bustype_ == "busust") {
        if (handle_) {
            BM_Close(handle_);
            handle_ = nullptr;
        }
        BM_UnInit();
    } else if (bustype_ == "pcan") {
        if (pcan_handle_ != PCAN_NONEBUS) {
            CAN_Uninitialize(pcan_handle_);
            pcan_handle_ = PCAN_NONEBUS;
        }
    }
    is_open_ = false;
    return env.Undefined();
}

Napi::Value CANBus::IsAvailable(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1) {
        Napi::TypeError::New(env, "Expected bustype").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    std::string bustype = info[0].As<Napi::String>().Utf8Value();
    bool available = (bustype == "busust" || bustype == "pcan");
    return Napi::Boolean::New(env, available);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return CANBus::Init(env, exports);
}

NODE_API_MODULE(ace_can, InitAll)
