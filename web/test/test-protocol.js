#!/usr/bin/env node
/*
 * test-protocol.js -- checks web/plugout-protocol.js against tools/plugout.py.
 *
 *   node web/test/test-protocol.js [path/to/Code*.Dat ...]
 *
 * A fake synth answers every question the page asks (docs/PROTOCOL.md §2, §5)
 * and records every message the library sends, so the whole install runs
 * without hardware. For each Code*.Dat (default: every plugins/<name>/Script/
 * Code1.Dat and Code8_*.Dat found) it also checks that the JavaScript decoder
 * gives the same messages as tools/plugout.py, and replays the install.
 * Exit code 1 if a check fails.
 */
'use strict';

const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const ROOT = path.resolve(__dirname, '..', '..');
const P = require(path.join(ROOT, 'web', 'plugout-protocol.js'));

let failures = 0;
function check(name, ok, detail) {
  console.log((ok ? 'ok   ' : 'FAIL ') + name + (ok || !detail ? '' : '\n     ' + detail));
  if (!ok) failures++;
}

// ---------------------------------------------------------------------------
// a fake synth: answers as the device does (reply = address | bit 21)
// ---------------------------------------------------------------------------

// The 0x30-byte slot answer: firmware/model/version as one nibble per byte,
// then the name as two nibbles per character (tools/plugout.py parse_slot_info).
function slotInfo(name, model, version, firmware) {
  const data = new Array(0x30).fill(0);
  const put = (value, at, digits) => {
    for (let i = 0; i < digits; i++) data[at + i] = (value >>> (4 * (digits - 1 - i))) & 0x0f;
  };
  put(firmware === undefined ? 0x130 : firmware, 0, 4);
  put(model, 8, 4);
  put(version, 12, 4);
  for (let i = 0; i < 16; i++) put(i < name.length ? name.charCodeAt(i) : 0, 16 + i * 2, 2);
  return data;
}

/**
 * opts: type (device type, default 0 = SYSTEM-1), slots ([[name, model, version]]
 * for slots 1..; slot 0 = the synth), refuse (START answers 1), reject (index of a
 * data record the device rejects), silent (no answers at all), noise (an unrelated
 * SysEx after every answer).
 */
function fakeSynth(opts) {
  const o = opts || {};
  const slots = [['', 0, 0]].concat(o.slots || [['', 0xffff, 0xffff], ['', 0xffff, 0xffff]]);
  const out = { id: 'out-1', name: 'SYSTEM-1 MIDI 2', manufacturer: 'Roland', sent: [],
    closed: false, close() { this.closed = true; } };
  const inp = { id: 'in-1', name: 'SYSTEM-1 MIDI 2', manufacturer: 'Roland',
    onmidimessage: null, closed: false, close() { this.closed = true; } };
  let records = 0;
  const answer = (address, data) => {
    if (!inp.onmidimessage) return;
    inp.onmidimessage({ data: P.buildMessage(0x12, address | P.REPLY_BIT, data, 0x10) });
    if (o.noise) inp.onmidimessage({ data: Uint8Array.from([0xf0, 0x7e, 0x7f, 0x06, 0x01, 0xf7]) });
  };
  out.send = (m) => {
    out.sent.push(Uint8Array.from(m));
    if (o.silent) return;
    const address = P.addressOf(m);
    const arg = m[11];
    switch (address) {
      case P.ADDR.DEVICE_TYPE: answer(address, [o.type || 0]); break;
      case P.ADDR.SLOT_INFO: {
        const s = slots[arg] || ['', 0xffff, 0xffff];
        answer(address, slotInfo(s[0], s[1], s[2], o.firmware));
        break;
      }
      case 0x0e000101: answer(address, [8]); break;
      case 0x0e000102: answer(address, [8]); break;
      case 0x0e000103: answer(address, [slots.length]); break;
      case P.ADDR.START: answer(address, [o.refuse ? 1 : 0]); break;
      case P.ADDR.END: answer(address, [0]); break;
      case P.ADDR.FLOW: answer(address, [0, 0, 0, 0]); break;
      case P.ADDR.DATA_CHUNK:
      case P.ADDR.DATA_FILL:
        if (records++ === o.reject) {
          inp.onmidimessage({ data: P.buildMessage(0x12, P.ADDR.DATA_ERROR, [5], 0x10) });
        }
        break;
      default: break;
    }
  };
  return { out, inp };
}

function openPort(fake, extra) {
  // sleep() only lets the event loop run: the fake answers at once.
  return new P.Port(fake.out, fake.inp, Object.assign({ sleep: () => new Promise(setImmediate) },
    extra || {}));
}

