const map = getMap();
let ws = null;

// let hostname = window.location.hostname; // <-- this line is used when inlining to arduino code
let hostname = "192.168.1.150"; // this IP is allocated by my router to my PS/2 remote using the mac address as identifier
connect();
init();

function init() {
  const buttons = (localStorage.getItem("buttons") || "F12").split(",");

  buttons.forEach(makeButton);
}

function makeButton(text) {
  const b = document.createElement("button");
  b.textContent = text;
  document.querySelector("#container").appendChild(b);
}

function connect() {
  ws = new WebSocket(`ws://${hostname}:81/`);
  ws.binaryType = "arraybuffer";

  document.body.dataset.status = ws.readyState;

  let heartbeat = null;

  ws.onopen = () => {
    document.body.dataset.status = ws.readyState;
    heartbeat = setInterval(() => {
      ws.send(Uint8Array.of(0));
    }, 1000);
  };
  ws.onclose = () => {
    document.body.dataset.status = ws.readyState;
    ws = null;
    console.log("connection closed");
    clearTimeout(heartbeat);
    setTimeout(() => connect(), 500);
  };
  ws.onerror = (evt) => {
    document.body.dataset.status = ws.readyState;
    console.log(evt);
  };

  ws.onmessage = function (evt) {
    const data = new Uint8Array(evt.data);
    if (data.length === 0 && data[0] === 0) {
      // heartbeat
      console.log("heartbeat");
    } else {
      console.log(evt.data.byteLength, new Uint8Array(evt.data));
    }
  };
}

document.querySelector("#make").addEventListener("click", (e) => {
  const chr = prompt("Which key?");

  const code = chr.toUpperCase().trim();
  if (!map[code]) {
    alert(`${code} isn't available`);
    return;
  }

  if (!confirm(`Adding ${code}, correct?`)) {
    return;
  }

  // add
  const buttons = (localStorage.getItem("buttons") || "F12").split(",");
  buttons.push(code);
  localStorage.setItem("buttons", buttons.join(","));
  makeButton(code);
});

document.querySelector("#reset").addEventListener("click", () => {
  localStorage.setItem("buttons", "");
  window.location.reload();
});

document.addEventListener("click", (e) => {
  if (e.target.nodeName !== "BUTTON") {
    return;
  }

  if (e.target.id) return; // has specific action

  const code = e.target.textContent;
  send(code);
});

document.addEventListener("keydown", (e) => {
  send(e.code, "make");
});

document.addEventListener("keyup", (e) => {
  send(e.code, "brk");
});

function send(code, mode = "both") {
  if (code.startsWith("Key")) {
    code = code.substr(3);
  }

  if (code.startsWith("Arrow")) {
    code = code.substr(5);
  }

  if (code.startsWith("Digit")) {
    code = code.substr(5);
  }

  const key = map[code.toUpperCase()];
  if (!key) return;
  let codes;
  if (mode === "both") {
    codes = [
      ...key.make.split(",").map((_) => parseInt(_, 16)),
      ...key.brk.split(",").map((_) => parseInt(_, 16)),
    ];
  } else {
    codes = key[mode].split(",").map((_) => parseInt(_, 16));
  }

  console.log(key, codes);
  ws.send(Uint8Array.from(codes));
}

