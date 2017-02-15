function DOOM() {
	var WIDTH = 320;
	var HEIGHT = 200;

	var screen = document.getElementById("screen");
	var ctx = screen.getContext("2d");
	var frame = ctx.createImageData(WIDTH, HEIGHT);
	var palette = new Uint8Array(256 * 3);

	for (var i = 0; i < 256; i++) {
		palette[i * 3 + 0] = i;
		palette[i * 3 + 1] = i;
		palette[i * 3 + 2] = i;
	}

	var fps = document.getElementById("fps");
	var rendered = 0;

	function updateFPS() {
		fps.innerText = rendered;
		rendered = 0;

		setTimeout(updateFPS, 1000);
	}

	setTimeout(updateFPS, 1000);

	function handleFrame(array) {
		for (var i = 0; i < array.byteLength; i++) {
			var index = array[i];
			var r = palette[index * 3 + 0];
			var g = palette[index * 3 + 1];
			var b = palette[index * 3 + 2];
			frame.data[i * 4 + 0] = r;
			frame.data[i * 4 + 1] = g;
			frame.data[i * 4 + 2] = b;
			frame.data[i * 4 + 3] = 255;
		}

		ctx.putImageData(frame, 0, 0);

		rendered++;
	}

	function handlePalette(array) {
		palette = array;
	}

	var events = [];

	function getKeyCode(key) {
		var code;

		if (key.length == 1) {
			code = key.charCodeAt(0);
		} else {
			switch (key) {
			case "ArrowLeft":    code = 276; break;
			case "ArrowRight":   code = 275; break;
			case "ArrowDown":    code = 274; break;
			case "ArrowUp":      code = 273; break;
			case "Enter":        code = 13;  break;
			case "Tab":          code = 9;   break;
			case "F1":           code = 282; break;
			case "F2":           code = 283; break;
			case "F3":           code = 284; break;
			case "F4":           code = 285; break;
			case "F5":           code = 286; break;
			case "F6":           code = 287; break;
			case "F7":           code = 288; break;
			case "F8":           code = 289; break;
			case "F9":           code = 290; break;
			case "F10":          code = 291; break;
			case "F11":          code = 292; break;
			case "F12":          code = 293; break;
			case "Backspace":    code = 8;   break;
			case "Pause":        code = 19;  break;
			case "Equals":       code = 272; break;
			case "Minus":        code = 269; break;
			case "Shift":        code = 303; break;
			case "Control":      code = 305; break;
			case "Alt":          code = 308; break;
			case "AltGraph":     code = 308; break;
			}
		}

		return code;
	}

	document.addEventListener("keydown", function (event) {
		if (!document.pointerLockElement && event.key == "Escape") {
			events.push([0, 0, 27]);
		} else {
			var code = getKeyCode(event.key);
			if (code)
				events.push([0, 0, code]);
		}
	});

	document.addEventListener("keyup", function (event) {
		if (!document.pointerLockElement && event.key == "Escape") {
			events.push([1, 0, 27]);
		} else {
			var code = getKeyCode(event.key);
			if (code)
				events.push([1, 0, code]);
		}
	});

	document.addEventListener("mousedown", function (event) {
		if (document.pointerLockElement)
			events.push([2, 1+event.button, 0]);
	});

	document.addEventListener("mouseup", function (event) {
		if (document.pointerLockElement)
			events.push([3, 1+event.button, 0]);
		else if (screen)
			screen.requestPointerLock();
	});

	document.addEventListener("mousemove", function (event) {
		if (document.pointerLockElement)
			events.push([4, 0, event.movementX, event.movementY]);
	});

	document.addEventListener("pointerlockchange", function (event) {
		if (!document.pointerLockElement) {
			events.push([0, 0, 27]); // Escape keydown
			events.push([1, 0, 27]); // Escape keyup
		}
	});

	var messageTypePrefix = "github.com/tsavola/npipe/";
	var session = NinchatClient.newSession();
	var peerUserId;
	var channelId;
	var myUserId;
	var myUserAuth;

	var tokens = location.hash.substring(1).split("/");
	if (tokens.length >= 1) {
		peerUserId = tokens[0];
	}
	if (tokens.length >= 2) {
		channelId = tokens[1];
	}
	if (tokens.length >= 4) {
		myUserId = tokens[2];
		myUserAuth = tokens[3];
	}

	if (!peerUserId) {
		alert("URL hash format: #peer-user-id[/channel-id[/my-user-id/my-user-auth]]");
		return;
	}

	function receive(payload) {
		var targetLength = -8;

		for (var i in payload) {
			targetLength += payload[i].byteLength;
		}

		if (targetLength <= 0)
			return;

		var header = new Uint32Array(payload[0]);
		var type = header[1];

		var target = new Uint8Array(targetLength);
		var targetOffset = 0;

		for (var i in payload) {
			var sourceOffset = 0;
			if (i == 0)
				sourceOffset = 8;
			var source = new Uint8Array(payload[i], sourceOffset);
			target.set(source, targetOffset);
			targetOffset += source.byteLength;
		}

		switch (type) {
		case 0x66726d65: // frme
			handleFrame(target);
			break;

		case 0x706c7474: // pltt
			handlePalette(target);
			break;

		default:
			alert("Unknown packet type");
			break;
		}
	}

	function send() {
		var buffer = new ArrayBuffer(8 + 6 * events.length);
		var data = new DataView(buffer);

		data.setUint32(0, buffer.byteLength, true);
		data.setUint32(4, 0x65766e74, true); // evnt

		for (var i in events) {
			var offset = 8 + 6 * i;
			var event = events[i];

			data.setUint8(offset + 0, event[0]);
			data.setUint8(offset + 1, event[1]);

			if (event.length == 3) {
				data.setUint32(offset + 2, event[2], true);
			} else {
				data.setUint16(offset + 2, event[2], true);
				data.setUint16(offset + 4, event[3], true);
			}
		}

		events = [];

		var header = {
			action:       "send_message",
			action_id:    null,
			message_type: messageTypePrefix + "evnt",
			message_fold: true,
			message_ttl:  0.1
		};

		if (channelId)
			header.channel_id = channelId;
		else
			header.user_id = peerUserId;

		session.send(header, [buffer]);
	}

	var sessionCreated = false;

	session.onSessionEvent(function(header) {
		switch (header.event) {
		case "session_created":
			if (sessionCreated) {
				session.close();
				alert("session lost");
				return;
			}

			sessionCreated = true;
			myUserId = header.user_id;

			if (channelId && !myUserAuth) {
				session.send({
					action:     "follow_channel",
					channel_id: channelId
				});
			} else {
				send();
			}
			break;

		default:
			session.close();
			alert(header.event);
		}
	});

	session.onEvent(function(header, payload) {
		switch (header.event) {
		case "message_received":
			if (header.message_user_id == myUserId)
				break; // reply

			var forMe = false;

			if (channelId)
				forMe = (header.channel_id == channelId && header.message_user_id == peerUserId);
			else
				forMe = (header.user_id == peerUserId);

			if (forMe) {
				receive(payload);

				if (myUserAuth)
					send();
			}
			break;

		case "error":
			session.close();
			alert(header.event);
			break;
		}
	});

	session.onConnState(function(state) {
		console.log("connection:", state);
	});

	session.onLog(function(message) {
		console.log("client:", message);
	});

	var params = {
		message_types: [
			messageTypePrefix + "frme",
			messageTypePrefix + "pltt"
		]
	};

	if (myUserId) {
		params.user_id = myUserId;
		params.user_auth = myUserAuth;
	}

	session.setParams(params);

	session.open();
}
