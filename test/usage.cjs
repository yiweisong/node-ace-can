const { CANBus, isAvailable } = require('../dist');

function main() {
    const bus = new CANBus(0, 'busmust', 250000);
    console.log('CANBus instance created:', bus);
    console.log('isAvailable("busmust"):', isAvailable('busmust'));
    bus.close();
}

main();