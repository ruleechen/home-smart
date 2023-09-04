((win) => {
  const _init = [];
  const vic = (fn) => {
    _init.push(fn);
  };
  vic._last = [];
  const run = () => {
    const fn = _init.shift() || vic._last.shift();
    if (typeof fn === "function") {
      Promise.resolve(fn())
        .then(run)
        .catch((ex) => console.error(ex));
    }
  };
  win.vic = vic;
  win.addEventListener("DOMContentLoaded", run);
})(window);

vic.query = (s) => document.querySelector(s);
vic.queryAll = (s) => Array.from(document.querySelectorAll(s));
vic.confirm = () => confirm("Are you sure you want to do it?");

vic.bytes = (bytes) =>
  bytes >= 1024 * 1024
    ? (bytes / 1024 / 1024).toFixed(2) + " MB"
    : bytes >= 1024
    ? (bytes / 1024).toFixed(2) + " KB"
    : `${bytes} BY`;

vic.ago = (millis, timestamp) => {
  const pad = (n) => `${n}`.padStart(2, "0");
  let span = (millis - timestamp) / 1000;
  let str = "";
  // d
  const oneDay = 86400;
  let days = span / oneDay;
  if (days > 1) {
    days = Math.floor(days);
    span -= days * oneDay;
    str += `${days}x24 `;
  }
  // h
  const oneHour = 3600;
  let hours = span / oneHour;
  if (hours > 1) {
    hours = Math.floor(hours);
    span -= hours * oneHour;
  } else {
    hours = 0;
  }
  str += pad(hours);
  // m
  const oneMinute = 60;
  let minutes = span / oneMinute;
  if (minutes > 1) {
    minutes = Math.floor(minutes);
    span -= minutes * oneMinute;
  } else {
    minutes = 0;
  }
  str += ":" + pad(minutes);
  // s
  const seconds = Math.floor(span);
  str += ":" + pad(seconds);
  // ret
  return str;
};

vic._navFns = [];
vic.appendNav = (fn) => vic._navFns.push(fn);
vic.getNav = () => {
  let items = [];
  if (vic._navFns) {
    vic._navFns.forEach((fn) => {
      const ret = fn(items);
      if (Array.isArray(ret)) {
        items = ret;
      }
    });
  }
  return m("p.nav", items);
};
vic.navItem = (href, text) => {
  const curr = m.route.get();
  const match = curr === href || curr.indexOf(href + "/") === 0;
  return m(m.route.Link, { href, class: match ? "match" : "" }, text);
};

vic._routeFns = [];
vic.appendRoute = (fn) => vic._routeFns.push(fn);
vic.getRoute = () => {
  let config = {};
  if (vic._routeFns) {
    vic._routeFns.forEach((fn) => {
      const ret = fn(config);
      if (ret) {
        config = ret;
      }
    });
  }
  return config;
};

// state manager
const smgr = ((win) => {
  let d = { _ready: false, _info: null, _error: null };
  const set = (state, ready) => {
    d = state;
    d._ready = !!ready;
    d._info = null;
    d._error = null;
  };
  const info = (msg) => {
    d._info = msg;
    win.m.redraw();
  };
  const error = (msg) => {
    d._error = msg;
    win.m.redraw();
  };
  const req = (opts) =>
    win.m
      .request(opts)
      .then((res) => {
        d._ready = true;
        info(res && res.msg);
        error(res && res.err);
        return res && res.err
          ? Promise.reject() // stop the following `then()`
          : res;
      })
      .catch((err) => {
        d._ready = true;
        error(err instanceof Error ? err.stack : err);
        return Promise.reject(); // stop the following `then()`
      });
  const loading = () => (d._ready ? null : [vic.getNav(), m("div.spinner")]);
  const message = () =>
    [
      d._info ? m("div.info.message", d._info) : null,
      d._error ? m("div.error.message", d._error) : null,
    ].filter((x) => !!x);
  return { set, info, error, req, loading, message };
})(window);

vic.mSelect = (name, value, options) => {
  return m(
    "select",
    { name },
    options.map((x) =>
      m("option", { value: x.value, selected: value === x.value }, x.text)
    )
  );
};

