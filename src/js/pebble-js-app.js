//  file: pebble-js-app.js
//  auth: Matthew Clark, SetPebble
//var setPebbleToken = 'C45W'; //theirs

var setPebbleToken = '2TU4'; //mine
console.log("substance, setpebble token " + setPebbleToken + " for app v. 1.3.x");
console.log("request.open( http://x.SetPebble.com/api/" + setPebbleToken + '/' + Pebble.getAccountToken());
Pebble.addEventListener('ready', function(e) {
});

Pebble.addEventListener('appmessage', function(e) {
    key = e.payload.action;
    if (typeof(key) != 'undefined') {
        var settings = localStorage.getItem(setPebbleToken);
        if (typeof(settings) == 'string') {
            try {
                Pebble.sendAppMessage(JSON.parse(settings));
            } catch (e) {
            }
        }
        var request = new XMLHttpRequest();
        request.open('GET', 'http://x.SetPebble.com/api/' + setPebbleToken + '/' + Pebble.getAccountToken(), true);
        request.onload = function(e) {
            if (request.readyState == 4)
                if (request.status == 200)
                    try {
                        Pebble.sendAppMessage(JSON.parse(request.responseText));
                    } catch (e) {
                    }
        }
        request.send(null);
    }
});

Pebble.addEventListener('showConfiguration', function(e) {
    Pebble.openURL('http://x.SetPebble.com/' + setPebbleToken + '/' + Pebble.getAccountToken());
});

Pebble.addEventListener('webviewclosed', function(e) {
    if ((typeof(e.response) == 'string') && (e.response.length > 0)) {
        try {
            Pebble.sendAppMessage(JSON.parse(e.response));
            localStorage.setItem(setPebbleToken, e.response);
        } catch(e) {
        }
    }
});