const asyncChecks = [];

// ---------------------------------------------------------------------------
// SysEx helpers
// ---------------------------------------------------------------------------

check('addresses are 4 x 7-bit bytes',
  P.hex(P.addrBytes(0x0e040001)) === '70 10 00 01', P.hex(P.addrBytes(0x0e040001)));
check('checksum makes the payload sum 0 mod 128',
  P.checksum([0x70, 0x10, 0x00, 0x00, 0x01]) === 0x7f);
check('the start message is the one the plugin sends',
  P.hex(P.buildMessage(0x12, 0x0e040000, [0x01])) === 'f0 41 10 00 00 7e 12 70 10 00 00 01 7f f7',
  P.hex(P.buildMessage(0x12, 0x0e040000, [0x01])));
check('withData() rewrites the data and the checksum',
  P.hex(P.withData(P.buildMessage(0x12, 0x0e040000, [0]), [0x41]))
    === 'f0 41 10 00 00 7e 12 70 10 00 00 41 3f f7');
check('withDeviceId() changes byte 2 only',
  P.hex(P.withDeviceId(P.buildMessage(0x12, 0x0e040000, [1]), 0x11))
    === 'f0 41 11 00 00 7e 12 70 10 00 00 01 7f f7');
check('unpack7in8() puts the msb byte back (tools/plugout.py unpack_7in8)',
  P.hex(P.unpack7in8([0x05, 0x01, 0x02, 0x03])) === '81 02 83');

// ---------------------------------------------------------------------------
// device answers
// ---------------------------------------------------------------------------

const parsed = P.parseSlotInfo(1, slotInfo('SH-101', 0x01, 0x14fa));
check('slot info decodes name, model and version',
  parsed.name === 'SH-101' && parsed.modelId === 0x01 && parsed.version === 0x14fa &&
  !parsed.empty, JSON.stringify(parsed));
check('an unwritten slot reads as empty',
  P.parseSlotInfo(1, new Array(0x30).fill(0x0f)).empty);
check('version strings match the plugin %X.%02X',
  P.versionStr(0x14fa) === '14.FA' && P.versionStr(0x130) === '1.30');

// ---------------------------------------------------------------------------
// port choice (same rule as tools/midi_io.py pick_port)
// ---------------------------------------------------------------------------

const access = (names) => {
  const ports = names.map((n, i) => ({ id: 'p' + i, name: n, manufacturer: '' }));
  return { outputs: ports, inputs: ports };
};
check('Linux names: the second SYSTEM-1 port is the control port',
  P.findDevicePorts(access(['Midi Through Port-0', 'SYSTEM-1 MIDI 1', 'SYSTEM-1 MIDI 2']))
    .output.name === 'SYSTEM-1 MIDI 2');
check('a port with CTRL in its name wins',
  P.findDevicePorts(access(['SYSTEM-8', 'SYSTEM-8 CTRL'])).output.name === 'SYSTEM-8 CTRL');
check('the port lists carry the MIDIPort objects themselves (Port() sends on them)',
  P.listOutputs(access(['SYSTEM-1 MIDI 2']))[0].port.name === 'SYSTEM-1 MIDI 2' &&
  P.findDevicePorts(access(['SYSTEM-1 MIDI 1', 'SYSTEM-1 MIDI 2'])).input.port.id === 'p1');
check('without allowAny no other port is a candidate',
  P.findDevicePorts(access(['Midi Through Port-0'])).output === null);

// ---------------------------------------------------------------------------
// the request/reply engine
// ---------------------------------------------------------------------------

asyncChecks.push((async () => {
  const fake = fakeSynth();
  const port = openPort(fake);
  const reply = await port.write(P.ADDR.DEVICE_TYPE, [0], 1);
  check('a query gets its answer', reply[0] === 0, String(reply));
  check('the query is a DT1 to 70 00 02 00',
    P.hex(fake.out.sent[0]) === 'f0 41 10 00 00 7e 12 70 00 02 00 00 0e f7',
    P.hex(fake.out.sent[0]));
})());

asyncChecks.push((async () => {
  // a reply to the previous question is still queued when the next one starts
  const fake = fakeSynth();
  const port = openPort(fake);
  fake.inp.onmidimessage({ data: P.buildMessage(0x12, P.ADDR.DEVICE_TYPE | P.REPLY_BIT,
    [7], 0x10) });
  const reply = await port.write(P.ADDR.DEVICE_TYPE, [0], 1);
  check('a stale answer is dropped, the fresh one is taken',
    reply[0] === 0 && fake.out.sent.length === 1, String(reply) + ', ' +
    fake.out.sent.length + ' sent');
})());