function fc(cell) {
  if (typeof cell !== "string") {
    return cell;
  }
  const parts = cell.split("|");
  if (parts.length !== 2) {
    return cell;
  }
  if (parts[0] === "ago") {
    const millis = parseFloat(parts[1], 10);
    return vic.ago(millis, 0);
  }
  return cell;
}

vic.mTable = (data) => {
  let rows = [];
  let cols = 0;
  if (data.header && data.header.length > 0) {
    cols = data.header.length;
    rows.push(
      m(
        "tr",
        data.header.map((c) => m("th", [fc(c)]))
      )
    );
  }
  if (data.rows && data.rows.length > 0) {
    if (cols === 0) {
      cols = data.rows[0].length;
    }
    rows = rows.concat(
      data.rows.map((row) =>
        m(
          "tr",
          row.map((cell) => m("td", [fc(cell)]))
        )
      )
    );
  }
  return m(`table.cols${cols}`, rows);
};

const mSelectionList = (type, name, values, list) => {
  const items = list.map((item) =>
    m("li", [
      m("input", {
        type,
        name,
        id: `rdo${name}${item.value}`,
        value: item.value,
        checked: values.includes(item.value),
      }),
      m("label", { for: `rdo${name}${item.value}` }, item.text),
    ])
  );
  return m("ul", items);
};

vic.mRadioList = (name, values, list) =>
  mSelectionList("radio", name, values, list);
vic.mCheckList = (name, values, list) =>
  mSelectionList("checkbox", name, values, list);

const ServiceView = (() => {
  const state = {
    states: [],
    buttons: [],
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/service/get",
      })
      .then((res) => {
        state.states = res.states || [];
        state.buttons = res.buttons || [];
        m.redraw();
      });
  };
  const post = (ev) => {
    if (vic.confirm()) {
      smgr
        .req({
          method: "POST",
          url: "/service/post",
          body: { value: ev.target.value },
        })
        .then(() => {
          oninit();
        });
    }
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "Service"),
          ...smgr.message(),
          vic.mTable({
            header: null,
            rows: state.states,
          }),
          m("div.form", [
            m(
              "p",
              state.buttons.map(([text, value]) =>
                m("button.btn.weak", { value, onclick: post }, text)
              )
            ),
          ]),
        ]
      );
    },
  };
})();

const SystemView = (() => {
  const state = {
    data: {},
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/system/status",
      })
      .then((res) => {
        state.data = res;
        m.redraw();
      });
  };
  return {
    oninit,
    view() {
      const d = state.data;
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "System"),
          m("p", [m(m.route.Link, { href: "/system/reset" }, "Reset")]),
          ...smgr.message(),
          m("p", [
            vic.mTable({
              header: ["Status", ""],
              rows: [
                ["Boot", vic.ago(d.millis, 0)],
                ["Reset Reason", d.resetReason],
                ["Free Stack", vic.bytes(d.freeStack)],
                ["Free Heap", vic.bytes(d.freeHeap)],
                ["Max Free Block Size", vic.bytes(d.maxFreeBlockSize)],
                ["Heap Fragmentation", d.heapFragmentation],
              ],
            }),
            vic.mTable({
              header: ["Hardware", ""],
              rows: [
                ["Chip ID", d.chipId],
                ["CPU Freq", `${d.cupFreqMHz} MHz`],
                ["Flash ID", d.flashId],
                ["Flash Size", vic.bytes(d.flashSize)],
                ["Flash Size (Real)", vic.bytes(d.flashSizeReal)],
                ["Flash Speed", `${d.flashSpeedMHz} MHz`],
              ],
            }),
            vic.mTable({
              header: ["Software", ""],
              rows: [
                [
                  "Sketch MD5",
                  m("span.ellipsis", { title: d.sketchMD5 }, d.sketchMD5),
                ],
                ["Sketch Size", vic.bytes(d.sketchSize)],
                ["Sketch Free Space", vic.bytes(d.sketchFreeSpace)],
                ["SDK Version", d.sdkVersion],
                ["Core Version", d.coreVersion],
                ["Firmware Version", _vic.firmwareVersion],
                ["Build Time", new Date(_vic.unixTime * 1000).toLocaleString()],
              ],
            }),
          ]),
        ]
      );
    },
  };
})();

