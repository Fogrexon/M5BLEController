const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CHARACTERISTIC_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const SERVICE_NAME = 'm5-stack';

const onButtonDown = async () => {
  let device = null;

  try {
    device = await navigator.bluetooth.requestDevice({
      filters: [
        { services: [SERVICE_UUID] },
        { name: [SERVICE_NAME] },
      ],
    });

    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    const characteristic = await service.getCharacteristics(CHARACTERISTIC_UUID);

    if (characteristic.length === 0) {
      console.error('Characteristic UUIDが存在しません');
      return;
    }

    const myCharacteristic = characteristic[0];

    const decoder = new TextDecoder('utf-8');
    // const encoder = new TextEncoder('utf-8');

    await myCharacteristic.startNotifications();
    myCharacteristic.addEventListener('characteristicvaluechanged', (e) => {
      const { value } = e.target;
      document.getElementById('log').textContent = `${decoder.decode(value)}\n`;
    });
  } catch (error) {
    console.error(error);
  }

  setTimeout(() => {
    if (device.gatt.connected) {
      device.gatt.disconnect();
      document.getElementById('log').textContent = 'disconnected';
    }
  }, 60 * 1000);
};

// setup
document.getElementById('btn')
  .addEventListener('click', onButtonDown);
