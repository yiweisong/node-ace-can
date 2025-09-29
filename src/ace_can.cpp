#include "ace_can.h"
#include <napi.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

#ifdef _WIN32
// #define NOMINMAX
#include <windows.h>
#else
#include <poll.h>
#include <unistd.h>
#endif

extern "C" {
#include "bmapi.h"
}

#ifndef __stdcall
#define __stdcall
#endif
#if defined(__linux__) || defined(__APPLE__)
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

constexpr uint16_t kBusmustLanguageEnglish = 0x09;
constexpr WORD kPcanLanguageEnglish = 0x09;

std::atomic<int> g_busmust_instance_count{0};

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

std::string BusmustStatusToString(BM_StatusTypeDef status) {
    char buffer[256] = {0};
    BM_GetErrorText(status, buffer, sizeof(buffer), kBusmustLanguageEnglish);
    if (buffer[0] != '\0') {
        return std::string(buffer);
    }
    std::ostringstream oss;
    oss << "BM error 0x" << std::hex << std::uppercase << status;
    return oss.str();
}

bool BuildBusmustBitrate(int bitrate, BM_BitrateTypeDef& out) {
    if (bitrate <= 0 || (bitrate % 1000) != 0) {
        return false;
    }
    std::memset(&out, 0, sizeof(out));
    out.nbitrate = static_cast<uint16_t>(bitrate / 1000);
    out.nsamplepos = 75; // default sample point in percent
    out.dsamplepos = 75;
    return out.nbitrate != 0;
}

