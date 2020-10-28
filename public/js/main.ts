import { TetrahedronGeometry } from 'three';

import * as THREE from 'three';

const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CHARACTERISTIC_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const SERVICE_NAME = 'm5-stack';

const scene: THREE.Scene = new THREE.Scene();

const camera = new THREE.PerspectiveCamera(75, 1920 / 1080, 1, 10000);
camera.position.z = 1000;

const geometry = new THREE.BoxGeometry(200, 200, 600);
const material = new THREE.MeshBasicMaterial({ color: 0xff0000, wireframe: true });

const box = new THREE.Mesh(geometry, material);

const mesh = new THREE.Group();
mesh.add(box);
box.scale.z = 1;
box.position.z -= 300;

scene.add(mesh);
// mesh.rotation.order = 'YXZ';

const renderer = new THREE.WebGLRenderer();
renderer.setSize(1920, 1080);

document.body.appendChild(renderer.domElement);

let accX = 0;
let accY = 0;
let accZ = 0;

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
      const decoded = JSON.parse(decoder.decode(value));
      // mesh.rotation.set(decoded.pitch/180 * Math.PI, decoded.roll / 180 * Math.PI, 0);
      accX = accX * 0.97 + decoded.x * 0.03;
      accY = accY * 0.97 + decoded.y * 0.03;
      accZ = accZ * 0.97 + decoded.z * 0.03;
      document.getElementById('log').textContent = `${accX}, ${accY}, ${accZ}`;
      mesh.lookAt(accX, -accZ, -accY);
      renderer.render(scene, camera);
    });
  } catch (error) {
    console.error(error);
  }

  setTimeout(() => {
    if (device.gatt.connected) {
      device.gatt.disconnect();
      document.getElementById('log').textContent = 'disconnected';
    }
  }, 600 * 1000);
};

// setup
document.getElementById('btn')
  .addEventListener('click', onButtonDown);
