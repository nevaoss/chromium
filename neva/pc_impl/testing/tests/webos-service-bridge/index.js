function getDeviceList(e) {
  try {
    var bridge = new WebOSServiceBridge();
  } catch (ex) {
    e.value = ex;
    return;
  }

  var url = 'luna://com.webos.service.mediaindexer/getDeviceList';

  bridge.onservicecallback = function (msg) {
    e.value += msg + '\t<--\t' + url + '\n';
  }

  var params = JSON.stringify({'subscribe': true});
  e.value = params + '\t-->\t' + url + '\n';

  bridge.call(url, params);
}

function onLoad() {
  getDeviceList(document.getElementById('console'));
}
