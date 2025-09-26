//console.log(process.cwd())
process.env.PATH = process.env.PATH + ";"+ process.cwd();
//console.log(process.env.PATH.split(';'))

function main() {
    const { CANBus, isAvailable } = require('../dist');
    
    const bus = new CANBus(0, 'busmust', 250000);
    console.log('CANBus instance created:', bus);
    console.log('isAvailable("busmust"):', isAvailable('busmust'));
    bus.on('message',(message)=>{
        console.log(message.data);
    })
    //bus.close();
}

main();