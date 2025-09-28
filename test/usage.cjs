process.env.PATH = process.env.PATH + ";" + process.cwd();

function main() {
    const { CANBus, isAvailable } = require('../dist');

    const bus = new CANBus(0, 'busmust', 250000);
    console.log('CANBus instance created:', bus);
    console.log('isAvailable("busmust"):', isAvailable('busmust'));
    bus.on('message', (message) => {
        console.log(message.data);
    })

    // 监听 Ctrl+C (SIGINT) 信号
    process.on('SIGINT', async () => {
        bus.close();
    });

    // 监听终止信号 (SIGTERM)，如关闭终端窗口
    process.on('SIGTERM', async () => {
        bus.close();
    });
}

main();
