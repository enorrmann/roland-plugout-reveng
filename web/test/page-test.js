/*
 * page-test.js -- drives ../index.html in an iframe with a fake Web MIDI synth
 * (docs/WEB_INSTALLER.md §5). Scenarios:
 *   1. SYSTEM-1: the page finds the synth by itself; wizard: slot 1, a wrongly
 *      named file is refused, Code1.Dat is taken, Install stays locked until the
 *      box is ticked, installs into slot 1 and closes the MIDI ports; the
 *      messages sent are checked one by one.
 *   2. SYSTEM-8 with a SYSTEM-1 build named Code8_1.Dat: the install is blocked.
 * Query string: ?dat=<path from the server root> (default: our C-S1SY13F build).
 * Waits are in the virtual time of headless Chrome (run-page-test.sh), hence the
 * long limits; real time is a few seconds.
 * The verdict goes into <pre id="result">: "PASS ..." or "FAIL ...".
 */
(function () {
  'use strict';

  var DAT = new URLSearchParams(location.search).get('dat') ||
    '/plugins/C-S1SY13F/Script/Code1.Dat';
  var lines = [];
  var failed = 0;

  function check(name, ok, detail) {
    lines.push((ok ? 'ok   ' : 'FAIL ') + name + (ok || !detail ? '' : '\n     ' + detail));
    if (!ok) failed++;
  }

  function finish(extra) {
    if (extra) lines.push(extra);
    var f = document.getElementById('page');
    var d = f && f.contentDocument;
    if (failed && d) {                       // where the page stood
      ['webmidi-state', 'device-state', 'firmware-state', 'install-state', 'log']
        .forEach(function (id) { lines.push('  ' + id + ': ' + text(d, id).slice(-1500)); });
    }
    lines.unshift((failed ? 'FAIL ' + failed + ' check(s)' : 'PASS all page checks'));
    document.getElementById('result').textContent = lines.join('\n');
  }

  // ---- the fake synth, behind a fake MIDIAccess ---------------------------------

  function makeAccess(P, opts) {
    var slots = opts.slots;
    var sent = [];
    var input = { id: 'in-2', name: opts.portName, manufacturer: 'Roland', state: 'connected',
      onmidimessage: null, closed: 0, close: function () { this.closed++; return Promise.resolve(); } };
    var other = { id: 'in-1', name: opts.otherName, manufacturer: 'Roland',
      state: 'connected', onmidimessage: null, close: function () { return Promise.resolve(); } };
    var nibbles = function (data, value, at, digits) {
      for (var i = 0; i < digits; i++) data[at + i] = (value >>> (4 * (digits - 1 - i))) & 0x0f;
    };
    var slotInfo = function (s) {
      var d = new Array(0x30).fill(0);
      nibbles(d, 0x130, 0, 4);
      nibbles(d, s[1], 8, 4);
      nibbles(d, s[2], 12, 4);
      for (var i = 0; i < 16; i++) nibbles(d, i < s[0].length ? s[0].charCodeAt(i) : 0, 16 + i * 2, 2);
      return d;
    };
    var answer = function (address, data) {
      var m = P.buildMessage(0x12, address | P.REPLY_BIT, data, 0x10);
      setTimeout(function () { if (input.onmidimessage) input.onmidimessage({ data: m }); }, 0);
    };
    var output = { id: 'out-2', name: opts.portName, manufacturer: 'Roland', state: 'connected',
      closed: 0, close: function () { this.closed++; return Promise.resolve(); },
      send: function (m) {
        sent.push(Uint8Array.from(m));
        var a = P.addressOf(m);
        var arg = m[11];
        if (opts.silent) return;
        if (a === P.ADDR.DEVICE_TYPE) answer(a, [opts.type]);
        else if (a === P.ADDR.SLOT_INFO) answer(a, slotInfo(slots[arg] || ['', 0xffff, 0xffff]));
        else if (a === 0x0e000101 || a === 0x0e000102) answer(a, [8]);
        else if (a === 0x0e000103) answer(a, [slots.length]);
        else if (a === P.ADDR.START || a === P.ADDR.END) answer(a, [0]);
        else if (a === P.ADDR.FLOW) answer(a, [0, 0, 0, 0]);
      } };
    var outOther = { id: 'out-1', name: other.name, manufacturer: 'Roland', state: 'connected',
      send: function () { throw new Error('the page wrote to the wrong port'); },
      close: function () { return Promise.resolve(); } };
    var access = { sysexEnabled: true, onstatechange: null,
      outputs: new Map([['out-1', outOther], ['out-2', output]]),
      inputs: new Map([['in-1', other], ['in-2', input]]) };
    // As Chrome: opening a port (send, onmidimessage) or closing it changes its
    // `connection` and fires MIDIAccess.onstatechange; `state` stays 'connected'.
    var fire = function (port, connection) {
      if (port.connection === connection) return;
      port.connection = connection;
      setTimeout(function () { if (access.onstatechange) access.onstatechange({ port: port }); }, 0);
    };
    [input, other, output, outOther].forEach(function (port) {
      port.connection = 'closed';
      var close = port.close;
      port.close = function () { fire(port, 'closed'); return close.call(port); };
      if (port.send) {
        var send = port.send;
        port.send = function (m) { fire(port, 'open'); return send.call(port, m); };
      } else {
        var handler = null;
        Object.defineProperty(port, 'onmidimessage', { configurable: true,
          get: function () { return handler; },
          set: function (f) { handler = f; if (f) fire(port, 'open'); } });
      }
    });
    return { access: access, sent: sent, output: output, input: input };
  }

  // ---- load the page with the fake MIDI in place ---------------------------------

  function loadPage(opts) {
    return fetch('../index.html', { cache: 'no-store' }).then(function (r) { return r.text(); })
      .then(function (html) {
        var frame = document.getElementById('page');
        var fake = null;
        window.__makeFake = function (P) {
          fake = makeAccess(P, opts);
          return fake.access;
        };
        var stub = '<base href="../">\n<script>Object.defineProperty(navigator, ' +
          '"requestMIDIAccess", { configurable: true, value: function () { ' +
          'return Promise.resolve(parent.__makeFake(window.Plugout)); } });<\/script>';
        frame.srcdoc = html.replace('<head>', '<head>\n' + stub);
        return new Promise(function (resolve) {
          frame.onload = function () {
            resolve({ doc: frame.contentDocument, win: frame.contentWindow,
              fake: function () { return fake; } });
          };
        });
      });
  }

  function waitFor(fn, what, ms) {
    var t0 = Date.now();
    return new Promise(function (resolve, reject) {
      (function poll() {
        var v;
        try { v = fn(); } catch (e) { v = null; }
        if (v) return resolve(v);
        if (Date.now() - t0 > (ms || 600000)) return reject(new Error('timeout waiting for ' + what));
        setTimeout(poll, 20);
      })();
    });
  }

  function text(doc, id) {
    var el = doc.getElementById(id);
    return el ? el.textContent : '';
  }

  function next(page) {
    page.doc.getElementById('btn-next').click();
  }

  function visible(doc, id) {
    var el = doc.getElementById(id);
    return !!el && !el.classList.contains('hidden');
  }

  function chooseFile(page, bytes, name) {
    var dt = new page.win.DataTransfer();
    dt.items.add(new page.win.File([bytes], name));
    var input = page.doc.getElementById('file-input');
    input.files = dt.files;
    input.dispatchEvent(new page.win.Event('change'));
  }

  // ---- scenarios -------------------------------------------------------------------

  function scenario1(bytes) {
    var slots = [['', 0, 0], ['C-S1SY13', 0x10, 0x0001], ['', 0xffff, 0xffff], ['', 0xffff, 0xffff]];
    return loadPage({ type: 0, portName: 'SYSTEM-1 MIDI 2', otherName: 'SYSTEM-1 MIDI 1', slots: slots }).then(function (page) {
      var doc = page.doc;
      return waitFor(function () { return text(doc, 'device-state') === 'Connected'; },
        'the synth to be found').then(function () {
        var fake = page.fake();
        check('the page found the SYSTEM-1 by itself on its control port',
          /SYSTEM-1 MIDI 2/.test(text(doc, 'device-body')), text(doc, 'device-body'));
        check('the questions closed the MIDI ports again', fake.output.closed >= 1 && fake.input.closed >= 1);
        check('only step 2 is visible', visible(doc, 'card-device') && !visible(doc, 'card-slot'));
        next(page);
        check('step 3 (slot) comes before the file', visible(doc, 'card-slot'));
        check('SYSTEM-1: no slot choice, slot 1',
          doc.getElementById('slot-select').parentNode.classList.contains('hidden') &&
          text(doc, 'slot-state') === 'Slot 1', text(doc, 'slot-state'));
        next(page);
        check('step 4 asks for Code1.Dat', text(doc, 'firmware-name') === 'Code1.Dat',
          text(doc, 'firmware-name'));
        check('Next is locked without a file', doc.getElementById('btn-next').disabled);
        chooseFile(page, bytes, 'Code1 (1).Dat');
        check('a file with another name is refused',
          text(doc, 'firmware-state') === 'Not taken' && doc.getElementById('btn-next').disabled,
          text(doc, 'firmware-body'));
        chooseFile(page, bytes, 'Code1.Dat');
        var name = page.win.Plugout.loadContainer(bytes.buffer, 'x').model;
        return waitFor(function () { return text(doc, 'firmware-state') === name; }, 'the file');
      }).then(function () {
        var model = page.win.Plugout.loadContainer(bytes.buffer, 'x');
        check('step 4 shows the plug-out and its target device',
          text(doc, 'firmware-body').indexOf(model.model) >= 0 &&
          /Built forSYSTEM-1/.test(text(doc, 'firmware-body')), text(doc, 'firmware-body'));
        next(page);
        check('slot 1: the overwrite of C-S1SY13 is announced',
          /Slot 1 holds C-S1SY13.*overwritten/.test(text(doc, 'warn-body')), text(doc, 'warn-body'));
        check('step 5 is the last step: no Next', doc.getElementById('btn-next').classList.contains('hidden'));
        check('Install stays locked until the box is ticked', doc.getElementById('btn-install').disabled);
        var box = doc.getElementById('slot-confirm');
        box.checked = true;
        box.dispatchEvent(new page.win.Event('change'));
        check('ticking the box unlocks Install', visible(doc, 'card-install') &&
          !doc.getElementById('btn-install').disabled);
        var fake = page.fake();
        fake.sent.length = 0;
        var closed = fake.output.closed;
        doc.getElementById('btn-install').click();
        return waitFor(function () {
          var s = text(doc, 'install-state');
          return s === 'Done' || s === 'Failed' ? s : null;
        }, 'the install').then(function (s) {
          check('the install finished', s === 'Done', text(doc, 'install-body'));
          var P = page.win.Plugout;
          var sent = fake.sent;
          var expected = [P.hex(P.withData(model.startMessage, [1]))];
          for (var i = 0; i < model.dataCount; i++) {
            if (i % 32 === 31) expected.push(P.hex(P.buildMessage(0x12, P.ADDR.FLOW, [0])));
            expected.push(P.hex(model.dataMessage(i)));
          }
          expected.push(P.hex(model.endMessage));
          var first = -1;
          for (var k = 0; k < Math.max(sent.length, expected.length) && first < 0; k++) {
            if (!sent[k] || P.hex(sent[k]) !== expected[k]) first = k;
          }
          check('the page sent START (slot 1), every record with the flow queries, END, nothing else',
            first < 0, sent.length + ' sent, ' + expected.length + ' expected, first difference at ' +
            first + (first >= 0 ? ':\n     sent     ' + (sent[first] ? P.hex(sent[first]).slice(0, 90) : '-') +
              '\n     expected ' + (expected[first] || '-').slice(0, 90) : ''));
          check('the MIDI ports are closed after the install', fake.output.closed > closed);
          check('the page says to pick PLG1', /PLG1/.test(text(doc, 'install-body')));
        });
      });
    });
  }

  function scenario2(bytes) {
    var slots = [['', 0, 0], ['JX-3P', 0x08, 0x14fa], ['', 0xffff, 0xffff], ['', 0xffff, 0xffff]];
    return loadPage({ type: 2, portName: 'SYSTEM-8 CTRL', otherName: 'SYSTEM-8', slots: slots }).then(function (page) {
      var doc = page.doc;
      return waitFor(function () { return text(doc, 'device-state') === 'Connected'; },
        'the SYSTEM-8').then(function () {
        next(page);
        var sel = doc.getElementById('slot-select');
        sel.value = '1';
        sel.dispatchEvent(new page.win.Event('change'));
        next(page);
        check('SYSTEM-8 slot 1 asks for Code8_1.Dat', text(doc, 'firmware-name') === 'Code8_1.Dat',
          text(doc, 'firmware-name'));
        chooseFile(page, bytes, 'Code8_1.Dat');
        var name = page.win.Plugout.loadContainer(bytes.buffer, 'x').model;
        return waitFor(function () { return text(doc, 'firmware-state') === name; }, 'the file')
          .then(function () {
            next(page);
            return text(doc, 'install-state') === 'Blocked';
          });
      }).then(function (blocked) {
        check('a SYSTEM-1 file on a SYSTEM-8 is blocked',
          blocked && /built for a SYSTEM-1/.test(text(doc, 'warn-body')), text(doc, 'warn-body'));
        check('blocked: Install stays disabled', doc.getElementById('btn-install').disabled);
        var sent = page.fake().sent.filter(function (m) {
          return page.win.Plugout.addressOf(m) === page.win.Plugout.ADDR.START;
        });
        check('blocked: no START was sent', sent.length === 0);
      });
    });
  }

  /** A synth that never answers: the page asks a few times, then stops (no loop). */
  function scenario3() {
    var slots = [['', 0, 0], ['', 0xffff, 0xffff]];
    return loadPage({ type: 0, portName: 'SYSTEM-1 MIDI 2', otherName: 'SYSTEM-1 MIDI 1',
      slots: slots, silent: true }).then(function (page) {
      var doc = page.doc;
      // 5 rounds x 3 attempts x 5 s + 4 pauses of 2 s = 83 s; wait well past that
      return new Promise(function (resolve) { setTimeout(resolve, 300000); }).then(function () {
        var P = page.win.Plugout;
        var asked = page.fake().sent.filter(function (m) { return P.addressOf(m) === P.ADDR.DEVICE_TYPE; });
        check('a silent synth is asked at most 5 rounds x 3 attempts (no open/close loop)',
          asked.length <= 15, asked.length + ' questions');
        check('then the page waits for the button', /Click Find synth/.test(text(doc, 'device-body')),
          text(doc, 'device-body'));
      });
    });
  }

  fetch(DAT, { cache: 'no-store' }).then(function (r) {
    if (!r.ok) throw new Error(DAT + ': HTTP ' + r.status);
    return r.arrayBuffer();
  }).then(function (buf) {
    var bytes = new Uint8Array(buf);
    return scenario1(bytes).then(function () { return scenario2(bytes); }).then(scenario3);
  }).then(function () { finish(); }, function (e) {
    failed++;
    finish('FAIL ' + (e && e.stack ? e.stack : e));
  });
})();
