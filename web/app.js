/*
 * app.js -- the PLUG-OUT installer page (docs/WEB_INSTALLER.md).
 *
 * A wizard: one step visible at a time, Back / Next below it.
 *   1. Web MIDI with SysEx available and granted,
 *   2. the synth connected over USB MIDI and answering (SYSTEM-8 or SYSTEM-1),
 *   3. the target slot (it decides the file name on a SYSTEM-8),
 *   4. the firmware file, chosen or dropped on the page; only the exact name
 *      the plug-in would pick is taken: Code1.Dat (SYSTEM-1) or Code8_<slot>.Dat
 *      (SYSTEM-8); never a default file,
 *   5. what gets overwritten, confirmed (and the checks that block the install),
 *      and the install on the same step: START + the firmware records + END, as
 *      `plugout_tool.py install --no-patches --force`; after the last record
 *      the page says it waits for the synth's answer to END.
 *
 * The MIDI protocol itself lives in plugout-protocol.js; this file only drives
 * the page. Between the questions and after the install the page closes the
 * synth's MIDI ports, so the command line tools can use the synth while the
 * page stays open.
 */
(function () {
  'use strict';

  var P = window.Plugout;
  var RETRY_MS = 2000;              // pause between automatic questions to the synth
  var AUTO_TRIES = 5;               // automatic questions before waiting for the button

  var $ = function (id, root) { return (root || document).getElementById(id); };

  var state = {
    support: P.webMidiSupport(),
    access: null,
    ports: [],          // [{id, out, in, synth}] candidates, best first
    portIndex: 0,
    manualPort: false,  // the user picked the port under Advanced: keep it
    probing: false,     // a question round is running
    askAgain: false,    // the ports changed during that round: ask again after it
    info: null,         // answer of deviceInfo()
    step: 1,
    slot: 1,
    plan: null,
    firmware: null,     // {container, name, bytes}
    firmwareError: null,
    installing: false,
    done: false,
    logLines: [],
    tries: 0,
    retryTimer: null,
  };

  // -------------------------------------------------------------------------
  // small DOM helpers
  // -------------------------------------------------------------------------

  function setState(id, text, cls) {
    var el = $(id);
    if (!el) return;
    el.textContent = text;
    el.className = 'state' + (cls ? ' ' + cls : '');
  }

  function show(id, html) {
    var el = $(id);
    if (el) el.innerHTML = html;
  }

  function esc(s) {
    return String(s === undefined || s === null ? '' : s)
      .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;');
  }

  function log(line) {
    var t = new Date().toLocaleTimeString();
    state.logLines.push(t + '  ' + line);
    if (state.logLines.length > 600) state.logLines.splice(0, state.logLines.length - 600);
    var pre = $('log');
    if (pre) {
      pre.textContent = state.logLines.join('\n');
      pre.scrollTop = pre.scrollHeight;
    }
  }

  function portName(p) {
    if (!p) return '(none)';
    return p.name + (p.manufacturer ? '  [' + p.manufacturer + ']' : '');
  }

  function hexId(v) {
    return '0x' + (v >>> 0).toString(16).toUpperCase().padStart(2, '0');
  }

  // -------------------------------------------------------------------------
  // 1. Web MIDI
  // -------------------------------------------------------------------------

  function checkWebMidi() {
    var s = state.support;
    if (!s.supported) {
      var why = s.reason === 'insecure'
        ? 'This page is not on a secure origin (it was opened as a <code>file://</code> ' +
          'page or from a plain-IP address), so the browser will not give it MIDI. ' +
          'Serve the folder locally, for example with <code>python3 -m http.server 8000</code> ' +
          'in <code>web/</code>, and open <code>http://localhost:8000/</code>.'
        : 'This browser has no Web MIDI. Use Chrome, Edge or another Chromium browser.';
      setState('webmidi-state', 'Not available', 'bad');
      show('webmidi-body',
        '<p class="tight">Web MIDI with SysEx is not available here.</p>' +
        '<div class="badbox">' + why + '</div>');
      $('btn-midi').disabled = true;
      return;
    }
    if (state.access) return;
    setState('webmidi-state', 'Permission needed', 'warn');
    show('webmidi-body',
      '<p class="tight">Web MIDI + SysEx has not been enabled yet.</p>' +
      '<p class="tight">When you click the button the browser asks for permission. ' +
      'Allow it, including the SysEx half (&ldquo;Send and receive SysEx&rdquo; in Chrome). ' +
      'The permission is remembered for this page.</p>');
  }

  function enableMidi() {
    $('btn-midi').disabled = true;
    $('midi-hint').innerHTML = '<span class="spin"></span> asking the browser…';
    P.requestMidi().then(grantAccess, function (err) {
      setState('webmidi-state', 'Refused', 'bad');
      show('webmidi-body',
        '<p class="tight">The page was not allowed to use MIDI with SysEx.</p>' +
        '<div class="badbox">' + esc(err.message) + '</div>' +
        '<p class="tight">Click the button again, or open the site settings of this page ' +
        'and clear the MIDI block.</p>');
      $('midi-hint').textContent = '';
      $('btn-midi').disabled = false;
    });
  }

  /** The ports that exist (id + state), not whether they are open. */
  function portSignature(access) {
    return P.listOutputs(access).concat(P.listInputs(access)).map(function (p) {
      return p.id + '=' + p.state;
    }).sort().join(' ');
  }

  /**
   * Keep the MIDI access: a cable plugged in or a synth switched on shows up.
   * Chrome also fires statechange when a port is merely opened or closed, which
   * every question round does: those events are ignored (docs/WEB_INSTALLER.md §4),
   * or a synth that does not answer would be asked again and again.
   */
  function grantAccess(access) {
    state.access = access;
    var known = portSignature(access);
    P.watchAccess(access, function () {
      var now = portSignature(access);
      if (now === known) return;
      known = now;
      refreshPorts();
      lostDevice();
      if (!state.info) {
        state.tries = 0;
        maybeConnect();
      }
    });
    onMidiReady();
  }

  /** The synth that answered is gone (cable out, switched off): forget it. */
  function lostDevice() {
    if (!state.info || state.installing) return;
    var still = state.ports.some(function (p) {
      return p.id === state.info.portId && p.out.state !== 'disconnected';
    });
    if (still) return;
    log('the synth\'s port is gone');
    state.info = null;
    setState('device-state', 'Disconnected', 'warn');
    show('device-body', '<p class="tight">The synth is no longer connected. Connect it again; ' +
      'the page notices it by itself.</p>');
    goTo(2);
  }

  function onMidiReady() {
    var st = P.accessState(state.access);
    $('btn-midi').disabled = true;
    $('midi-hint').textContent = '';
    if (st === 'best') {
      setState('webmidi-state', 'Ready, with SysEx', 'ok');
      show('webmidi-body',
        '<p class="tight">Web MIDI is allowed and SysEx is enabled. That is everything ' +
        'the installer needs from the browser.</p>');
      refreshPorts();
      if (state.step === 1) goTo(2);
      maybeConnect();
    } else {
      setState('webmidi-state', 'No SysEx', 'bad');
      show('webmidi-body',
        '<p class="tight">Web MIDI is on, but the browser did not give the page SysEx ' +
        'access.</p><div class="badbox">Without SysEx the page cannot talk to the synth. ' +
        'Chrome asks for it in the permission pop-up (&ldquo;Send and receive SysEx&rdquo;); ' +
        'if it was refused, open this page\'s site settings, remove the MIDI permission ' +
        'and reload the page.</div>');
    }
    updateNav();
  }

  // -------------------------------------------------------------------------
  // 2. the device
  // -------------------------------------------------------------------------

  /**
   * Candidates, best first: the synth's control port (same rule as
   * tools/midi_io.py), then the other ports. Only ports that name a SYSTEM-8 /
   * SYSTEM-1 are asked automatically; any other port only when picked by hand.
   */
  function refreshPorts() {
    var sel = $('port-select');
    var found = P.findDevicePorts(state.access, false);
    var outs = P.listOutputs(state.access);
    var ins = P.listInputs(state.access);
    // Keep the port the user picked, or the one the synth answered on. Otherwise
    // take the best candidate again: a synth plugged in after the page opened
    // must win over the "Midi Through" port that was alone before.
    var keep = state.manualPort && currentPort() ? currentPort().id
      : state.info ? state.info.portId : null;
    var used = {};
    state.ports = [];
    var add = function (out) {
      if (!out || used[out.id]) return;
      used[out.id] = true;
      var i = ins.find(function (p) {
        return p.name === out.name && p.manufacturer === out.manufacturer;
      }) || ins.find(function (p) { return p.name === out.name; });
      state.ports.push({ id: out.id, out: out, in: i || null, synth: P.isSynthPort(out.name) });
    };
    if (found.output) add(found.output);
    outs.forEach(add);

    sel.innerHTML = '';
    if (!state.ports.length) {
      var o = document.createElement('option');
      o.value = '';
      o.textContent = state.access
        ? '(no MIDI output found — is the synth connected and switched on?)'
        : '(enable MIDI first)';
      sel.appendChild(o);
      $('btn-find').disabled = true;
      if (!state.info) {
        setState('device-state', 'No MIDI port', 'warn');
        show('device-body',
          '<p class="tight">No MIDI port yet. Connect the synth with its USB cable and ' +
          'switch it on; the page notices it by itself.</p>');
      }
      return;
    }
    state.ports.forEach(function (p, i) {
      var o = document.createElement('option');
      o.value = String(i);
      o.textContent = portName(p.out) + (!p.in ? '  (no input — cannot read answers)'
        : !p.synth ? '' : P.isControlName(p.out.name) ? '  (control port — use this one)'
        : '  (not the control port)');
      sel.appendChild(o);
    });
    var at = state.ports.findIndex(function (p) { return p.id === keep; });
    state.portIndex = at >= 0 ? at : 0;
    sel.value = String(state.portIndex);
    $('btn-find').disabled = state.installing;
  }

  function currentPort() {
    return state.ports[state.portIndex] || null;
  }

  /** The byte rate of the install: P.WEB_BYTES_PER_SECOND, or ?rate=<bytes per second>. */
  function rate() {
    var r = Number(new URLSearchParams(location.search).get('rate'));
    return r > 0 ? r : P.WEB_BYTES_PER_SECOND;
  }

  /**
   * Where the answers are heard: the input of the chosen port plus every other
   * input of the synth (the answer may come back on the other USB port).
   */
  function listenInputs(port) {
    var list = [port.in.port];
    P.listInputs(state.access).forEach(function (i) {
      if (P.isSynthPort(i.name) && i.port !== port.in.port) list.push(i.port);
    });
    return list;
  }

  /** One question round: open the port, ask the device, close the port again. */
  function probe(port) {
    var session = new P.Port(port.out.port, listenInputs(port), { log: log });
    log('asking ' + portName(port.out));
    return session.deviceInfo(function (step) { log(step); })
      .then(function (info) {
        session.close();
        info.portName = portName(port.out);
        info.portId = port.id;
        return info;
      }, function (err) {
        session.close();
        throw err;
      });
  }

  function findSynth() {
    var port = currentPort();
    clearTimeout(state.retryTimer);
    state.retryTimer = null;
    if (!port) {
      setState('device-state', 'No MIDI port', 'warn');
      return;
    }
    if (!port.in) {
      setState('device-state', 'No answer possible', 'bad');
      show('device-body',
        '<div class="badbox">This MIDI output has no matching input, so the page cannot ' +
        'hear the synth\'s answers. Choose the synth\'s control port (<code>CTRL</code>, ' +
        'or the second SYSTEM port, e.g. <code>SYSTEM-1 MIDI 2</code>) under <em>Advanced</em>.</div>');
      return;
    }
    if (state.probing) {                 // one round at a time; ask again after it
      state.askAgain = true;
      return;
    }
    state.info = null;
    state.probing = true;
    state.askAgain = false;
    $('btn-find').disabled = true;
    setState('device-state', 'Asking…', 'warn');
    show('device-body', '<p class="tight"><span class="spin"></span> Asking ' +
      esc(portName(port.out)) + ' what it is…</p>');
    probe(port).then(function (info) {
      state.probing = false;
      acceptDevice(info);
    }, function (err) {
      state.probing = false;
      rejectDevice(port, err);
      if (state.askAgain) {               // the ports changed meanwhile (synth plugged in)
        state.askAgain = false;
        state.tries = 0;
        clearTimeout(state.retryTimer);
        maybeConnect();
      }
    });
  }

  function acceptDevice(info) {
    state.info = info;
    state.tries = 0;
    $('btn-find').disabled = false;
    setState('device-state', 'Connected', 'ok');
    show('device-body',
      '<dl class="kv">' +
      '<dt>Device</dt><dd>' + esc(info.deviceName) + '</dd>' +
      '<dt>Firmware</dt><dd>' + P.versionStr(info.firmware) +
      (info.firmware < info.minFirmware ? ' <strong>(too old: ' +
        P.versionStr(info.minFirmware) + ' or later needed)</strong>' : '') + '</dd>' +
      '<dt>Port</dt><dd class="mono">' + esc(info.portName) +
      (P.isControlName(currentPort() && currentPort().out.name) ? ' (control port)' : '') + '</dd>' +
      '<dt>Slots</dt><dd>' + info.slots.length + ' (slot 0 is the synth itself)</dd>' +
      '</dl>' + (P.isControlName(currentPort() && currentPort().out.name) ? '' :
      '<div class="warnbox"><strong>This is not the synth\'s control port.</strong> It answers ' +
      'the questions, but during the install it answers the flow control only after resends ' +
      '(about 10 s every 32 records). Pick the port ending in <code>2</code> or ' +
      '<code>CTRL</code> under <em>Advanced</em> below.</div>'));
    if (!P.isControlName(currentPort() && currentPort().out.name)) $('port-details').open = true;
    log('device: ' + info.deviceName + ', firmware ' + P.versionStr(info.firmware) +
        ', ' + info.slots.length + ' slot(s)');
    $('device-badge').textContent = info.deviceName;
    if (singleSlot(info)) state.slot = 1;
    else if (!(state.slot >= 1 && state.slot < info.slots.length)) state.slot = slotSuggestion(info);
    dropWrongFile();
    updateNav();
  }

  function rejectDevice(port, err) {
    state.info = null;
    $('btn-find').disabled = false;
    state.tries++;
    var again = port.synth && state.tries < AUTO_TRIES;
    setState('device-state', 'No answer', 'warn');
    show('device-body',
      '<div class="badbox"><strong>The synth did not answer</strong> on ' +
      esc(portName(port.out)) + '.<br>' + esc(err.message) + '</div>' +
      '<p class="tight">Check that the cable is in, the synth is on, and that no other ' +
      'program (a plug-in, a DAW, <code>plugout_tool.py</code>, another browser tab) is ' +
      'using the synth\'s MIDI port. ' + (again ? 'The page asks again in a moment, or click '
        : 'Click ') + '<em>Find synth</em>.</p>');
    log('no answer: ' + err.message);
    if (again) state.retryTimer = setTimeout(maybeConnect, RETRY_MS);
    updateNav();
  }

  /** Ask by itself, only on a port that names the synth, a few times at most. */
  function maybeConnect() {
    state.retryTimer = null;
    if (!state.access || state.installing || state.info) return;
    var port = currentPort();
    if (!port || !port.synth || state.tries >= AUTO_TRIES) return;
    findSynth();
  }

  // -------------------------------------------------------------------------
  // wizard: one step at a time
  // -------------------------------------------------------------------------

  var LAST_STEP = 5;

  /** Can the user leave this step with Next? '' = yes, otherwise why not. */
  function nextBlocked(step) {
    var info = state.info;
    switch (step) {
      case 1: return P.accessState(state.access) === 'best' ? '' : 'Enable MIDI with SysEx first.';
      case 2:
        if (!info) return 'Waiting for the synth to answer.';
        if (info.firmware < info.minFirmware) return 'The synth\'s firmware is too old.';
        return '';
      case 3: return info && state.slot >= 1 && state.slot < info.slots.length ? '' : 'Choose a slot.';
      case 4: return state.firmware ? '' : 'Choose the file ' + expectedName() + '.';
      default: return '';
    }
  }

  function goTo(step) {
    if (state.installing) return;
    state.step = step;
    document.querySelectorAll('section.card[data-step]').forEach(function (el) {
      el.classList.toggle('hidden', Number(el.dataset.step) !== step);
    });
    document.querySelectorAll('#stepper li').forEach(function (li) {
      var n = Number(li.dataset.step);
      li.className = n === step ? 'current' : n < step ? 'done' : '';
    });
    if (step === 3) showSlots();
    if (step === 4) showFirmware();
    if (step === 5 && !state.done) {    // after an install: keep the result on the page
      showWarning();
      showInstallInfo();
    }
    updateNav();
  }

  function updateNav() {
    var step = state.step;
    var why = nextBlocked(step);
    $('btn-back').classList.toggle('hidden', step === 1);
    $('btn-back').disabled = state.installing;
    $('btn-next').classList.toggle('hidden', step === LAST_STEP);
    $('btn-next').disabled = !!why || state.installing;
    $('nav-hint').textContent = step === LAST_STEP ? '' : why;
    if (step === LAST_STEP) updateInstallButton();
  }

  // -------------------------------------------------------------------------
  // 3. slot
  // -------------------------------------------------------------------------

  /** First empty user slot, else slot 1. */
  function slotSuggestion(info) {
    var free = info.slots.find(function (s) { return s.index !== 0 && s.empty; });
    return free ? free.index : 1;
  }

  /** A SYSTEM-1 / 1m has one PLUG-OUT slot: no choice, slot 1. */
  function singleSlot(info) {
    return !!info && (info.type === 0 || info.type === 1);
  }

  function showSlots() {
    var info = state.info;
    if (!info) return;
    var one = singleSlot(info);
    var sel = $('slot-select');
    sel.parentNode.classList.toggle('hidden', one);
    sel.innerHTML = '';
    for (var i = 1; i < (one ? 2 : info.slots.length); i++) {
      var o = document.createElement('option');
      o.value = String(i);
      o.textContent = 'Slot ' + i + ' — now: ' + P.describeSlot(info.slots[i]);
      sel.appendChild(o);
    }
    sel.value = String(state.slot);
    setState('slot-state', 'Slot ' + state.slot, 'ok');
    var rows = info.slots.filter(function (s) { return !one || s.index < 2; }).map(function (s) {
      var label = s.index === 0 ? '0 (the synth itself)' : String(s.index);
      return '<tr' + (s.index === state.slot ? ' class="slot-target"' : '') + '><td>' +
        esc(label) + '</td><td>' + esc(P.describeSlot(s)) + '</td>' +
        '<td class="slot-new">' + (s.index === state.slot ? 'the new PLUG-OUT' : '') + '</td></tr>';
    }).join('');
    show('slot-body', '<table><thead><tr><th>Slot</th><th>Now</th><th>After the install</th>' +
      '</tr></thead><tbody>' + rows + '</tbody></table>' +
      (one ? '<p class="tight">The ' + esc(info.deviceName) + ' has one PLUG-OUT slot: the ' +
        'PLUG-OUT goes into slot 1.</p>' : '') +
      '<p class="small">Next you choose the file <code>' + esc(expectedName()) + '</code>.</p>');
  }

  // -------------------------------------------------------------------------
  // 4. the firmware file
  // -------------------------------------------------------------------------

  /** The only file name taken: Code1.Dat (SYSTEM-1/1m) or Code8_<slot>.Dat (SYSTEM-8). */
  function expectedName() {
    if (!state.info) return 'Code1.Dat';
    return P.containerPath(state.info.type, state.slot).replace(/^.*\//, '');
  }

  /** A file chosen for another slot (SYSTEM-8) no longer fits: forget it. */
  function dropWrongFile() {
    if (state.firmware && state.firmware.name !== expectedName()) {
      log('firmware: ' + state.firmware.name + ' is not ' + expectedName() + ', dropped');
      state.firmware = null;
      state.firmwareError = null;
    }
    state.done = false;
  }

  function takeFirmware(buffer, name) {
    state.firmware = null;
    state.firmwareError = null;
    try {
      var c = P.loadContainer(buffer, name);
      state.firmware = { container: c, name: name, bytes: buffer.byteLength };
      log('firmware: ' + name + ': ' + c.model + ' ' + P.versionStr(c.version) + ', ' +
          c.dataCount + ' records, ' + Math.round(c.firmwareBytes / 1024) + ' KB, built for ' +
          (c.target || 'an unknown device'));
    } catch (err) {
      state.firmwareError = err;
      log('firmware: ' + name + ': ' + err.message);
    }
    state.done = false;
    showFirmware();
    updateNav();
  }

  function readFile(file) {
    if (!file) return;
    var want = expectedName();
    if (file.name !== want) {
      state.firmware = null;
      state.firmwareError = new Error('The file is called ' + file.name + '; for ' +
        (state.info ? state.info.deviceName + ' slot ' + state.slot : 'this synth') +
        ' only ' + want + ' is taken.');
      log('firmware: ' + file.name + ' refused (not ' + want + ')');
      showFirmware();
      updateNav();
      return;
    }
    var reader = new FileReader();
    reader.onload = function () { takeFirmware(reader.result, file.name); };
    reader.onerror = function () {
      state.firmware = null;
      state.firmwareError = new Error('the browser could not read ' + file.name);
      showFirmware();
      updateNav();
    };
    reader.readAsArrayBuffer(file);
  }

  function showFirmware() {
    var want = expectedName();
    $('firmware-name').textContent = want;
    if (state.firmwareError) {
      setState('firmware-state', 'Not taken', 'bad');
      show('firmware-body', '<div class="badbox"><strong>This file cannot be installed.' +
        '</strong><br>' + esc(state.firmwareError.message) + '</div>' +
        '<p class="tight">Pick <code>' + esc(want) + '</code> from the <code>Script</code> ' +
        'folder of a plug-in (e.g. <code>plugins/&lt;NAME&gt;/Script/' + esc(want) + '</code>).</p>');
      return;
    }
    if (!state.firmware) {
      setState('firmware-state', 'Choose a file', 'warn');
      show('firmware-body', '<p class="tight">No file chosen yet.</p>');
      return;
    }
    var f = state.firmware;
    var c = f.container;
    setState('firmware-state', c.model || 'ready', 'ok');
    show('firmware-body',
      '<dl class="kv">' +
      '<dt>File</dt><dd class="mono">' + esc(f.name) + '</dd>' +
      '<dt>Plug-out</dt><dd>' + (esc(c.model) || '&mdash;') + ' (model id ' + hexId(c.modelId) +
      ', version ' + P.versionStr(c.version) + ')</dd>' +
      '<dt>Built for</dt><dd>' + (c.target ? esc(c.target) : 'unknown (no region header)') +
      ', ' + c.regions.length + ' region(s)</dd>' +
      '<dt>Data records</dt><dd>' + c.dataCount + ' (' + Math.round(c.firmwareBytes / 1024) +
      ' KB of firmware)</dd>' +
      '</dl>');
  }

  // -------------------------------------------------------------------------
  // 5. checks and warning
  // -------------------------------------------------------------------------

  /** Reasons the install must not start (as plugout_tool.py and the plug-in). */
  function blockers(plan, c) {
    var info = state.info;
    var out = [];
    if (plan.firmwareTooOld) {
      out.push('The synth runs firmware ' + P.versionStr(info.firmware) + '; PLUG-OUTs need ' +
        P.versionStr(info.minFirmware) + ' or later. Update the synth first.');
    }
    if (plan.wrongDevice) {
      out.push('This file is built for a ' + esc(c.target) + ', the synth is a ' +
        esc(info.deviceName) + '. Pick the ' + esc(expectedName()) + ' of a ' +
        esc(info.deviceName) + ' plug-in.');
    }
    return out;
  }

  function showWarning() {
    var c = state.firmware.container;
    var plan = P.planInstall(state.info, c, state.slot);
    state.plan = plan;
    plan.blockers = blockers(plan, c);

    var model = esc(c.model) || 'the PLUG-OUT';
    var html = '<dl class="kv">' +
      '<dt>Firmware</dt><dd>' + model + ' ' + P.versionStr(c.version) + ' from <code>' +
      esc(state.firmware.name) + '</code></dd>' +
      '<dt>Target</dt><dd>' + esc(state.info.deviceName) + ', slot ' + state.slot + '</dd>' +
      '<dt>In the slot now</dt><dd>' + esc(plan.current) + '</dd>' +
      '<dt>Will be sent</dt><dd>' + c.dataCount + ' data records (' +
      Math.round(c.firmwareBytes / 1024) + ' KB). No patch bank: the plug-out keeps the ' +
      'patches the slot has.</dd></dl>';

    if (plan.blockers.length) {
      html += plan.blockers.map(function (b) {
        return '<div class="badbox"><strong>Cannot install:</strong> ' + b + '</div>';
      }).join('');
      show('warn-body', html);
      setState('install-state', 'Blocked', 'bad');
      return;
    }
    if (plan.unknownTarget) {
      html += '<div class="warnbox"><strong>The file does not say which synth it is for' +
        '</strong> (its regions have no <code>SYSTEM-x:Plugout</code> header). Install it only ' +
        'if you know it is a ' + esc(state.info.deviceName) + ' build.</div>';
    }
    if (plan.state === 'same') {
      // the same build again: nothing to warn about
    } else if (plan.state === 'other-version') {
      html += '<div class="warnbox"><strong>Another version of ' + model + ' is in slot ' +
        state.slot + '</strong> (' + esc(plan.current) + '). It is replaced.</div>';
    } else if (plan.state === 'empty') {
      html += '<div class="okbox">Slot ' + state.slot + ' is empty: nothing gets lost.</div>';
    } else {
      html += '<div class="warnbox"><strong>Slot ' + state.slot + ' holds ' +
        esc(plan.current) + '. It will be overwritten</strong> by ' + model + '. To get it ' +
        'back later, install it again (Roland\'s plug-in or this page).</div>';
    }
    html += '<label class="check"><input type="checkbox" id="slot-confirm">' +
      '<span>' + (plan.state === 'empty' ? 'Put ' + model + ' into the empty slot ' + state.slot
        : 'Replace ' + esc(plan.current) + ' in slot ' + state.slot + ' with ' + model) +
      '.</span></label>';
    show('warn-body', html);
    setState('install-state', 'Read and tick the box', 'warn');
    $('slot-confirm').addEventListener('change', function () {
      setState('install-state', this.checked ? 'Ready to install' : 'Read and tick the box',
        this.checked ? 'ok' : 'warn');
      updateInstallButton();
    });
  }

  // -------------------------------------------------------------------------
  // 5. install (same step)
  // -------------------------------------------------------------------------

  function showInstallInfo() {
    $('progress').classList.add('hidden');
    $('progress-text').textContent = '';
    $('btn-again').classList.add('hidden');
    $('btn-install').classList.remove('hidden');
    show('install-body', state.plan.blockers.length ? '' : '<p class="small">After the last record the page waits for the ' +
      'synth to confirm (a few seconds). Do not unplug the synth and do not switch it off ' +
      'until the page says it is done.</p>');
    updateInstallButton();
  }

  function updateInstallButton() {
    var check = $('slot-confirm');
    var ready = !!(state.info && state.plan && !state.plan.blockers.length && state.firmware &&
      check && check.checked && currentPort() && !state.installing && !state.done);
    $('btn-install').disabled = !ready;
    $('install-hint').textContent = state.installing ? 'transfer in progress…'
      : state.plan && state.plan.blockers.length ? 'Blocked: see above.'
      : ready || state.done ? '' : 'Tick the box to unlock Install.';
  }

  function install() {
    var p = currentPort();
    var c = state.firmware && state.firmware.container;
    if (!p || !p.in || !c || !state.plan || state.plan.blockers.length) {
      setState('install-state', 'No port', 'bad');
      return;
    }
    clearTimeout(state.retryTimer);
    var port = new P.Port(p.out.port, listenInputs(p), { log: log, bytesPerSecond: rate() });
    log('installing ' + c.model + ' into slot ' + state.slot + ' on ' + portName(p.out));
    state.installing = true;
    updateNav();
    $('btn-find').disabled = true;
    $('file-input').disabled = true;
    $('slot-select').disabled = true;
    if ($('slot-confirm')) $('slot-confirm').disabled = true;
    $('progress').classList.remove('hidden');
    $('progress').value = 0;
    $('progress').max = c.dataCount;
    setState('install-state', 'Sending', 'warn');
    show('install-body', '<p class="tight"><span class="spin"></span> Sending ' +
      (esc(c.model) || 'the PLUG-OUT') + ' to slot ' + state.slot + '. Do not unplug or switch ' +
      'off the synth.</p>');
    var t0 = Date.now();
    var lastShown = 0;
    var waiting = false;

    port.install(c, state.slot, 0, function (done, total) {
      $('progress').value = done;
      if (done === total) {
        if (waiting) return;
        waiting = true;
        log('all ' + total + ' records sent; waiting for the synth to confirm');
        setState('install-state', 'Waiting for the synth', 'warn');
        $('progress-text').textContent = 'all ' + total + ' records sent';
        show('install-body', '<div class="waitbox"><span class="spin"></span> <strong>All data ' +
          'sent. Waiting for the synth to confirm…</strong><br>The synth checks and stores the ' +
          'PLUG-OUT now; the page waits for its answer (up to 20 s). Do not unplug or switch ' +
          'off the synth.</div>');
        return;
      }
      var now = Date.now();
      if (now - lastShown > 150) {
        lastShown = now;
        var r = done / Math.max(0.001, (now - t0) / 1000);
        $('progress-text').textContent = 'record ' + done + ' / ' + total + '   (' +
          Math.round(r) + '/s, about ' + Math.round((total - done) / Math.max(r, 0.001)) +
          ' s left)';
      }
    }).then(function () {
      port.close();
      state.installing = false;
      state.done = true;
      var secs = ((Date.now() - t0) / 1000).toFixed(1);
      log('firmware accepted by the device after ' + secs + ' s; MIDI port closed');
      setState('install-state', 'Done', 'ok');
      $('progress-text').textContent = 'finished in ' + secs + ' s';
      show('warn-body', '');
      show('install-body',
        '<div class="okbox"><strong>The synth confirmed ' + esc(c.model || 'the PLUG-OUT') +
        '.</strong> It is now in slot ' + state.slot + '.</div>' +
        '<p class="tight">On the synth: press <strong>PLUG-OUT</strong> and pick ' +
        '<strong>PLG' + state.slot + '</strong>. If the panel still shows the old plug-out, ' +
        'switch the synth off and on again and select PLG' + state.slot + ' again.</p>');
      finishInstall();
    }, function (err) {
      port.close();
      state.installing = false;
      log('ERROR: ' + err.message);
      setState('install-state', 'Failed', 'bad');
      $('progress-text').textContent = '';
      show('install-body',
        '<div class="badbox"><strong>The install stopped.</strong><br>' + esc(err.message) +
        '</div><p class="tight">Check the cable, close other programs that use the synth\'s ' +
        'MIDI port, then tick the box and install again (or go back to step 2 and <em>Find ' +
        'synth</em>). A slot ' +
        'is written whole, so a second run replaces whatever the failed one left in it.</p>');
      finishInstall();
    });
  }

  function finishInstall() {
    $('btn-find').disabled = false;
    $('file-input').disabled = false;
    $('slot-select').disabled = false;
    if ($('slot-confirm')) {
      $('slot-confirm').disabled = false;
      $('slot-confirm').checked = false;
    }
    $('btn-install').classList.toggle('hidden', state.done);
    $('btn-again').classList.toggle('hidden', !state.done);
    updateNav();
  }

  /** After an install the slot list is stale: ask the synth again, start at step 2. */
  function installAnother() {
    state.done = false;
    state.firmware = null;
    state.firmwareError = null;
    state.plan = null;
    state.tries = 0;
    goTo(2);
    findSynth();
  }

  // -------------------------------------------------------------------------
  // wiring
  // -------------------------------------------------------------------------

  function init() {
    goTo(1);
    $('btn-midi').addEventListener('click', enableMidi);
    $('btn-midi').disabled = !state.support.supported;
    $('btn-back').addEventListener('click', function () {
      if (state.step > 1) goTo(state.step - 1);
    });
    $('btn-next').addEventListener('click', function () {
      if (!nextBlocked(state.step) && state.step < LAST_STEP) goTo(state.step + 1);
    });
    $('btn-find').addEventListener('click', function () {
      state.tries = 0;
      findSynth();
    });
    $('port-select').addEventListener('change', function () {
      state.portIndex = Number(this.value) || 0;
      state.manualPort = true;
      state.tries = 0;
      findSynth();
    });
    $('slot-select').addEventListener('change', function () {
      state.slot = Number(this.value) || 1;
      dropWrongFile();
      showSlots();
      updateNav();
    });
    // Where the browser has it (Chrome, Edge), the File System Access picker
    // offers only .Dat files, without "all files". It cannot filter by exact
    // name either; readFile() checks that. Brave ships it switched off: then
    // the plain <input type=file>, whose filter the user can switch away.
    $('file-btn').addEventListener('click', function (e) {
      if (typeof window.showOpenFilePicker !== 'function' || $('file-input').disabled) return;
      e.preventDefault();
      window.showOpenFilePicker({
        multiple: false,
        excludeAcceptAllOption: true,
        types: [{ description: 'PLUG-OUT firmware (' + expectedName() + ')',
          accept: { 'application/octet-stream': ['.Dat'] } }],
      }).then(function (handles) { return handles[0].getFile(); })
        .then(readFile, function (err) {
          if (err && err.name !== 'AbortError') log('file picker: ' + err.message);
        });
    });
    $('file-input').addEventListener('change', function () {
      readFile(this.files && this.files[0]);
      this.value = '';
    });
    var drop = $('card-firmware');
    drop.addEventListener('dragover', function (e) {
      e.preventDefault();
      drop.classList.add('drop');
    });
    drop.addEventListener('dragleave', function () { drop.classList.remove('drop'); });
    drop.addEventListener('drop', function (e) {
      e.preventDefault();
      drop.classList.remove('drop');
      if (!state.installing) readFile(e.dataTransfer.files && e.dataTransfer.files[0]);
    });
    $('btn-install').addEventListener('click', install);
    $('btn-again').addEventListener('click', installAnother);

    if (!state.support.supported) {
      checkWebMidi();
      return;
    }
    // Ask at once: the browser shows its own prompt. A refusal leaves the
    // button in step 1 available.
    P.requestMidi().then(grantAccess, function () {
      checkWebMidi();
    });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
