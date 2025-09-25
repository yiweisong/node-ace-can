# 一个类似python-can的CAN总线库
## 目标
- 提供一个类似python-can的CAN总线库，支持多种硬件接口。目前计划支持的接口包括：
  - busmust
  - Peak PCAN

## 技术栈
- nodejs
- node-gyp
- node-addon-api
- C/C++

## 目标平台
- Windows
- Linux（待定）
- MacOS（待定）

## 头文件和库文件
位于 deps 目录下
- busmust
  - 头文件：`deps/busmust/include`
  - 库文件：`deps/busmust/lib`
- Peak PCAN
  - 头文件：`deps/peak/include`
  - 库文件：`deps/peak/lib`

## nodejs 暴露的接口
- `CANBus` 类
  - 构造函数 `constructor(channel, bustype, bitrate)`：初始化CAN总线接口
    - `channel`：CAN通道号
    - `bustype`：总线类型（如 'busmust' 或 'pcan'）
    - `bitrate`：波特率
  - 方法 `send(message)`：发送CAN消息
    - `message`：包含ID和数据的对象
  - 方法 `on(event, callback)`：监听CAN消息接收事件
    - `event`：事件名称（如 'message','error','close'）
    - `callback`：回调函数，接收消息对象作为参数
  - 方法 `close()`：关闭CAN总线接口

- `isAvailable(bustype)` 静态方法：检查指定的总线类型是否可用
  - `bustype`：总线类型（如 'busmust' 或 'pcan'）
  - 返回布尔值，表示总线类型是否可用

## 发布
- 使用 `semantic-release`, `prebuildify`
- 发布到 npm，包名暂定为 `ace-can`