asyncChecks.push((async () => {
  // unrelated SysEx between the answers must not make the library resend
  const fake = fakeSynth({ noise: true });
  const port = openPort(fake);
  const info = await port.deviceInfo();
  check('unrelated SysEx does not cause resends',
    fake.out.sent.length === 5 + info.slots.length, fake.out.sent.length + ' messages sent');
})());

asyncChecks.push((async () => {
  let t = 0;
  const fake = fakeSynth({ silent: true });
  const port = openPort(fake, { clock: () => t, sleep: () => { t += 1000; return Promise.resolve(); } });
  let err = null;
  try {
    await port.write(P.ADDR.DEVICE_TYPE, [0], 1);
  } catch (e) {
    err = e;
  }
  check('no answer: 3 attempts, then a timeout error',
    err && err.code === 'timeout' && fake.out.sent.length === 3,
    (err ? err.code : 'no error') + ', ' + fake.out.sent.length + ' sent');
})());

asyncChecks.push((async () => {
  const fake = fakeSynth({ type: 2, slots: [['JX-3P', 0x08, 0x14fa], ['', 0xffff, 0xffff],
    ['C-S1SY', 0x10, 0x0001]] });
  const port = openPort(fake);
  const info = await port.deviceInfo();
  check('deviceInfo() reads type, firmware, patches and every slot',
    info.type === 2 && info.deviceName === 'SYSTEM-8' && info.firmware === 0x130 &&
    info.patches === 64 && info.slots.length === 4 && info.slots[1].name === 'JX-3P' &&
    info.slots[2].empty && info.slots[3].modelId === 0x10, JSON.stringify(info));
  port.close();
  check('close() closes both Web MIDI ports', fake.out.closed && fake.inp.closed);
})());

const oneRecord = (n) => ({
  startMessage: P.buildMessage(0x12, P.ADDR.START, [0]),
  endMessage: P.buildMessage(0x12, P.ADDR.END, [0, 0, 0, 0]),
  dataCount: n || 1,
  dataMessage: () => P.buildMessage(0x12, P.ADDR.DATA_CHUNK, [0]),
});

asyncChecks.push((async () => {
  const fake = fakeSynth({ refuse: true });
  const port = openPort(fake);
  let err = null;
  try {
    await port.install(oneRecord(), 1, 0);
  } catch (e) {
    err = e;
  }
  check('a refusal stops the install before any firmware is sent',
    err && err.code === 'refused' && fake.out.sent.length === 1,
    err ? err.message : 'no error');
})());

asyncChecks.push((async () => {
  const fake = fakeSynth({ reject: 40 });
  const port = openPort(fake);
  let err = null;
  try {
    await port.install(oneRecord(100), 1, 0);
  } catch (e) {
    err = e;
  }
  const records = fake.out.sent.filter((m) => P.addressOf(m) === P.ADDR.DATA_CHUNK).length;
  check('a rejected record stops the install at that record (no END)',
    err && err.code === 'record' && records === 41 &&
    P.addressOf(fake.out.sent[fake.out.sent.length - 1]) === P.ADDR.DATA_CHUNK,
    (err ? err.message : 'no error') + ', ' + records + ' records sent');
})());

asyncChecks.push((async () => {
  const fake = fakeSynth();
  const port = openPort(fake, { deviceId: 0x11 });
  await port.install(oneRecord(3), 2, 0);
  check('the device id goes into START, every record and END',
    fake.out.sent.every((m) => m[2] === 0x11 || P.addressOf(m) === P.ADDR.FLOW) &&
    fake.out.sent[0][11] === 2, fake.out.sent.map((m) => P.hex(m.subarray(0, 12))).join(' | '));
})());

// ---------------------------------------------------------------------------
// install plan
// ---------------------------------------------------------------------------

{
  const info = { type: 0, firmware: 0x130, minFirmware: 0x130,
    slots: [P.parseSlotInfo(0, slotInfo('', 0, 0)), P.parseSlotInfo(1, slotInfo('C-S1SY13', 0x10, 1)),
      P.parseSlotInfo(2, new Array(0x30).fill(0x0f))] };
  const c = { model: 'C-S1SY13F', modelId: 0x10, version: 1, target: 'SYSTEM-1' };
  const plan = P.planInstall(info, c, 1);
  check('planInstall(): another plug-out in the slot is overwritten',
    plan.state === 'other' && plan.willOverwrite && !plan.wrongDevice, JSON.stringify(plan));
  check('planInstall(): a SYSTEM-8 image on a SYSTEM-1 is flagged',
    P.planInstall(info, Object.assign({}, c, { target: 'SYSTEM-8' }), 2).wrongDevice);
  check('planInstall(): a SYSTEM-1 image is fine on a SYSTEM-1m',
    !P.planInstall(Object.assign({}, info, { type: 1 }), c, 2).wrongDevice);
  check('planInstall(): the same model in another slot is reported',
    P.planInstall(info, Object.assign({}, c, { model: 'C-S1SY13' }), 2).sameModelInSlots[0] === 1);
  check('planInstall(): old firmware is reported',
    P.planInstall(Object.assign({}, info, { firmware: 0x120 }), c, 2).firmwareTooOld);
}

