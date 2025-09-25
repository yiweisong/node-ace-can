import path from 'path';

const loadNativeBinding: (dir?: string) => NativeModule = require('node-gyp-build');

export type Bustype = 'busust' | 'pcan';

export interface CANMessage {
  id: number;
  data: Buffer;
}

export interface CANError {
  code: number;
  message: string;
}

export type MessageListener = (message: CANMessage) => void;
export type ErrorListener = (error: CANError) => void;
export type CloseListener = () => void;

interface NativeModule {
  CANBus: NativeCANBusConstructor;
}

interface NativeCANBusConstructor {
  new (channel: number, bustype: Bustype, bitrate: number): NativeCANBusInstance;
  isAvailable(bustype: Bustype): boolean;
}

interface NativeCANBusInstance {
  send(message: CANMessage): void;
  on(event: 'message', listener: MessageListener): void;
  on(event: 'error', listener: ErrorListener): void;
  on(event: 'close', listener: CloseListener): void;
  close(): void;
}

const native: NativeModule = loadNativeBinding(path.resolve(__dirname, '..'));
const { CANBus: NativeCANBus } = native;

export class CANBus {
  private readonly native: NativeCANBusInstance;

  constructor(channel: number, bustype: Bustype, bitrate: number) {
    this.native = new NativeCANBus(channel, bustype, bitrate);
  }

  send(message: CANMessage): void {
    this.native.send(message);
  }

  on(event: 'message', listener: MessageListener): this;
  on(event: 'error', listener: ErrorListener): this;
  on(event: 'close', listener: CloseListener): this;
  on(event: 'message' | 'error' | 'close', listener: MessageListener | ErrorListener | CloseListener): this {
    (this.native as unknown as { on(event: string, listener: (...args: unknown[]) => void): void }).on(
      event,
      listener as unknown as (...args: unknown[]) => void,
    );
    return this;
  }

  close(): void {
    this.native.close();
  }

  static isAvailable(bustype: Bustype): boolean {
    return NativeCANBus.isAvailable(bustype);
  }
}

export function isAvailable(bustype: Bustype): boolean {
  return CANBus.isAvailable(bustype);
}
