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
        console.log(port);
        if (pnpId == port.pnpId) {
            return port.path;
        }
    }
}

async function main() {

    var phpPort = await getPort('usb-FTDI_FT230X_Basic_UART_DO00MRHP-if00-port0');
    var arduinoPort = await getPort('usb-1a86_USB2.0-Serial-if00-port0');

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
            client.publish('home-assistant/hydroponics/phStatus', 'online');
        });

        ph.on('close', () => {
            client.publish('home-assistant/hydroponics/phStatus', 'offline');
            console.log(`Ph connection lost.`);
        });
    }

    if (arduinoPort) {
        console.log('Initalizing arduino port');
        const arduino = new SerialPort(arduinoPort, { baudRate: 9600 }, err => {
            if (err) {
                console.error('Could not open port: ', err.message)
                process.exit(1);
            }
        })
        
        const arduinoParser = arduino.pipe(new Readline());

        arduinoParser.on('data', data => {
            var sensorData = JSON.parse(data);

            console.log(`Arduino Data: ${data}`);
            
            client.publish('home-assistant/hydroponics/ec', sensorData.ec.toString());
            client.publish('home-assistant/hydroponics/ppm', sensorData.ppm.toString());
            client.publish('home-assistant/hydroponics/temperature', sensorData.temperature.toString());
            client.publish('home-assistant/hydroponics/arduinoStatus', 'online');
        });

        arduino.on('close', () => {
            client.publish('home-assistant/hydroponics/arduinoStatus', 'offline');
            console.log(`Arduino connection lost.`);
        });
    }

}

main();