const SystemResetView = (() => {
  const state = {};
  const oninit = () => {
    smgr.set(state, true);
  };
  const reset = () => {
    if (vic.confirm()) {
      const values = vic
        .queryAll("input[name=esp]:checked")
        .map((x) => x.value)
        .join(",");
      smgr.req({
        method: "POST",
        url: "/system/reset",
        body: { values },
      });
    }
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("p", [m(m.route.Link, { href: "/system" }, "< System")]),
          m("h3", "Reset (ESP)"),
          ...smgr.message(),
          m("div.form", [
            vic.mCheckList(
              "esp",
              [],
              [
                { value: "1", text: "Restart" },
                { value: "2", text: "Reset" },
                { value: "3", text: "Erase Config" },
              ]
            ),
            m("p", [m("button.btn.weak", { onclick: reset }, "Submit")]),
          ]),
        ]
      );
    },
  };
})();

const FileSystemView = (() => {
  const state = {
    data: {},
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/fs",
      })
      .then((res) => {
        state.data = res;
        m.redraw();
      });
  };
  return {
    oninit,
    view() {
      const d = state.data;
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "File System"),
          m("p", [m(m.route.Link, { href: "/fs/files" }, "Files")]),
          ...smgr.message(),
          m("p", [
            vic.mTable({
              rows: [
                ["Total", vic.bytes(d.totalBytes)],
                ["Used", vic.bytes(d.usedBytes)],
                ["Max Path Length", d.maxPathLength],
                ["Max Open Files", d.maxOpenFiles],
                ["Block Size", vic.bytes(d.blockSize)],
                ["Page Size", vic.bytes(d.pageSize)],
              ],
            }),
          ]),
        ]
      );
    },
  };
})();

const FileListView = (() => {
  const state = {
    files: [],
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/files",
      })
      .then((res) => {
        state.files = res.files || [];
        m.redraw();
      });
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "Files"),
          m("p", [m(m.route.Link, { href: "/fs" }, "< FS")]),
          ...smgr.message(),
          m("p", [
            vic.mTable({
              rows: state.files.map(({ path, size }) => [
                m(
                  m.route.Link,
                  {
                    href: m.buildPathname("/fs/files/:path", { path, size }),
                  },
                  path
                ),
                vic.bytes(size),
              ]),
            }),
          ]),
        ]
      );
    },
  };
})();

const FileItemView = (() => {
  const state = {
    path: "",
    size: 0,
    content: "",
    editable: false,
  };
  const fileReq = (method, body) => {
    const path = m.route.param("path");
    return smgr.req({
      method,
      url: "/file",
      params: { path },
      body,
    });
  };
  const oninit = () => {
    smgr.set(state, true);
    state.path = m.route.param("path");
    state.size = parseInt(m.route.param("size"), 10);
    state.editable = state.size <= _vic.maxEditSize;
    if (state.editable) {
      smgr.set(state);
      fileReq("GET").then(({ size, editable, content }) => {
        state.size = size;
        state.content = content;
        state.editable = editable;
        if (editable) {
          m.redraw();
        } else {
          smgr.error("File not editable");
        }
      });
    } else {
      smgr.error("File size limited");
    }
  };
  const save = () => {
    fileReq("POST", {
      content: vic.query("textarea").value,
      saveAs: vic.query("#txtSaveAs").value,
    }).then(() => {
      oninit();
    });
  };
  const remove = () => {
    if (vic.confirm()) {
      fileReq("DELETE").then(() => {
        m.route.set("/fs/files");
      });
    }
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", `${state.path} (${vic.bytes(state.size)})`),
          m("p", [m(m.route.Link, { href: "/fs/files" }, "< Files")]),
          ...smgr.message(),
          state.editable &&
            m("div.form", [
              m("p", [
                m(
                  "textarea",
                  { cols: 50, rows: 15, maxLength: _vic.maxEditSize },
                  state.content
                ),
              ]),
              m("p", [
                m("input[type=text]", {
                  id: "txtSaveAs",
                  placeholder: "Save As",
                }),
              ]),
              m("p", [
                m("button.btn", { onclick: save }, "Save"),
                m("button.btn.weak", { onclick: remove }, "Delete"),
              ]),
            ]),
        ]
      );
    },
  };
})();