bool BusmustSupportsCan(const BM_ChannelInfoTypeDef& info) {
    return (info.cap & (BM_CAN_CAP | BM_CAN_FD_CAP)) != 0;
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
    std::string requestedBustype = info[1].As<Napi::String>().Utf8Value();
    std::transform(requestedBustype.begin(), requestedBustype.end(), requestedBustype.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (requestedBustype == "busust") {
        requestedBustype = "busmust";
    }
    bustype_ = requestedBustype;
    bitrate_ = info[2].As<Napi::Number>().Int32Value();
    handle_ = nullptr;
    notification_handle_ = nullptr;
    pcan_handle_ = PCAN_NONEBUS;
    is_open_ = false;

    if (bustype_ == "busmust") {
        if (channel_ < 0) {
            Napi::TypeError::New(env, "Busmust channel must be >= 0").ThrowAsJavaScriptException();
            return;
        }

        auto cleanup_and_throw = [&](const std::string& message) {
            if (handle_) {
                BM_Close(static_cast<BM_ChannelHandle>(handle_));
                handle_ = nullptr;
            }
            if (busmust_registered_) {
                if (g_busmust_instance_count.fetch_sub(1) == 1) {
                    BM_UnInit();
                }
                busmust_registered_ = false;
            }
            if (!message.empty()) {
                Napi::Error::New(env, message).ThrowAsJavaScriptException();
            }
        };

        if (g_busmust_instance_count.fetch_add(1) == 0) {
            BM_StatusTypeDef initStatus = BM_Init();
            if (initStatus != BM_ERROR_OK) {
                g_busmust_instance_count.fetch_sub(1);
                Napi::Error::New(env, "BM_Init failed: " + BusmustStatusToString(initStatus)).ThrowAsJavaScriptException();
                return;
            }
        }
        busmust_registered_ = true;

        BM_BitrateTypeDef bitrateConfig{};
        if (!BuildBusmustBitrate(bitrate_, bitrateConfig)) {
            cleanup_and_throw("Unsupported Busmust bitrate (must be multiple of 1 kbps)");
            return;
        }

        std::vector<BM_ChannelInfoTypeDef> channels;
        channels.reserve(16);
        BM_StatusTypeDef status = BM_ERROR_OK;
        int enumerated = 0;
        bool enumeration_success = false;
        for (int attempt = 0; attempt < 4; ++attempt) {
            int capacity = static_cast<int>(channels.capacity());
            if (capacity == 0) {
                capacity = 16;
                channels.reserve(capacity);
            }
            channels.assign(capacity, {});
            enumerated = capacity;
            status = BM_Enumerate(channels.data(), &enumerated);
            if (status != BM_ERROR_OK) {
                break;
            }
            if (enumerated <= capacity) {
                channels.resize(enumerated);
                enumeration_success = true;
                break;
            }
            channels.reserve(capacity * 2);
        }

        if (status != BM_ERROR_OK) {
            cleanup_and_throw("BM_Enumerate failed: " + BusmustStatusToString(status));
            return;
        }

        if (!enumeration_success) {
            cleanup_and_throw("BM_Enumerate ran out of buffer space");
            return;
        }

        if (channels.empty()) {
            cleanup_and_throw("No Busmust channels detected");
            return;
        }

        if (channel_ >= static_cast<int>(channels.size())) {
            cleanup_and_throw("Busmust channel index out of range");
            return;
        }

        BM_ChannelInfoTypeDef channelInfo = channels[channel_];
        if (!BusmustSupportsCan(channelInfo)) {
            cleanup_and_throw("Selected Busmust channel does not support CAN");
            return;
        }

        BM_ChannelHandle openedHandle = nullptr;
        status = BM_OpenEx(
            &openedHandle,
            &channelInfo,
            BM_CAN_NORMAL_MODE,
            BM_TRESISTOR_120,
            &bitrateConfig,
            nullptr,
            0);
        if (status != BM_ERROR_OK || openedHandle == nullptr) {
            cleanup_and_throw("BM_OpenEx failed: " + BusmustStatusToString(status));
            return;
        }

        handle_ = openedHandle;

        BM_NotificationHandle notification = nullptr;
        status = BM_GetNotification(openedHandle, &notification);
        if (status != BM_ERROR_OK || notification == nullptr) {
            cleanup_and_throw("BM_GetNotification failed: " + BusmustStatusToString(status));
            return;
        }
        notification_handle_ = notification;

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
#ifdef _WIN32
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (eventHandle != nullptr) {
            HANDLE value = eventHandle;
            TPCANStatus eventStatus = CAN_SetValue(pcan_handle_, PCAN_RECEIVE_EVENT, &value, static_cast<DWORD>(sizeof(value)));
            if (eventStatus == PCAN_ERROR_OK) {
                pcan_event_handle_ = eventHandle;
            } else {
                CloseHandle(eventHandle);
            }
        }
#else
        int eventFd = -1;
        TPCANStatus eventStatus = CAN_GetValue(pcan_handle_, PCAN_RECEIVE_EVENT, &eventFd, static_cast<DWORD>(sizeof(eventFd)));
        if (eventStatus == PCAN_ERROR_OK && eventFd >= 0) {
            pcan_event_fd_ = eventFd;
        }
#endif
        is_open_ = true;
    } else {
        Napi::Error::New(env, "Unsupported bustype: " + bustype_).ThrowAsJavaScriptException();
    }
}

CANBus::~CANBus() {
    StopReceiveThread();

    if (bustype_ == "busmust") {
        if (handle_) {
            BM_Close(static_cast<BM_ChannelHandle>(handle_));
            handle_ = nullptr;
        }
        notification_handle_ = nullptr;
        if (busmust_registered_) {
            if (g_busmust_instance_count.fetch_sub(1) == 1) {
                BM_UnInit();
            }
            busmust_registered_ = false;
        }
    } else if (bustype_ == "pcan") {
        DetachPcanEvent();
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

    if (bustype_ == "busmust") {
        if (!handle_) {
            Napi::Error::New(env, "Busmust handle not open").ThrowAsJavaScriptException();
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
        BM_StatusTypeDef status = BM_WriteCanMessage(static_cast<BM_ChannelHandle>(handle_), &msg, 0, 100, &timestamp);
        if (status != BM_ERROR_OK) {
            std::string reason = BusmustStatusToString(status);
            EmitError(static_cast<int>(status), reason);
            Napi::Error::New(env, "BM_WriteCanMessage failed: " + reason).ThrowAsJavaScriptException();
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

            if (bustype_ == "busmust") {
                auto channelHandle = static_cast<BM_ChannelHandle>(handle_);
                if (!channelHandle) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    continue;
                }

                if (notification_handle_) {
                    BM_NotificationHandle handles[1] = { static_cast<BM_NotificationHandle>(notification_handle_) };
                    int waitResult = BM_WaitForNotifications(handles, 1, 50);
                    if (waitResult < 0) {
                        continue;
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }

                while (recv_running_) {
                    BM_CanMessageTypeDef msg = {};
                    uint32_t channel = 0;
                    uint32_t timestamp = 0;
                    BM_StatusTypeDef status = BM_ReadCanMessage(channelHandle, &msg, &channel, &timestamp);
                    if (status == BM_ERROR_OK) {
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
                                recv_running_ = false;
                                break;
                            }
                        }
                    } else if (status == BM_ERROR_QRCVEMPTY) {
                        break;
                    } else {
                        EmitError(static_cast<int>(status), BusmustStatusToString(status));
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        break;
                    }
                }
            } else if (bustype_ == "pcan") {
                if (pcan_handle_ == PCAN_NONEBUS) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    continue;
                }
                bool ready = (pcan_event_handle_ == nullptr && pcan_event_fd_ < 0);
#ifdef _WIN32
                if (pcan_event_handle_) {
                    HANDLE waitHandle = static_cast<HANDLE>(pcan_event_handle_);
                    DWORD waitResult = WaitForSingleObject(waitHandle, 50);
                    if (waitResult == WAIT_OBJECT_0) {
                        ready = true;
                    } else if (waitResult == WAIT_TIMEOUT) {
                        ready = false;
                    } else {
                        DWORD lastError = (waitResult == WAIT_FAILED) ? GetLastError() : waitResult;
                        EmitError(static_cast<int>(lastError), "PCAN receive event wait failed");
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        ready = false;
                    }
                }
#else
                if (pcan_event_fd_ >= 0) {
                    struct pollfd pfd;
                    std::memset(&pfd, 0, sizeof(pfd));
                    pfd.fd = pcan_event_fd_;
                    pfd.events = POLLIN;
                    int pollResult = poll(&pfd, 1, 50);
                    if (pollResult > 0 && (pfd.revents & POLLIN) != 0) {
                        ready = true;
                    } else if (pollResult == 0 || (pollResult < 0 && errno == EINTR)) {
                        ready = false;
                    } else if (pollResult < 0) {
                        EmitError(errno, "PCAN receive event poll failed");
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        ready = false;
                    }
                }
#endif
                if (!ready) {
                    continue;
                }

                while (recv_running_) {
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
                                recv_running_ = false;
                                break;
                            }
                        }
                    } else if (status == PCAN_ERROR_QRCVEMPTY) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;
                    } else {
                        EmitError(static_cast<int>(status), PcanStatusToString(status));
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        break;
                    }
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    });
}

