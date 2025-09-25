# ace-can

> Node.js CAN bus bindings for Busust and Peak PCAN hardware.

## Getting started

```bash
npm install --ignore-scripts
npm run build:ts
```

The `--ignore-scripts` flag avoids compiling the native addon when the vendor
SDK libraries are not available on your machine. Once you configure the hardware
SDKs, you can rebuild the native module with:

```bash
npm run build:native
```

## API smoke tests

The repository now includes a small suite of Node.js tests that exercise the
TypeScript wrapper around the native addon. The tests inject a mocked native
binding so they do **not** require real hardware.

```bash
npm test
```

What the tests cover:

- Argument forwarding from the JavaScript `CANBus` class to the native
  constructor.
- `send()` delegation to the native instance.
- Listener wiring for `message` and `close` events.
- Static helper `CANBus.isAvailable()` and top-level `isAvailable()` utility.

Because the tests mock the native layer, they are suitable for CI environments
that lack Busust or PCAN hardware. For end-to-end validation against actual
interfaces, set `ACE_CAN_CHANNEL`, `ACE_CAN_BUSTYPE`, and `ACE_CAN_BITRATE`
environment variables in your own integration scripts and exercise the
real hardware using the same API shown in `test/canbus-wrapper.test.cjs`.