const WiFiView = (() => {
  const state = {
    data: {},
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/wifi",
      })
      .then((res) => {
        state.data = res;
        m.redraw();
      });
  };
  return {
    oninit,
    view() {
      const d = state.data;
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "WiFi"),
          m("p", [
            m(m.route.Link, { href: "/wifi/list" }, "Join"),
            m("span", " | "),
            m(m.route.Link, { href: "/wifi/mode" }, "Mode"),
          ]),
          ...smgr.message(),
          m("p", [
            vic.mTable({
              rows: [
                [
                  "Local Host",
                  d.hostName
                    ? m("a", { href: `http://${d.hostName}.local` }, d.hostName)
                    : "",
                ],
                ["MDNS", d.mdns ? "Running" : "-"],
                ["Mode", d.mode],
                ["Sleep Mode", d.sleepMode],
                ["(DTIM) Listen Interval", d.listenInterval],
                ["Joined", d.joined ? `${d.joined} (${100 + d.rssi}%)` : "-"],
                [
                  "STA IP",
                  d.staAddress
                    ? m("a", { href: `http://${d.staAddress}` }, d.staAddress)
                    : "-",
                ],
                ["STA MAC", d.staMacAddress],
                [
                  "AP IP",
                  d.apAddress
                    ? m("a", { href: `http://${d.apAddress}` }, d.apAddress)
                    : "-",
                ],
                ["AP MAC", d.apMacAddress],
              ],
            }),
          ]),
        ]
      );
    },
  };
})();

const WiFiListView = (() => {
  const state = {
    bssid: null,
    founds: [{ bssid: "", ssid: "", channel: 0, rssi: 10 }],
    pswd: "",
    times: -1,
    status: -3, // unknown
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/wifi/list",
      })
      .then((res) => {
        Object.assign(state, res);
        state.founds = res.founds || [];
        m.redraw();
      });
  };
  const scan = () => {
    state.pswd = vic.query("#txtPswd").value;
    oninit();
  };
  const statusName = (code) => {
    const map = {
      "-3": "UNKNOWN",
      "-2": "ERROR",
      "-1": "TIMEOUT",
      0: "IDLE_STATUS",
      1: "NO_SSID_AVAIL",
      2: "SCAN_COMPLETED",
      3: "CONNECTED",
      4: "CONNECT_FAILED",
      5: "CONNECTION_LOST",
      6: "WRONG_PASSWORD",
      7: "DISCONNECTED",
    };
    return map[code];
  };
  let countNext;
  const countStatus = (times) => {
    state.times = times;
    m.redraw();
    if (state.times > 0) {
      m.request({
        method: "GET",
        url: "/wifi/join/status",
      })
        .then((res) => {
          state.status = res.status;
          countNext();
        })
        .catch(() => {
          state.status = -2; // error
          countNext();
        });
    }
  };
  countNext = () => {
    m.redraw();
    if (state.status === 3) {
      alert("Connected!");
    } else if (state.times > 0) {
      setTimeout(() => {
        countStatus(state.times - 1);
      }, 1000);
    }
  };
  const join = () => {
    // validate
    const passEl = vic.query("#txtPswd");
    const bssidEl = vic.query("input[type=radio]:checked");
    if (!bssidEl) {
      smgr.error("Please select wifi to join");
      return;
    }
    state.bssid = bssidEl.value;
    state.pswd = passEl.value;
    // join
    const ap = state.founds.find((x) => x.bssid === state.bssid);
    smgr
      .req({
        method: "POST",
        url: "/wifi/join",
        body: Object.assign({}, { pswd: state.pswd }, ap),
      })
      .then((res) => {
        if (!res.msg) {
          state.status = -3; // unknown
          countStatus(60); // start monitor wifi join state
        }
      });
  };
  const reset = () => {
    if (vic.confirm()) {
      smgr
        .req({
          method: "POST",
          url: "/wifi/reset",
          body: {},
        })
        .then(() => {
          oninit();
        });
    }
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "Join WiFi"),
          m("p", [m(m.route.Link, { href: "/wifi" }, "< WiFi")]),
          ...smgr.message(),
          m("div.form", [
            vic.mTable({
              rows: state.founds.map((x) => [
                m("input", {
                  type: "radio",
                  name: "bssid",
                  id: x.bssid,
                  value: x.bssid,
                  checked: x.bssid === state.bssid,
                }),
                m("label", { for: x.bssid }, x.ssid),
                m("label", { for: x.bssid }, x.bssid),
                m("label", { for: x.bssid }, `${100 + x.rssi}%`), // the closer the value is to 0, the stronger the received signal has been.
              ]),
            }),
            m("p", [
              m("label", { for: "txtPswd" }, "Password"),
              m("input[type=text]", {
                id: "txtPswd",
                maxLength: 32,
                value: state.pswd,
              }),
            ]),
            m("p", [
              m("button.btn.weak", { onclick: reset }, "Reset"),
              m("button.btn", { onclick: scan }, "Scan"),
              m("button.btn", { onclick: join }, "Join"),
            ]),
            state.times >= 0
              ? m(
                  "p.info",
                  `Please wait... (${state.times})/${statusName(state.status)}`
                )
              : null,
          ]),
        ]
      );
    },
  };
})();