// ---------------------------------------------------------------------------
// containers: parity with the Python side, then a full replayed install
// ---------------------------------------------------------------------------

function defaultContainers() {
  const dir = path.join(ROOT, 'plugins');
  if (!fs.existsSync(dir)) return [];
  const found = [];
  for (const name of fs.readdirSync(dir).sort()) {
    const script = path.join(dir, name, 'Script');
    if (!fs.existsSync(script)) continue;
    for (const f of fs.readdirSync(script).sort()) {
      if (/^Code(1|8_\d)\.Dat$/.test(f)) found.push(path.join(script, f));
    }
  }
  // one of each kind is enough: our SYSTEM-1 build, Roland's SYSTEM-8 one
  const pick = [found.find((f) => /C-S1SY/.test(f) && /Code1\.Dat$/.test(f)),
    found.find((f) => /Code8_1\.Dat$/.test(f))];
  return pick.filter(Boolean);
}

const dats = process.argv.length > 2 ? process.argv.slice(2) : defaultContainers();
if (!dats.length) console.log('skip container checks: no Code*.Dat given or found in plugins/');

for (const datPath of dats) {
  const tag = path.relative(ROOT, datPath);
  const c = P.loadContainer(fs.readFileSync(datPath), datPath);
  const py = JSON.parse(execFileSync('python3',
    [path.join(ROOT, 'tools', 'plugout.py'), '--json', datPath],
    { encoding: 'utf8', maxBuffer: 256 << 20 }));
  check(tag + ': header matches tools/plugout.py',
    c.model === py.model && c.modelId === py.modelId && c.version === py.version &&
    c.dataCount === py.dataCount,
    JSON.stringify(c.summary()) + ' vs ' + JSON.stringify([py.model, py.modelId, py.version]));
  let differ = 0;
  for (let i = 0; i < c.dataCount; i++) {
    if (P.hex(c.dataMessage(i), '') !== py.records[i]) differ++;
  }
  check(tag + ': all ' + c.dataCount + ' data records match the Python decoder',
    differ === 0 && py.records.length === c.dataCount,
    differ + ' of ' + py.records.length + ' records differ');
  check(tag + ': start and end records match the Python decoder',
    P.hex(c.startMessage) === py.start && P.hex(c.endMessage) === py.end);
  check(tag + ': regions and target device match the Python image_regions()',
    c.regions.length === py.regions.length &&
    c.regions.every((r, i) => r.magic === py.regions[i]) &&
    c.target === (py.regions[0] || '').split(':')[0],
    JSON.stringify(c.regions) + ' vs ' + JSON.stringify(py.regions));

  asyncChecks.push((async () => {
    const type = c.target === 'SYSTEM-8' ? 2 : 0;
    const fake = fakeSynth({ type: type });
    const port = openPort(fake);
    await port.install(c, 1, 0);
    const sent = fake.out.sent.map((m) => P.hex(m, ''));
    const flow = P.hex(P.buildMessage(0x12, P.ADDR.FLOW, [0]), '');
    const expected = [P.hex(P.withData(c.startMessage, [1]), '')];
    for (let i = 0; i < c.dataCount; i++) {
      if (i % 32 === 31) expected.push(flow);
      expected.push(py.records[i]);
    }
    expected.push(py.end.replace(/ /g, ''));
    let first = -1;
    for (let i = 0; i < Math.max(sent.length, expected.length) && first < 0; i++) {
      if (sent[i] !== expected[i]) first = i;
    }
    check(tag + ': install() sends START, the records with a flow query every 32, END',
      first < 0, sent.length + ' messages, ' + expected.length + ' expected, first difference at ' +
      first);
  })());
}

Promise.all(asyncChecks).then(() => {
  console.log(failures ? failures + ' check(s) failed' : 'all checks passed');
  process.exitCode = failures ? 1 : 0;
}, (e) => {
  console.log('FAIL ' + (e && e.stack ? e.stack : e));
  process.exitCode = 1;
});