void CANBus::StopReceiveThread() {
    recv_running_ = false;
#ifdef _WIN32
    if (bustype_ == "pcan" && pcan_event_handle_) {
        SetEvent(static_cast<HANDLE>(pcan_event_handle_));
    }
#endif
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

void CANBus::DetachPcanEvent() {
#ifdef _WIN32
    if (pcan_event_handle_) {
        if (pcan_handle_ != PCAN_NONEBUS) {
            HANDLE nullHandle = nullptr;
            CAN_SetValue(pcan_handle_, PCAN_RECEIVE_EVENT, &nullHandle, static_cast<DWORD>(sizeof(nullHandle)));
        }
        CloseHandle(static_cast<HANDLE>(pcan_event_handle_));
        pcan_event_handle_ = nullptr;
    }
#else
    if (pcan_event_fd_ >= 0) {
        pcan_event_fd_ = -1;
    }
#endif
}

Napi::Value CANBus::Close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    StopReceiveThread();
    if (!is_open_) {
        return env.Undefined();
    }

    if (bustype_ == "busmust") {
        if (handle_) {
            BM_Close(static_cast<BM_ChannelHandle>(handle_));
            handle_ = nullptr;
        }
        notification_handle_ = nullptr;
        if (busmust_registered_) {
            if (g_busmust_instance_count.fetch_sub(1) == 1) {
                BM_UnInit();
            }
            busmust_registered_ = false;
        }
    } else if (bustype_ == "pcan") {
        DetachPcanEvent();
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
    std::transform(bustype.begin(), bustype.end(), bustype.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (bustype == "busust") {
        bustype = "busmust";
    }
    bool available = (bustype == "busmust" || bustype == "pcan");
    return Napi::Boolean::New(env, available);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return CANBus::Init(env, exports);
}

NODE_API_MODULE(ace_can, InitAll)