const WiFiModeView = (() => {
  const state = {
    mode: "",
  };
  const oninit = () => {
    smgr.set(state, true);
  };
  const save = () => {
    const modeEl = vic.query("input[type=radio]:checked");
    smgr
      .req({
        method: "POST",
        url: "/wifi/mode",
        body: { mode: parseInt(modeEl.value, 10) },
      })
      .then(() => {
        oninit();
      });
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "WiFi Mode"),
          m("p", [m(m.route.Link, { href: "/wifi" }, "< WiFi")]),
          ...smgr.message(),
          m("div.form", [
            vic.mRadioList(
              "WiFiMode",
              [state.mode],
              [
                { value: "3", text: "AP+STA" },
                { value: "2", text: "AP" },
                { value: "1", text: "STA" },
                { value: "0", text: "OFF" },
              ]
            ),
            m("p", [m("button.btn", { onclick: save }, "Save")]),
          ]),
        ]
      );
    },
  };
})();

const OtaView = (() => {
  const state = {
    data: {},
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/ota",
      })
      .then((res) => {
        state.data = res;
        m.redraw();
      });
  };
  const fire = () => {
    const version = "";
    const otaType = "";
    smgr.req({
      method: "POST",
      url: "/ota/fire",
      body: { version, otaType },
    });
  };
  return {
    oninit,
    view() {
      const d = state.data;
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "OTA"),
          m("p", [m(m.route.Link, { href: "/ota/otw" }, "Over Web")]),
          ...smgr.message(),
          m("div.form", [
            m("p", [
              vic.mTable({
                rows: [
                  ["Flash Size", vic.bytes(d.flashSize)],
                  ["Flash Size (Real)", vic.bytes(d.flashSizeReal)],
                  ["Sketch Size", vic.bytes(d.sketchSize)],
                  ["Sketch Free Space", vic.bytes(d.sketchFreeSpace)],
                  ["Remote Latest", d.otaNewVersion],
                  ["Local Firmware", d.otaVersion],
                ],
              }),
            ]),
            vic.mRadioList(
              "OtaType",
              ["sketch"],
              [
                { value: "all", text: "All" },
                { value: "sketch", text: "Sketch" },
                { value: "fs", text: "File System" },
              ]
            ),
            m("p", [
              m(
                "button.btn",
                { onclick: fire },
                "Load + Burn " + d.otaNewVersion
              ),
            ]),
          ]),
        ]
      );
    },
  };
})();

const OtaOtwView = {
  view() {
    return [
      vic.getNav(),
      m("h3", "Over The Web"),
      m("p", [m(m.route.Link, { href: "/ota" }, "< OTA")]),
      ...smgr.message(),
      m("div.form", [
        m("iframe", {
          width: "100%",
          height: "300",
          frameborder: "0",
          scrolling: "no",
          marginHeight: "0",
          marginWidth: "0",
          src: "/update",
        }),
      ]),
    ];
  },
};

