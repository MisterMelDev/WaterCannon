var joy = new JoyStick("joyDiv");
var socketState = document.getElementById('socket-state');
var controlState = document.getElementById('control-state');

var ws = new WebSocket("ws://" + window.location.host + ":81/");
ws.onopen = function(event) {
    setSocketState("WebSocket connected");
};

ws.onclose = function(event) {
    setSocketState("WebSocket closed");
};

setInterval(function() {
    if(ws.readyState == WebSocket.OPEN && !animationEnabled) {
        ws.send("j" + joy.GetX() + "/" + joy.GetY());
    }
}, 150);

function setSocketState(msg) {
    socketState.innerHTML = msg;
    console.log("New socket state: " + msg);
}

function setControlState(msg) {
	controlState.innerHTML = msg;
	console.log("New control state: " + msg);
}

var relayOneEnabled = false;
var relayTwoEnabled = false;

var relayOneButton = document.getElementById("relay-1");
var relayTwoButton = document.getElementById("relay-2");

relayOneButton.onclick = function() {
    relayOneEnabled = !relayOneEnabled;
    updateButton(relayOneButton, relayOneEnabled);
    ws.send("r1" + (relayOneEnabled ? "1" : "0"));
};

relayTwoButton.onclick = function() {
    relayTwoEnabled = !relayTwoEnabled;
    updateButton(relayTwoButton, relayTwoEnabled);
    ws.send("r2" + (relayTwoEnabled ? "1" : "0"));
};

var animationEnabled = false;
var animationSelect = document.getElementById("animation-select");
var animationToggleButton = document.getElementById("animation-toggle");

animationToggleButton.onclick = function() {
	animationEnabled = !animationEnabled;
	updateButton(animationToggleButton, animationEnabled);
	
	animationSelect.disabled = animationEnabled;
	if(animationEnabled) {
		setControlState("Animation running, joystick disabled");
		ws.send("ae" + animationSelect.value);
	} else {
		setControlState("<br>");
		ws.send("ad");
	}
};

function updateButton(object, enabled) {
    if(enabled) {
        object.classList.remove("button-switch-disabled");
        object.classList.add("button-switch-enabled");
    } else {
        object.classList.add("button-switch-disabled");
        object.classList.remove("button-switch-enabled");
    }
}