// via https://web.archive.org/web/20070210083447/http://www.computer-engineering.org/ps2keyboard/scancodes2.html
function getMap() {
  return {
    0: {
      make: "45",
      brk: "F0,45",
    },
    1: {
      make: "16",
      brk: "F0,16",
    },
    2: {
      make: "1E",
      brk: "F0,1E",
    },
    3: {
      make: "26",
      brk: "F0,26",
    },
    4: {
      make: "25",
      brk: "F0,25",
    },
    5: {
      make: "2E",
      brk: "F0,2E",
    },
    6: {
      make: "36",
      brk: "F0,36",
    },
    7: {
      make: "3D",
      brk: "F0,3D",
    },
    8: {
      make: "3E",
      brk: "F0,3E",
    },
    9: {
      make: "46",
      brk: "F0,46",
    },
    A: {
      make: "1C",
      brk: "F0,1C",
    },
    BRACKETLEFT: {
      make: "54",
      brk: "F0,54",
    },
    B: {
      make: "32",
      brk: "F0,32",
    },
    BACKQUOTE: {
      make: "0E",
      brk: "F0,0E",
    },
    INSERT: {
      make: "E0,70",
      brk: "E0,F0,70",
    },
    C: {
      make: "21",
      brk: "F0,21",
    },
    MINUS: {
      make: "4E",
      brk: "F0,4E",
    },
    HOME: {
      make: "E0,6C",
      brk: "E0,F0,6C",
    },
    D: {
      make: "23",
      brk: "F0,23",
    },
    EQUAL: {
      make: "55",
      brk: "F0,55",
    },
    PAGEUP: {
      make: "E0,7D",
      brk: "E0,F0,7D",
    },
    E: {
      make: "24",
      brk: "F0,24",
    },
    BACKSLASH: {
      make: "5D",
      brk: "F0,5D",
    },
    DELETE: {
      make: "E0,71",
      brk: "E0,F0,71",
    },
    F: {
      make: "2B",
      brk: "F0,2B",
    },
    BACKSPACE: {
      make: "66",
      brk: "F0,66",
    },
    END: {
      make: "E0,69",
      brk: "E0,F0,69",
    },
    G: {
      make: "34",
      brk: "F0,34",
    },
    SPACE: {
      make: "29",
      brk: "F0,29",
    },
    PAGEDOWN: {
      make: "E0,7A",
      brk: "E0,F0,7A",
    },
    H: {
      make: "33",
      brk: "F0,33",
    },
    TAB: {
      make: "0D",
      brk: "F0,0D",
    },
    UP: {
      make: "E0,75",
      brk: "E0,F0,75",
    },
    I: {
      make: "43",
      brk: "F0,43",
    },
    CAPS: {
      make: "58",
      brk: "F0,58",
    },
    LEFT: {
      make: "E0,6B",
      brk: "E0,F0,6B",
    },
    J: {
      make: "3B",
      brk: "F0,3B",
    },
    SHIFTLEFT: {
      make: "12",
      brk: "F0,12",
    },
    DOWN: {
      make: "E0,72",
      brk: "E0,F0,72",
    },
    K: {
      make: "42",
      brk: "F0,42",
    },
    CONTROLLEFT: {
      make: "14",
      brk: "F0,14",
    },
    RIGHT: {
      make: "E0,74",
      brk: "E0,F0,74",
    },
    L: {
      make: "4B",
      brk: "F0,4B",
    },
    LGUI: {
      make: "E0,1F",
      brk: "E0,F0,1F",
    },
    NUM: {
      make: "77",
      brk: "F0,77",
    },
    M: {
      make: "3A",
      brk: "F0,3A",
    },
    ALTLEFT: {
      make: "11",
      brk: "F0,11",
    },
    "KP/": {
      make: "E0,4A",
      brk: "E0,F0,4A",
    },
    N: {
      make: "31",
      brk: "F0,31",
    },
    SHIFTRIGHT: {
      make: "59",
      brk: "F0,59",
    },
    "KP*": {
      make: "7C",
      brk: "F0,7C",
    },
    O: {
      make: "44",
      brk: "F0,44",
    },
    CONTROLRIGHT: {
      make: "E0,14",
      brk: "E0,F0,14",
    },
    "KP-": {
      make: "7B",
      brk: "F0,7B",
    },
    P: {
      make: "4D",
      brk: "F0,4D",
    },
    RGUI: {
      make: "E0,27",
      brk: "E0,F0,27",
    },
    "KP+": {
      make: "79",
      brk: "F0,79",
    },
    Q: {
      make: "15",
      brk: "F0,15",
    },
    ALTRIGHT: {
      make: "E0,11",
      brk: "E0,F0,11",
    },
    KPEN: {
      make: "E0,5A",
      brk: "E0,F0,5A",
    },
    R: {
      make: "2D",
      brk: "F0,2D",
    },
    APPS: {
      make: "E0,2F",
      brk: "E0,F0,2F",
    },
    "KP.": {
      make: "71",
      brk: "F0,71",
    },
    S: {
      make: "1B",
      brk: "F0,1B",
    },
    ENTER: {
      make: "5A",
      brk: "F0,5A",
    },
    KP0: {
      make: "70",
      brk: "F0,70",
    },
    T: {
      make: "2C",
      brk: "F0,2C",
    },
    ESC: {
      make: "76",
      brk: "F0,76",
    },
    KP1: {
      make: "69",
      brk: "F0,69",
    },
    U: {
      make: "3C",
      brk: "F0,3C",
    },
    F1: {
      make: "05",
      brk: "F0,05",
    },
    KP2: {
      make: "72",
      brk: "F0,72",
    },
    V: {
      make: "2A",
      brk: "F0,2A",
    },
    F2: {
      make: "06",
      brk: "F0,06",
    },
    KP3: {
      make: "7A",
      brk: "F0,7A",
    },
    W: {
      make: "1D",
      brk: "F0,1D",
    },
    F3: {
      make: "04",
      brk: "F0,04",
    },
    KP4: {
      make: "6B",
      brk: "F0,6B",
    },
    X: {
      make: "22",
      brk: "F0,22",
    },
    F4: {
      make: "0C",
      brk: "F0,0C",
    },
    KP5: {
      make: "73",
      brk: "F0,73",
    },
    Y: {
      make: "35",
      brk: "F0,35",
    },
    F5: {
      make: "03",
      brk: "F0,03",
    },
    KP6: {
      make: "74",
      brk: "F0,74",
    },
    Z: {
      make: "1A",
      brk: "F0,1A",
    },
    F6: {
      make: "0B",
      brk: "F0,0B",
    },
    KP7: {
      make: "6C",
      brk: "F0,6C",
    },
    F7: {
      make: "83",
      brk: "F0,83",
    },
    KP8: {
      make: "75",
      brk: "F0,75",
    },
    F8: {
      make: "0A",
      brk: "F0,0A",
    },
    KP9: {
      make: "7D",
      brk: "F0,7D",
    },
    F9: {
      make: "01",
      brk: "F0,01",
    },
    BRACKETRIGHT: {
      make: "5B",
      brk: "F0,5B",
    },
    F10: {
      make: "09",
      brk: "F0,09",
    },
    SEMICOLON: {
      make: "4C",
      brk: "F0,4C",
    },
    F11: {
      make: "78",
      brk: "F0,78",
    },
    QUOTE: {
      make: "52",
      brk: "F0,52",
    },
    F12: {
      make: "07",
      brk: "F0,07",
    },
    COMMA: {
      make: "41",
      brk: "F0,41",
    },
    F13: {
      // print screen
      make: "E0,12,E0,7C",
      brk: "E0,F0,7C,E0,F0,12",
    },
    PERIOD: {
      make: "49",
      brk: "F0,49",
    },
    SCROLL: {
      make: "7E",
      brk: "F0,7E",
    },
    SLASH: {
      make: "4A",
      brk: "F0,4A",
    },
    PAUSE: {
      make: "E1,14,77,E1,F0,14,F0,77",
      brk: "",
    },
  };
}