const RadioView = (() => {
  const state = {
    millis: 0,
    inputPin: -1,
    lastReceived: {
      value: "",
      channel: 0,
      timestamp: 1,
    },
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/radio",
      })
      .then((res) => {
        state.millis = res.millis;
        state.inputPin = res.inputPin;
        state.outputPin = res.outputPin;
        state.lastReceived = res.lastReceived || {};
        m.redraw();
      });
  };
  const save = () => {
    const inputPin = vic.query("#txtInputPin").value;
    const outputPin = vic.query("#txtOutputPin").value;
    smgr
      .req({
        method: "POST",
        url: "/radio",
        body: { inputPin, outputPin },
      })
      .then(() => {
        oninit();
      });
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "Radio"),
          m("p", [
            m(m.route.Link, { href: "/radio/emit" }, "Emits"),
            m("span", " | "),
            m(m.route.Link, { href: "/radio/rule" }, "Rules"),
            m("span", " | "),
            m(m.route.Link, { href: "/radio/command" }, "Commands"),
          ]),
          ...smgr.message(),
          vic.mTable({
            header: null,
            rows: [
              [
                "Last Received",
                state.lastReceived.value
                  ? vic.ago(state.millis, state.lastReceived.timestamp) + " ago"
                  : "-",
              ],
              [
                "Value",
                state.lastReceived.value ? state.lastReceived.value : "-",
              ],
              [
                "Channel",
                state.lastReceived.value ? state.lastReceived.channel : "-",
              ],
            ],
          }),
          m("div.form", [
            m("p", [
              m("label", { for: "txtInputPin" }, "Input Pin"),
              m("input[type=number]", {
                id: "txtInputPin",
                min: -1,
                max: 100,
                value: state.inputPin,
              }),
            ]),
            m("p", [
              m("label", { for: "txtOutputPin" }, "Output Pin"),
              m("input[type=number]", {
                id: "txtOutputPin",
                min: -1,
                max: 100,
                value: state.outputPin,
              }),
            ]),
            m("p", [m("button.btn", { onclick: save }, "Save")]),
          ]),
        ]
      );
    },
  };
})();

const RadioEmitView = (() => {
  const state = {
    emits: [{ name: "", value: "", channel: 0, press: 1 }],
    lastReceived: { value: "", channel: 0 },
    pressOptions: [
      [1, "Click"],
      [2, "Double Click"],
      [3, "Long Press"],
    ],
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/radio/emit",
      })
      .then((res) => {
        state.emits = res.emits || [];
        state.lastReceived = res.lastReceived || {};
        m.redraw();
      });
  };
  const add = () => {
    state.emits.push({
      name: "",
      value: state.lastReceived.value,
      channel: state.lastReceived.channel,
      press: state.pressOptions[0][0],
    });
    m.redraw();
  };
  const remove = (ev) => {
    if (vic.confirm()) {
      const index = parseInt(ev.target.value, 10);
      state.emits.splice(index, 1);
      m.redraw();
    }
  };
  const save = () => {
    const nameEls = vic.queryAll("input[name=Name]");
    const valueEls = vic.queryAll("input[name=Value]");
    const channelEls = vic.queryAll("input[name=Channel]");
    const pressIdEls = vic.queryAll("select[name=PressId]");
    const emits = nameEls.map((nameEl, i) => ({
      name: nameEl.value,
      value: valueEls[i].value,
      channel: channelEls[i].value,
      press: pressIdEls[i].value,
    }));
    smgr
      .req({
        method: "POST",
        url: "/radio/emit",
        body: { emits },
      })
      .then(() => {
        oninit();
      });
  };
  const send = (ev) => {
    const index = parseInt(ev.target.value, 10);
    smgr.req({
      method: "POST",
      url: "/radio/emit/send",
      body: { index },
    });
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "Radio Emits"),
          m("p", [m(m.route.Link, { href: "/radio" }, "< Radio")]),
          ...smgr.message(),
          m("div.form", [
            vic.mTable({
              header: ["", "", "Name", "Value", "Channel", "Press"],
              rows: state.emits.map((emit, index) => [
                m(
                  "button.btn.weak",
                  {
                    name: "Remove",
                    value: index,
                    onclick: remove,
                  },
                  "Remove"
                ),
                m(
                  "button.btn",
                  { name: "Send", value: index, onclick: send },
                  "Send"
                ),
                m("input[type=text]", {
                  name: "Name",
                  value: emit.name,
                  maxLength: 8,
                  style: { width: "40px" },
                }),
                m("input[type=text]", {
                  name: "Value",
                  value: emit.value,
                  maxLength: 8,
                  style: { width: "60px" },
                }),
                m("input[type=number]", {
                  name: "Channel",
                  value: emit.channel,
                  min: -1,
                  max: 100,
                }),
                vic.mSelect(
                  "PressId",
                  emit.press,
                  state.pressOptions.map((x) => ({
                    value: x[0],
                    text: x[1],
                  }))
                ),
              ]),
            }),
            m("p", [
              m("button.btn", { onclick: add }, "Add+"),
              m("button.btn", { onclick: save }, "Save"),
            ]),
          ]),
        ]
      );
    },
  };
})();

