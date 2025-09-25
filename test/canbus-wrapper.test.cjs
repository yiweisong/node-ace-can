'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const { EventEmitter } = require('node:events');

class FakeNativeCANBus extends EventEmitter {
  constructor(channel, bustype, bitrate) {
    super();
    this.channel = channel;
    this.bustype = bustype;
    this.bitrate = bitrate;
    this.sentMessages = [];
    FakeNativeCANBus.instances.push(this);
  }

  send(message) {
    this.sentMessages.push(message);
    this.emit('sent', message);
  }

  close() {
    this.emit('close');
  }
}

FakeNativeCANBus.instances = [];
FakeNativeCANBus.isAvailable = (bustype) => bustype === 'busmust';

const fakeNativeModule = { CANBus: FakeNativeCANBus };

const nodeGypBuildPath = require.resolve('node-gyp-build');
require.cache[nodeGypBuildPath] = {
  id: nodeGypBuildPath,
  filename: nodeGypBuildPath,
  loaded: true,
  exports: () => fakeNativeModule,
};

const { CANBus, isAvailable } = require('../dist');

test.beforeEach(() => {
  FakeNativeCANBus.instances = [];
});

test('CANBus constructor passes parameters to native binding', () => {
  const bus = new CANBus(1, 'busmust', 500000);
  assert.equal(FakeNativeCANBus.instances.length, 1);
  const native = FakeNativeCANBus.instances[0];
  assert.equal(native.channel, 1);
  assert.equal(native.bustype, 'busmust');
  assert.equal(native.bitrate, 500000);
  bus.close();
});

test('CANBus forwards send calls to the native instance', () => {
  const bus = new CANBus(1, 'busmust', 500000);
  const payload = Buffer.from([0xde, 0xad, 0xbe, 0xef]);
  bus.send({ id: 0x123, data: payload });
  const native = FakeNativeCANBus.instances[0];
  assert.equal(native.sentMessages.length, 1);
  assert.deepEqual(native.sentMessages[0], { id: 0x123, data: payload });
  bus.close();
});

test('CANBus subscribes to message events from the native layer', async () => {
  const bus = new CANBus(2, 'busmust', 250000);
  const native = FakeNativeCANBus.instances[0];

  const messagePromise = new Promise((resolve) => {
    bus.on('message', resolve);
  });

  const sample = { id: 0x456, data: Buffer.from([1, 2, 3]) };
  native.emit('message', sample);

  const received = await messagePromise;
  assert.deepEqual(received, sample);
  bus.close();
});

test('CANBus exposes static and top-level availability checks', () => {
  assert.equal(CANBus.isAvailable('busmust'), true);
  assert.equal(CANBus.isAvailable('pcan'), false);
  assert.equal(isAvailable('busmust'), true);
});

test('close listeners run when native layer emits close', async () => {
  const bus = new CANBus(3, 'busmust', 125000);
  const native = FakeNativeCANBus.instances[0];

  const closePromise = new Promise((resolve) => {
    bus.on('close', resolve);
  });

  native.emit('close');
  await closePromise;
  bus.close();
});
