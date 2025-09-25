// ace_can.h/cpp will be compiled as native addon
// This file is for JS/TS type hints and usage

/**
 * @typedef {Object} CANMessage
 * @property {number} id - CAN message ID
 * @property {Buffer} data - CAN message data
 */

/**
 * @class CANBus
 * @param {number} channel
 * @param {string} bustype - 'busmust' | 'pcan'
 * @param {number} bitrate
 * @example
 *   const { CANBus } = require('ace-can');
 *   const can = new CANBus(0, 'busmust', 500000);
 */

/**
 * @method send
 * @param {CANMessage} message
 * @returns {void}
 */

/**
 * @method on
 * @param {'message'|'error'|'close'} event
 * @param {Function} callback
 * @returns {void}
 */

/**
 * @method close
 * @returns {void}
 */

/**
 * @static
 * @method isAvailable
 * @param {string} bustype
 * @returns {boolean}
 */