const RadioRuleView = (() => {
  const state = {
    rules: [{ value: "", channel: 0, press: 1, action: 1 }],
    lastReceived: { value: "", channel: 0 },
    pressOptions: [
      [1, "Click"],
      [2, "Double Click"],
      [3, "Long Press"],
    ],
    actionOptions: [
      [0, "None"],
      [1, "True"],
      [2, "False"],
      [3, "Toggle"],
      [4, "WiFi STA"],
      [5, "WiFi STA+AP"],
      [6, "WiFi Reset"],
      [7, "ESP Restart"],
    ],
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/radio/rule",
      })
      .then((res) => {
        state.rules = res.rules || [];
        state.lastReceived = res.lastReceived || [];
        m.redraw();
      });
  };
  const add = () => {
    state.rules.push({
      value: state.lastReceived.value,
      channel: state.lastReceived.channel,
      press: state.pressOptions[0][0],
      action: state.actionOptions[0][0],
    });
    m.redraw();
  };
  const remove = (ev) => {
    if (vic.confirm()) {
      const index = parseInt(ev.target.value, 10);
      state.rules.splice(index, 1);
      m.redraw();
    }
  };
  const save = () => {
    const valueEls = vic.queryAll("input[name=Value]");
    const channelEls = vic.queryAll("input[name=Channel]");
    const pressIdEls = vic.queryAll("select[name=PressId]");
    const actionIdEls = vic.queryAll("select[name=ActionId]");
    const rules = valueEls.map((valueEl, i) => ({
      value: valueEl.value,
      channel: channelEls[i].value,
      press: pressIdEls[i].value,
      action: actionIdEls[i].value,
    }));
    smgr
      .req({
        method: "POST",
        url: "/radio/rule",
        body: { rules },
      })
      .then(() => {
        oninit();
      });
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "Radio Rules"),
          m("p", [m(m.route.Link, { href: "/radio" }, "< Radio")]),
          ...smgr.message(),
          m("div.form", [
            vic.mTable({
              header: ["", "Value", "Channel", "Press", "Action"],
              rows: state.rules.map((rule, index) => [
                m(
                  "button.btn.weak",
                  {
                    name: "Remove",
                    value: index,
                    onclick: remove,
                  },
                  "Remove"
                ),
                m("input[type=text]", {
                  name: "Value",
                  value: rule.value,
                  maxLength: 8,
                  style: { width: "60px" },
                }),
                m("input[type=number]", {
                  name: "Channel",
                  value: rule.channel,
                  min: -1,
                  max: 100,
                }),
                vic.mSelect(
                  "PressId",
                  rule.press,
                  state.pressOptions.map((x) => ({
                    value: x[0],
                    text: x[1],
                  }))
                ),
                vic.mSelect(
                  "ActionId",
                  rule.action,
                  state.actionOptions.map((x) => ({
                    value: x[0],
                    text: x[1],
                  }))
                ),
              ]),
            }),
            m("p", [
              m("button.btn", { onclick: add }, "Add+"),
              m("button.btn", { onclick: save }, "Save"),
            ]),
          ]),
        ]
      );
    },
  };
})();

