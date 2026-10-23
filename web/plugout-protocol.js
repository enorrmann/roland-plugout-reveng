/*
 * plugout-protocol.js -- Roland PLUG-OUT installer protocol for the browser.
 *
 * JavaScript port of the parts of tools/plugout.py and tools/plugout_tool.py
 * that the install needs. It speaks the same USB-MIDI protocol as Roland's
 * JUNO-60 / JX-3P / SH-101 plug-ins (docs/PROTOCOL.md) through Web MIDI:
 *
 *   1. query the device (type, firmware, number of slots, contents of each slot),
 *   2. send the START record, the firmware data records and the END record.
 *
 * The patch bank (docs/PROTOCOL.md §6) is not sent: the page installs the
 * firmware only, as `plugout_tool.py install --no-patches` (docs/WEB_INSTALLER.md).
 *
 * No MIDI is sent until install() is called. Nothing here touches the DOM.
 *
 * Plain script (no modules) so the page works from any static folder or a
 * plain file:// open, where ES modules are blocked by CORS. Exposes
 * globalThis.Plugout. Works in Node too (used by the test in web/test/).
 */
(function (root) {
  'use strict';

  // ---------------------------------------------------------------------------
  // constants (values from docs/PROTOCOL.md)
  // ---------------------------------------------------------------------------

  const ROLAND = 0x41;
  const MODEL_PLUGOUT = [0x00, 0x00, 0x7e];
  const DT1 = 0x12;
  const DEFAULT_DEVICE_ID = 0x10;

  const DELTA_IV = 0xbacadabe;     // cipher seed of the Code*.Dat container
  const HEADER_SIZE = 0x80;
  const TYPE_NAME = 'PLUGOUT_SYS_INF';

  // Host writes go to 0x0E0xxxxx; the device answers on the same address with
  // bit 21 set (0x0E2xxxxx, "70 .." -> "71 ..").
  const REPLY_BIT = 0x00200000;
  const ADDR_START = 0x0e040000;       // data: slot | flags -> 1 byte, 0 = ok
  const ADDR_END = 0x0e040001;         // data: from the container -> 1 byte, 0 = ok
  const ADDR_DATA_CHUNK = 0x0e0c0009;  // "70 30 00 09", 586 bytes = 512 bytes 7-in-8
  const ADDR_DATA_FILL = 0x0e0c0089;   // "70 30 01 09", filler record
  const ADDR_FLOW = 0x0e0c0180;        // "70 30 03 00" -> 4 bytes
  const ADDR_DATA_ERROR = 0x0e2c0000;  // "71 30 00 00" = device rejected a record
  const ADDR_SLOT_INFO = 0x0e000080;   // data [slot] -> 0x30 bytes
  const ADDR_DEVICE_TYPE = 0x0e000100; // -> 1 byte
  const ADDR_INFO_101 = 0x0e000101;
  const ADDR_INFO_102 = 0x0e000102;
  const ADDR_SLOT_COUNT = 0x0e000103;

  const FLOW_EVERY = 32;
  const START_TIMEOUT = 5000;   // ms, the plug-in uses 5 s and 3 attempts
  const END_TIMEOUT = 20000;
  const RETRIES = 3;
  // Rate of the page (bytes per second). tools/plugout_tool.py sends without
  // pauses because its rawmidi write blocks while the USB buffer is full: the
  // kernel holds it to the synth's pace. MIDIOutput.send never blocks, and a
  // burst of 32 records (19 KB) lost data (END answered 3; Brave on Linux,
  // 2026-10-23). The page therefore limits itself to about the pace the tool
  // reaches (at least 25-40 KB/s measured from its log; docs/WEB_INSTALLER.md §5).
  const WEB_BYTES_PER_SECOND = 32000;
  const RATE_SLACK_MS = 20;      // how far the sending may run ahead before it waits

  const DEVICE_TYPES = { 0: 'SYSTEM-1', 1: 'SYSTEM-1m', 2: 'SYSTEM-8' };
  // Every firmware region starts with "<device>:Plugout" (docs/PROTOCOL.md §4):
  // the device family the image was built for. A SYSTEM-1m takes SYSTEM-1 images.
  const REGION_MAGIC = /^(SYSTEM-[18]):Plugout/;
  const TARGET_OF_TYPE = { 0: 'SYSTEM-1', 1: 'SYSTEM-1', 2: 'SYSTEM-8' };
  const MIN_FIRMWARE = { 0: 0x130, 1: 0x130, 2: 0x131 };
  const SLOT_EMPTY = 0xffff;
  // Flags of the START record: 0x40 = the plug-in is not authorized by Roland
  // Cloud. Our code is not a Roland product, so the browser tool sends the same
  // value the plug-in sends in that case.
  const FLAG_NOT_AUTHORIZED = 0x40;

  // ---------------------------------------------------------------------------
  // small helpers
  // ---------------------------------------------------------------------------

  class PlugoutError extends Error {
    constructor(message, code) {
      super(message);
      this.name = 'PlugoutError';
      this.code = code || 'error';
    }
  }

  function hex(bytes, sep) {
    return Array.prototype.map.call(bytes, (b) => (b & 0xff).toString(16).padStart(2, '0'))
      .join(sep === undefined ? ' ' : sep);
  }

  function addrBytes(value) {
    return [(value >> 21) & 0x7f, (value >> 14) & 0x7f, (value >> 7) & 0x7f, value & 0x7f];
  }

  function addrValue(b) {
    return ((b[0] << 21) | (b[1] << 14) | (b[2] << 7) | b[3]) >>> 0;
  }

  function addrStr(value) {
    return hex(addrBytes(value));
  }

  function checksum(bytes) {
    let sum = 0;
    for (let i = 0; i < bytes.length; i++) sum += bytes[i];
    return (-sum) & 0x7f;
  }

  function buildMessage(command, address, data, deviceId) {
    const a = addrBytes(address);
    const dev = deviceId === undefined ? DEFAULT_DEVICE_ID : deviceId;
    const body = a.concat(Array.from(data));
    return Uint8Array.from([0xf0, ROLAND, dev].concat(MODEL_PLUGOUT, [command], body,
      [checksum(body), 0xf7]));
  }

  function isPlugoutSysex(m) {
    return m.length >= 13 && m[0] === 0xf0 && m[1] === ROLAND &&
      m[3] === 0 && m[4] === 0 && m[5] === 0x7e && m[m.length - 1] === 0xf7;
  }

  function addressOf(m) {
    return addrValue([m[7], m[8], m[9], m[10]]);
  }

  function sysexData(m) {
    return Array.from(m.slice(11, m.length - 2));
  }

  function withData(m, data) {
    const a = m.slice(7, 11);
    const body = Array.from(a).concat(Array.from(data));
    return Uint8Array.from(Array.from(m.slice(0, 11)).concat(Array.from(data),
      [checksum(body), 0xf7]));
  }

  /** A copy of m with another device id (byte 2), as tools/plugout.py with_device_id. */
  function withDeviceId(m, deviceId) {
    const out = Uint8Array.from(m);
    out[2] = deviceId & 0x7f;
    return out;
  }

  /** Groups of <msb byte><up to 7 bytes>; bit j of msb is bit 7 of byte j. */
  function unpack7in8(wire) {
    const out = [];
    for (let i = 0; i < wire.length; i += 8) {
      const msb = wire[i];
      for (let j = 0; j < 7 && i + 1 + j < wire.length; j++) {
        out.push(wire[i + 1 + j] | (((msb >> j) & 1) << 7));
      }
    }
    return Uint8Array.from(out);
  }

  /** Values sent as one 4-bit nibble per byte, most significant first. */
  function nibbles(bytes) {
    let v = 0;
    for (let i = 0; i < bytes.length; i++) v = (v << 4) | (bytes[i] & 0x0f);
    return v >>> 0;
  }

  function versionStr(v) {
    return (v >> 8).toString(16).toUpperCase() + '.' +
      (v & 0xff).toString(16).toUpperCase().padStart(2, '0');
  }

  // ---------------------------------------------------------------------------
  // container (Script/Code*.Dat)
  // ---------------------------------------------------------------------------

  /** plain[i] = cipher[i] ^ cipher[i-1], 32-bit little-endian words. */
  function decodeContainer(data) {
    const src = new Uint8Array(data);
    const n = src.length & ~3;
    const out = new Uint8Array(src.length);
    let prev = DELTA_IV >>> 0;
    for (let i = 0; i < n; i += 4) {
      const w = (src[i] | (src[i + 1] << 8) | (src[i + 2] << 16) | (src[i + 3] << 24)) >>> 0;
      const p = (w ^ prev) >>> 0;
      out[i] = p & 0xff;
      out[i + 1] = (p >>> 8) & 0xff;
      out[i + 2] = (p >>> 16) & 0xff;
      out[i + 3] = (p >>> 24) & 0xff;
      prev = w;
    }
    out.set(src.subarray(n), n);
    return out;
  }

  function u16(b, o) {
    return b[o] | (b[o + 1] << 8);
  }

  function u32(b, o) {
    return (b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24)) >>> 0;
  }

  function asciiField(b, o, n) {
    let s = '';
    for (let i = 0; i < n && b[o + i]; i++) s += String.fromCharCode(b[o + i]);
    return s;
  }

  /**
   * An opened Code*.Dat. Header layout:
   *   +0x00 u32 0x80, +0x04 "PLUGOUT_SYS_INF", +0x14 name (16),
   *   +0x24 u16 model id, +0x26 u16 version,
   *   +0x30 offset/size of the START record, +0x40 offset/size of the END record,
   *   +0x50 offset, stride and count of the data records.
   * Every record is <u32 length><complete SysEx message>.
   */
  class Container {
    constructor(plain, name) {
      this.name = name || '';
      this.plain = plain;
      if (u32(plain, 0) !== HEADER_SIZE || asciiField(plain, 4, 16) !== TYPE_NAME) {
        throw new PlugoutError(
          'This is not a PLUG-OUT firmware file (no PLUGOUT_SYS_INF header).', 'container');
      }
      this.model = asciiField(plain, 0x14, 16);
      this.modelId = u16(plain, 0x24);
      this.version = u16(plain, 0x26);
      this.dataCount = u32(plain, 0x58);
      if (this.dataCount === 0 || this.dataCount > 100000) {
        throw new PlugoutError('The firmware file has an unexpected record count.', 'container');
      }
      this._start = this._record(u32(plain, 0x30));
      this._end = this._record(u32(plain, 0x40));
      // Scan the data records once: the firmware blocks carry 512 bytes each,
      // the filler records nothing. The first block of each run of blocks starts
      // a region, whose first bytes name the device it was built for.
      let chunks = 0;
      let prevChunk = false;
      this.regions = [];
      for (let i = 0; i < this.dataCount; i++) {
        const m = this.dataMessage(i);
        const isChunk = addressOf(m) === ADDR_DATA_CHUNK;
        if (isChunk) {
          chunks++;
          if (!prevChunk) {
            const head = unpack7in8(sysexData(m)).subarray(0, 16);
            const magic = asciiField(head, 0, 16);
            const t = REGION_MAGIC.exec(magic);
            this.regions.push({ record: i, magic: magic, target: t ? t[1] : null });
          }
        }
        prevChunk = isChunk;
      }
      this.firmwareBytes = chunks * 512;
      const targets = this.regions.map((r) => r.target);
      // the device family, when every region names the same one
      this.target = targets.length && targets.every((t) => t && t === targets[0])
        ? targets[0] : null;
    }

    _record(offset) {
      const len = u32(this.plain, offset);
      if (len < 13 || offset + 4 + len > this.plain.length) {
        throw new PlugoutError('The firmware file is damaged (bad record).', 'container');
      }
      const body = this.plain.subarray(offset + 4, offset + 4 + len);
      if (body[0] !== 0xf0 || body[body.length - 1] !== 0xf7) {
        throw new PlugoutError('The firmware file is damaged (bad SysEx record).', 'container');
      }
      return Uint8Array.from(body);
    }

    get startMessage() { return this._start; }
    get endMessage() { return this._end; }

    dataMessage(index) {
      const base = u32(this.plain, 0x50);
      const stride = u32(this.plain, 0x54);
      return this._record(base + index * stride);
    }

    /** Bytes the host sends for the firmware (all records, no flow queries). */
    get wireBytes() {
      if (this._wireData === undefined) {
        let total = this.startMessage.length + this.endMessage.length;
        for (let i = 0; i < this.dataCount; i++) total += this.dataMessage(i).length;
        this._wireData = total;
      }
      return this._wireData;
    }

    summary() {
      return {
        model: this.model,
        modelId: this.modelId,
        version: this.version,
        dataRecords: this.dataCount,
        firmwareBytes: this.firmwareBytes,
        target: this.target,
        regions: this.regions.length,
        startMessage: hex(this.startMessage),
        endMessage: hex(this.endMessage),
      };
    }
  }

  function loadContainer(buffer, name) {
    return new Container(decodeContainer(buffer), name);
  }

  /**
   * Same as loadContainer(), but also reports what the file decodes to when
   * it is not a PLUG-OUT container at all, so the page can say which file it
   * thinks it read even in the error case.
   */
  function inspectContainer(buffer, name) {
    const plain = decodeContainer(buffer);
    const out = {
      name: name || '',
      bytes: plain.length,
      isContainer: u32(plain, 0) === HEADER_SIZE && asciiField(plain, 4, 16) === TYPE_NAME,
      model: asciiField(plain, 0x14, 16),
      modelId: u16(plain, 0x24),
      version: u16(plain, 0x26),
      dataRecords: u32(plain, 0x58),
    };
    return out;
  }

  /** The file the plug-in picks: Script/Code1.Dat (SYSTEM-1) or Code8_<slot>.Dat. */
  function containerPath(deviceType, slot) {
    if (deviceType === 0 || deviceType === 1) return 'Script/Code1.Dat';
    return 'Script/Code8_' + slot + '.Dat';
  }

  // ---------------------------------------------------------------------------
  // device answers
  // ---------------------------------------------------------------------------

  /**
   * The 0x30-byte answer to the slot query: bytes 0-3 firmware version (nibbles),
   * 8-11 model id, 12-15 version, 16-47 name (16 characters, two nibbles each).
   */
  function parseSlotInfo(index, data) {
    let name = '';
    for (let i = 0; i < 16; i++) {
      const c = nibbles(data.slice(16 + i * 2, 18 + i * 2)) & 0xff;
      if (c === 0) break;
      name += String.fromCharCode(c);
    }
    return {
      index: index,
      modelId: nibbles(data.slice(8, 12)),
      version: nibbles(data.slice(12, 16)),
      name: name.trim(),
      empty: nibbles(data.slice(8, 12)) === SLOT_EMPTY,
    };
  }

  function describeSlot(slot) {
    if (slot.empty) return 'empty';
    return (slot.name || 'unknown') +
      (slot.version ? ' (version ' + versionStr(slot.version) + ')' : '');
  }

  // ---------------------------------------------------------------------------
  // transport
  // ---------------------------------------------------------------------------

  function sleep(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
  }

  /**
   * Wraps a Web MIDI output port. The page sends, this class waits for the
   * answers, with the plug-in's retry rules.
   */
  class Port {
    constructor(output, input, options) {
      const o = options || {};
      this.output = output;
      // one MIDIInput or several: the answers are matched by address, so they may
      // come back on any of the synth's ports (docs/WEB_INSTALLER.md §4)
      this.inputs = (Array.isArray(input) ? input : [input]).filter(Boolean);
      this.input = this.inputs[0] || null;
      this.deviceId = o.deviceId === undefined ? DEFAULT_DEVICE_ID : o.deviceId;
      this.gap = o.gap || 0;            // extra pause between messages, ms
      this.wirePacing = !!o.wirePacing; // also wait the MIDI wire time
      this.rate = o.bytesPerSecond || 0; // byte rate limit (0 = none), see WEB_BYTES_PER_SECOND
      this._budget = 0;                 // when the bytes sent so far are due at that rate
      this.clock = o.clock || (() => Date.now());
      this.sleepFn = o.sleep || sleep;
      // Waiting for an answer: by default the wait ends on the MIDI message itself
      // (no polling: browsers throttle setTimeout, to 1 s and more in a background
      // tab, which made every flow-control round of 32 records slow). A custom
      // sleep (the tests' fake clock) polls instead.
      this.eventWait = !o.sleep;
      this._waiters = [];
      this.stats = { waits: 0, waitMs: 0, maxWaitMs: 0, resends: 0 };
      this.log = o.log || function () {};
      this.inbox = [];
      this.received = 0;
      this._nextSend = 0;
      this._onMessage = (event) => this._push(event.data);
      this.inputs.forEach((i) => { i.onmidimessage = this._onMessage; });
    }

    /**
     * Stop listening and close both Web MIDI ports, so that another program
     * (tools/plugout_tool.py, a DAW) can open the synth (docs/WEB_INSTALLER.md §4).
     */
    close() {
      this.inputs.forEach((i) => { i.onmidimessage = null; });
      for (const p of this.inputs.concat([this.output])) {
        if (p && typeof p.close === 'function') {
          Promise.resolve(p.close()).catch(() => {});
        }
      }
    }

    /** Drop the answers queued so far (a new round of questions starts clean). */
    clearInbox() {
      this.inbox = [];
    }

    /** Every message the device sends (used for the traffic log). */
    _push(data) {
      const m = Uint8Array.from(data);
      if (!isPlugoutSysex(m)) {
        if (m[0] === 0xf0) this.log('ignored SysEx ' + hex(m.subarray(0, 12)) + '...');
        return;
      }
      this.received++;
      if (this.received <= 8 || this.received % 500 === 0) this.log('<- ' + hex(m));
      this.inbox.push(m);
      const waiters = this._waiters;
      this._waiters = [];
      waiters.forEach((w) => w());
    }

    /** Resolves on the next message from the device, or after `ms`. */
    _wait(ms) {
      if (!this.eventWait) return this.sleepFn(Math.min(ms, 5));
      return new Promise((resolve) => {
        let timer = null;
        const done = () => {
          clearTimeout(timer);
          this._waiters = this._waiters.filter((w) => w !== done);
          resolve();
        };
        timer = setTimeout(done, ms);
        this._waiters.push(done);
      });
    }

    /**
     * Send one message, honouring the pacing (none by default). With a byte rate
     * the messages go out at once while they are within RATE_SLACK_MS of that
     * rate, then the sending waits: the average stays at the rate even though
     * browser timers are coarse.
     */
    send(msg) {
      const now = this.clock();
      let wait = this._nextSend - now;
      if (this.rate) {
        this._budget = Math.max(this._budget, now) + msg.length * 1000 / this.rate;
        wait = Math.max(wait, this._budget - now - RATE_SLACK_MS);
      }
      const send = () => {
        this._nextSend = this.clock() + this.gap +
          (this.wirePacing ? (msg.length / 3125) * 1000 : 0);
        this.output.send(msg);
      };
      if (wait > 0) {
        return this.sleepFn(wait).then(send);
      }
      send();
      return Promise.resolve();
    }

    take(address, size) {
      for (let i = 0; i < this.inbox.length; i++) {
        const m = this.inbox[i];
        if (addressOf(m) === address && m.length - 13 === size) {
          this.inbox.splice(i, 1);
          return sysexData(m);
        }
      }
      return null;
    }

    checkDataError(index) {
      const err = this.take(ADDR_DATA_ERROR, 1);
      if (err) {
        throw new PlugoutError(
          'The device rejected data block ' + index + ' (code ' + err[0] + '). ' +
          'Nothing else was sent.', 'record');
      }
    }

    /** Send msg and wait for `size` data bytes on `replyAddress`; resend on timeout. */
    async request(msg, replyAddress, size, timeout, retries) {
      timeout = timeout || START_TIMEOUT;
      retries = retries || RETRIES;
      // Drop every stale answer: the device only answers late replies, and with
      // one still queued this loop would never yield to the MIDI callback and
      // would spin until the timeout. Only "rejected record" notices survive.
      this.inbox = this.inbox.filter((m) => addressOf(m) === ADDR_DATA_ERROR);
      // MIDI callbacks only run while this function awaits, so an answer can
      // never be lost between the filter above and take() below.
      for (let attempt = 0; attempt < retries; attempt++) {
        if (attempt) {
          this.stats.resends++;
          if (timeout >= 1000 || attempt % 8 === 0) {
            this.log('no answer on ' + addrStr(replyAddress) + ' after ' + timeout / 1000 +
              ' s: sending again (attempt ' + (attempt + 1) + ')');
          }
        }
        const t0 = this.clock();
        await this.send(msg);
        const deadline = t0 + timeout;
        for (;;) {
          const data = this.take(replyAddress, size);
          if (data) {
            const ms = this.clock() - t0;
            this.stats.waits++;
            this.stats.waitMs += ms;
            this.stats.maxWaitMs = Math.max(this.stats.maxWaitMs, ms);
            return data;
          }
          const left = deadline - this.clock();
          if (left <= 0) break;
          await this._wait(left);
        }
      }
      throw new PlugoutError('No answer from the device (' + addrStr(replyAddress) +
        ') after ' + retries + ' attempts (' + (retries * timeout / 1000) + ' s). Check the cable, the port and that ' +
        'another program is not using the synth.', 'timeout');
    }

    write(address, data, size) {
      return this.request(buildMessage(DT1, address, data, this.deviceId),
        address | REPLY_BIT, size);
    }

    // -- device queries -------------------------------------------------------

    /** Asks the device what it is and what its slots hold. */
    async deviceInfo(onStep) {
      const step = onStep || function () {};
      step('Asking the device what it is');
      const info = {};
      info.type = (await this.write(ADDR_DEVICE_TYPE, [0], 1))[0];
      if (info.type > 2) {
        throw new PlugoutError('This device does not answer as a SYSTEM-8 / SYSTEM-1 ' +
          '(type ' + info.type + ').', 'device');
      }
      info.deviceName = DEVICE_TYPES[info.type];
      info.firmware = nibbles((await this.write(ADDR_SLOT_INFO, [0], 0x30)).slice(0, 4));
      info.minFirmware = MIN_FIRMWARE[info.type];
      step('Reading the slots');
      info.patches = (await this.write(ADDR_INFO_101, [0], 1))[0] *
        (await this.write(ADDR_INFO_102, [0], 1))[0];
      const count = (await this.write(ADDR_SLOT_COUNT, [0], 1))[0];
      if (count < 1 || count > 8) {
        throw new PlugoutError('The device reports an unexpected number of slots (' +
          count + ').', 'device');
      }
      info.slots = [];
      for (let i = 0; i < count; i++) {
        const data = await this.write(ADDR_SLOT_INFO, [i], 0x30);
        info.slots.push(parseSlotInfo(i, data));
      }
      return info;
    }

    // -- install --------------------------------------------------------------

    /**
     * Firmware: START, the data records with the flow-control query every 32,
     * END. `progress(done, total)` is called after every record.
     */
    async install(container, slot, flags, progress) {
      const fl = flags === undefined ? 0 : flags;
      const dev = this.deviceId;
      const start = withDeviceId(withData(container.startMessage,
        [(fl & 0x70) | (slot & 0x0f)]), dev);
      const reply = await this.request(start, ADDR_START | REPLY_BIT, 1);
      if (reply[0] !== 0) {
        throw new PlugoutError('The device refused the transfer (reply ' +
          reply[0] + '). Nothing was written.', 'refused');
      }
      const total = container.dataCount;
      const tStart = this.clock();
      this.stats = { waits: 0, waitMs: 0, maxWaitMs: 0, resends: 0 };
      for (let i = 0; i < total; i++) {
        if (i % FLOW_EVERY === FLOW_EVERY - 1) {
          await this.write(ADDR_FLOW, [0], 4);
          if (i === FLOW_EVERY - 1 || (i + 1) % 512 === 0) {
            this.log('record ' + (i + 1) + '/' + total + ': ' + (this.clock() - tStart) +
              ' ms so far, flow answers ' + Math.round(this.stats.waitMs /
              Math.max(1, this.stats.waits)) + ' ms on average, ' + this.stats.maxWaitMs +
              ' ms at most, ' + this.stats.resends + ' resent');
          }
        }
        await this.send(withDeviceId(container.dataMessage(i), dev));
        this.checkDataError(i);
        if (progress) progress(i + 1, total);
      }
      if (progress) progress(total, total);
      const end = await this.request(withDeviceId(container.endMessage, dev),
        ADDR_END | REPLY_BIT, 1, END_TIMEOUT);
      this.log('transfer (' + (this.rate ? Math.round(this.rate / 1000) + ' KB/s'
        : 'no rate limit') + '): ' + (this.clock() - tStart) + ' ms, ' + this.stats.waits +
        ' answers (' + Math.round(this.stats.waitMs / Math.max(1, this.stats.waits)) +
        ' ms on average, ' + this.stats.maxWaitMs + ' ms at most), ' + this.stats.resends +
        ' resent');
      if (end[0] !== 0) {
        throw new PlugoutError('The device reported an error at the end (' +
          end[0] + ').', 'end');
      }
    }
  }

  // ---------------------------------------------------------------------------
  // Web MIDI availability
  // ---------------------------------------------------------------------------

  /** Only https:// or a local page can request Web MIDI. */
  function isSecurePage() {
    return typeof window === 'undefined' || window.isSecureContext !== false;
  }

  /** Can this page ask for Web MIDI + SysEx? Same verdict as the Python port. */
  function webMidiSupport() {
    const hasApi = typeof navigator !== 'undefined' && typeof navigator.requestMIDIAccess === 'function';
    if (!isSecurePage()) return { supported: false, secure: false, reason: 'insecure' };
    if (!hasApi) return { supported: false, secure: true, reason: 'no-api' };
    return { supported: true, secure: true, reason: '' };
  }

  /** Ask for MIDI access with SysEx (the browser shows its own permission prompt). */
  function requestMidi() {
    if (!isSecurePage()) {
      return Promise.reject(new PlugoutError('Web MIDI needs a secure page: ' +
        'open this page on http://localhost or https://.', 'insecure'));
    }
    if (!webMidiSupport().supported) {
      return Promise.reject(new PlugoutError('This browser has no Web MIDI. ' +
        'Use Chrome, Edge, Opera or another Chromium browser.', 'no-api'));
    }
    const grant = navigator.permissions && navigator.permissions.query
      ? navigator.permissions.query({ name: 'midi', sysex: true }).catch(() => null)
      : Promise.resolve(null);
    return grant
      .then(() => navigator.requestMIDIAccess({ sysex: true, software: false }))
      .catch((err) => {
        throw new PlugoutError('MIDI access was refused (' +
          (err && err.message ? err.message : err) + '). Allow MIDI (and "Send and receive ' +
          'SysEx") for this page and try again.', 'denied');
      });
  }

  /** best = Web MIDI with SysEx granted; no-sysex = MIDI but no SysEx; denied = refused. */
  function accessState(access) {
    if (!access) return 'denied';
    return access.sysexEnabled ? 'best' : 'no-sysex';
  }

  function watchAccess(access, onchange) {
    if (access) access.onstatechange = () => onchange(access);
  }

  function listOutputs(access) {
    const out = [];
    if (access && access.outputs) {
      // `port` is the MIDIPort itself: what Port() sends on / listens to
      access.outputs.forEach((p) => out.push({ id: p.id, name: p.name,
        manufacturer: p.manufacturer || '', state: p.state || '', port: p }));
    }
    return out;
  }

  function listInputs(access) {
    const out = [];
    if (access && access.inputs) {
      // `port` is the MIDIPort itself: what Port() sends on / listens to
      access.inputs.forEach((p) => out.push({ id: p.id, name: p.name,
        manufacturer: p.manufacturer || '', state: p.state || '', port: p }));
    }
    return out;
  }

  /**
   * The synth's second USB MIDI port by its name: "SYSTEM-8 CTRL", Linux
   * "SYSTEM-1 MIDI 2", Windows "MIDIIN2 (SYSTEM-1)" / "MIDIOUT2 (SYSTEM-1)".
   * Only that port answers the flow-control queries at once (docs/PROTOCOL.md §1).
   */
  function isControlName(name) {
    return /CTRL|MIDI\s*2\b|MIDI(IN|OUT)2/i.test(name || '');
  }

  /** Same rule as tools/midi_io.py pick_port: the port names a SYSTEM-8 / SYSTEM-1. */
  function isSynthPort(name) {
    return /SYSTEM[-\s]?8|SYSTEM[-\s]?1/i.test(name || '');
  }

  /**
   * Find the synth as tools/midi_io.py pick_port: among the ports that name a
   * SYSTEM-8 / SYSTEM-1, the one with CTRL in its name, else the second one
   * (the control port; Linux: "SYSTEM-1 MIDI 2"), else the only one. With
   * `allowAny` every port is a candidate when none names the synth.
   */
  function findDevicePorts(access, allowAny) {
    const outs = listOutputs(access);
    const ins = listInputs(access);
    const isSynth = isSynthPort;
    // By name, never by the browser's list order (Brave on Linux lists
    // "SYSTEM-1 MIDI 2" before "MIDI 1": "the second one" was then MIDI 1).
    const pick = (list) => {
      const found = list.filter((p) => isSynth(p.name));
      const pool = (found.length ? found : (allowAny ? list : []))
        .slice().sort((a, b) => (a.name < b.name ? -1 : a.name > b.name ? 1 : 0));
      return pool.find((p) => /CTRL/i.test(p.name)) ||
        pool.find((p) => isControlName(p.name)) || pool[1] || pool[0] || null;
    };
    // the input that answers for an output: the CTRL port, or else one with the
    // synth's name (a USB MIDI interface usually names both ends the same)
    const output = pick(outs);
    const input = (output && (ins.find((p) => p.name === output.name &&
      /CTRL/i.test(p.name)) || ins.find((p) => p.name === output.name))) ||
      pick(ins);
    return { output: output, input: input || null };
  }

  // ---------------------------------------------------------------------------
  // the whole install, as one call (used by the page and by the tests)
  // ---------------------------------------------------------------------------

  function slotState(slot, container) {
    if (slot.empty) return 'empty';
    if (slot.modelId === container.modelId && slot.name === container.model) {
      return slot.version === container.version ? 'same' : 'other-version';
    }
    return 'other';
  }

  /**
   * Plan of an install: what is in the target slot, what will be lost and what
   * the page must warn about. Nothing is sent.
   */
  function planInstall(info, container, slot) {
    if (slot < 1 || slot >= info.slots.length) {
      throw new PlugoutError('Slot ' + slot + ' does not exist on this device.', 'slot');
    }
    const target = info.slots[slot];
    const state = slotState(target, container);
    const elsewhere = info.slots.filter((s) => s.index !== slot &&
      (slotState(s, container) === 'same' || slotState(s, container) === 'other-version'));
    return {
      target: target,
      state: state,
      current: describeSlot(target),
      willOverwrite: state === 'other' || state === 'other-version',
      skipFirmware: state === 'same',
      sameModelInSlots: elsewhere.map((s) => s.index),
      firmwareTooOld: info.firmware < info.minFirmware,
      // the image names another device family (null = could not tell)
      wrongDevice: !!container.target && container.target !== TARGET_OF_TYPE[info.type],
      unknownTarget: !container.target,
    };
  }

  const Plugout = {
    // constants
    DEFAULT_DEVICE_ID: DEFAULT_DEVICE_ID,
    REPLY_BIT: REPLY_BIT,
    FLAG_NOT_AUTHORIZED: FLAG_NOT_AUTHORIZED,
    START_TIMEOUT: START_TIMEOUT,
    FLOW_EVERY: FLOW_EVERY,
    WEB_BYTES_PER_SECOND: WEB_BYTES_PER_SECOND,
    END_TIMEOUT: END_TIMEOUT,
    DEVICE_TYPES: DEVICE_TYPES,
    TARGET_OF_TYPE: TARGET_OF_TYPE,
    ADDR: {
      START: ADDR_START,
      END: ADDR_END,
      DATA_CHUNK: ADDR_DATA_CHUNK,
      DATA_FILL: ADDR_DATA_FILL,
      FLOW: ADDR_FLOW,
      DATA_ERROR: ADDR_DATA_ERROR,
      SLOT_INFO: ADDR_SLOT_INFO,
      DEVICE_TYPE: ADDR_DEVICE_TYPE,
    },
    // protocol
    PlugoutError: PlugoutError,
    Container: Container,
    Port: Port,
    decodeContainer: decodeContainer,
    loadContainer: loadContainer,
    inspectContainer: inspectContainer,
    containerPath: containerPath,
    parseSlotInfo: parseSlotInfo,
    describeSlot: describeSlot,
    planInstall: planInstall,
    slotState: slotState,
    // SysEx helpers
    buildMessage: buildMessage,
    isPlugoutSysex: isPlugoutSysex,
    addressOf: addressOf,
    addrBytes: addrBytes,
    addrValue: addrValue,
    checksum: checksum,
    withData: withData,
    withDeviceId: withDeviceId,
    unpack7in8: unpack7in8,
    nibbles: nibbles,
    versionStr: versionStr,
    hex: hex,
    // Web MIDI
    webMidiSupport: webMidiSupport,
    isSecurePage: isSecurePage,
    requestMidi: requestMidi,
    accessState: accessState,
    watchAccess: watchAccess,
    listOutputs: listOutputs,
    listInputs: listInputs,
    findDevicePorts: findDevicePorts,
    isSynthPort: isSynthPort,
    isControlName: isControlName,
  };

  if (typeof module !== 'undefined' && module.exports) module.exports = Plugout;
  root.Plugout = Plugout;
})(typeof globalThis !== 'undefined' ? globalThis : this);
