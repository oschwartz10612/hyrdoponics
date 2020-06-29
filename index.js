const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')
var mqtt = require('mqtt')

var client  = mqtt.connect('mqtt://192.168.1.183');

async function getPort(pnpId) {
    var ports = await SerialPort.list().catch(err => {
        console.error("Could not list ports: " + err);
        process.exit(1);
    });

    for (let i = 0; i < ports.length; i++) {
        const port = ports[i];
        if (pnpId == port.pnpId) {
            return port.path;
        }
    }
}

async function main() {

    var phpPort = await getPort('usb-FTDI_FT230X_Basic_UART_DO00MRHP-if00-port0');
    var arduinoPort = await getPort('');

    if (phpPort) {
        console.log('Initalizing ph port');
        const ph = new SerialPort(phpPort, { baudRate: 9600 }, err => {
            if (err) {
                console.error('Could not open port: ', err.message)
                process.exit(1);
            }
        })

        const phParser = ph.pipe(new Readline({ delimiter: '\r' }));

        phParser.on('data', data => {
            console.log(`Ph Value: ${data}`);
            client.publish('home-assistant/hydroponics/ph', data)
        });
    }

    if (arduinoPort) {
        const arduino = new SerialPort(arduinoPort, { baudRate: 9600 }, err => {
            if (err) {
                console.error('Could not open port: ', err.message)
                process.exit(1);
            }
        })
        
        const arduinoParser = arduino.pipe(new Readline());

        arduinoParser.on('data', data => {
            var sensorData = JSON.parse(data);

            console.log(sensorData);
            
            client.publish('home-assistant/hydroponics/ph', data)
        });
    }

}

main();