const RadioCommandView = (() => {
  const state = {
    commands: [{ entry: 1, action: 1, press: 1 }],
    entryActionOptions: [
      ["0-0", "None"],
      ["1-1", "wifi-join"],
      ["1-2", "wifi-mode"],
      ["1-3", "wifi-reset"],
      ["2-1", "app-name"],
      ["2-2", "app-ota"],
      ["3-1", "esp-restart"],
      ["4-1", "boolean-set"],
      ["4-2", "boolean-toggle"],
    ],
    pressOptions: [
      [1, "Click"],
      [2, "Double Click"],
      [3, "Long Press"],
    ],
  };
  const oninit = () => {
    smgr.set(state);
    smgr
      .req({
        method: "GET",
        url: "/radio/command",
      })
      .then((res) => {
        state.commands = res.commands || [];
        m.redraw();
      });
  };
  const add = () => {
    const entryAction = state.entryActionOptions[0][0].split("-");
    state.commands.push({
      entry: entryAction[0],
      action: entryAction[1],
      press: state.pressOptions[0][0],
    });
    m.redraw();
  };
  const remove = (ev) => {
    if (vic.confirm()) {
      const index = parseInt(ev.target.value, 10);
      state.commands.splice(index, 1);
      m.redraw();
    }
  };
  const save = () => {
    const entryActionEls = vic.queryAll("select[name=EntryAction]");
    const pressIdEls = vic.queryAll("select[name=PressId]");
    const commands = entryActionEls.map((el, i) => {
      const entryAction = el.value.split("-");
      return {
        entry: entryAction[0],
        action: entryAction[1],
        press: pressIdEls[i].value,
      };
    });
    smgr
      .req({
        method: "POST",
        url: "/radio/command",
        body: { commands },
      })
      .then(() => {
        oninit();
      });
  };
  return {
    oninit,
    view() {
      return (
        smgr.loading() || [
          vic.getNav(),
          m("h3", "Radio Commands"),
          m("p", [m(m.route.Link, { href: "/radio" }, "< Radio")]),
          ...smgr.message(),
          m("div.form", [
            vic.mTable({
              header: ["", "Entry", "Press"],
              rows: state.commands.map((command, index) => [
                m(
                  "button.btn.weak",
                  {
                    name: "Remove",
                    value: index,
                    onclick: remove,
                  },
                  "Remove"
                ),
                vic.mSelect(
                  "EntryAction",
                  "" + command.entry + "-" + command.action,
                  state.entryActionOptions.map((x) => ({
                    value: x[0],
                    text: x[1],
                  }))
                ),
                vic.mSelect(
                  "PressId",
                  command.press,
                  state.pressOptions.map((x) => ({
                    value: x[0],
                    text: x[1],
                  }))
                ),
              ]),
            }),
            m("p", [
              m("button.btn", { onclick: add }, "Add+"),
              m("button.btn", { onclick: save }, "Save"),
            ]),
          ]),
        ]
      );
    },
  };
})();

const NotfoundView = {
  view() {
    return "Oops... Notfound";
  },
};

vic.appendNav((items) =>
  items.concat([
    vic.navItem("/service", "Service"),
    m("span", " | "),
    vic.navItem("/system", "System"),
    m("span", " | "),
    vic.navItem("/wifi", "WiFi"),
    m("span", " | "),
    vic.navItem("/fs", "FS"),
    m("span", " | "),
    vic.navItem("/ota", "OTA"),
  ])
);

vic.appendRoute((config) =>
  Object.assign(config, {
    "/service": ServiceView,
    "/system": SystemView,
    "/system/reset": SystemResetView,
    "/wifi": WiFiView,
    "/wifi/list": WiFiListView,
    "/wifi/mode": WiFiModeView,
    "/fs": FileSystemView,
    "/fs/files": FileListView,
    "/fs/files/:path": FileItemView,
    "/ota": OtaView,
    "/ota/otw": OtaOtwView,
    "/404": NotfoundView,
  })
);

if (_vic.hasRadio) {
  vic.appendNav((items) =>
    items.concat([m("span", " | "), vic.navItem("/radio", "Radio")])
  );
  vic.appendRoute((config) =>
    Object.assign(config, {
      "/radio": RadioView,
      "/radio/emit": RadioEmitView,
      "/radio/rule": RadioRuleView,
      "/radio/command": RadioCommandView,
    })
  );
}

// start
vic(() => {
  // main
  const root = vic.query("div.main");
  m.route(root, "/service", vic.getRoute());
  // foot
  const year = new Date(_vic.unixTime * 1000).getFullYear();
  const foot = vic.query("div.foot");
  foot.innerHTML = `<span>&copy; Copyright 2020 - ${year}</span> <a target="_blank" href="https://github.com/ruleechen">${_vic.firmwareManufacturer}</a>`;
});
