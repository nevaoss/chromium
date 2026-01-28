"use strict";

const NO_FRAME     = "thick solid black";
const BLUE_FRAME   = "thick solid #29B0D9";
const GREEN_FRAME  = "thick solid green";
const RED_FRAME    = "thick solid red";
const YELLOW_FRAME = "thick solid yellow";

var activeNode= "duckduckgo";
var activeApp= "duckduckgo";
var poll_period = 100;

var appsDescription = {
  duckduckgo: {
    default_url: 'http://www.duckduckgo.com',
    control_name: 'duckduckgo.img',
    image_src: './images/duckduckgo.png',
  },
  w3c: {
    default_url: 'http://www.w3.org',
    control_name: 'w3c.img',
    image_src: './images/w3c.png',
  },
  google: {
    default_url: 'http://www.google.com',
    control_name: 'google.img',
    image_src: './images/google.png',
  },
  wiki: {
    default_url: 'http://www.wikipedia.org',
    control_name: 'wiki.img',
    image_src: './images/wiki.png',
  },
  about_blank: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/about_blank.html',
    control_name: 'about_blank.img',
    image_src: './images/about_blank.png',
  },
  bbc: {
    default_url: 'http://bbc.co.uk',
    control_name: 'bbc.img',
    image_src: './images/bbc.png',
  },
  youtube: {
    default_url: 'https://www.youtube.com/watch?v=YE7VzlLtp-4&t=1s',
    control_name: 'youtube.img',
    image_src: './images/youtube.png',
  },
  hid: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/hid.html',
    control_name: 'hid.img',
    image_src: './images/hid.png',
  },
  auto: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/auto_integration_test_page.html',
    control_name: 'auto.img',
    image_src: './images/auto.png',
  },
  manual: {
    // Warning! This application has special permissions to make luna calls.
    // Do not rename it and do not change an URL.
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/test_page.html',
    control_name: 'manual.img',
    image_src: './images/manual.png',
  },
  xframe: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/xframeoptions_test_page.html',
    control_name: 'xframe.img',
    image_src: './images/xframe.png',
  },
  htmlkb: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/html_system_keyboard_test_page.html',
    control_name: 'htmlkb.img',
    image_src: './images/htmlvkb.png',
  },
  browsercontrol: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/browser_control_page.html',
    control_name: 'browsercontrol.img',
    image_src: './images/browsercontrol.png',
  },
  vps: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/viewport_size_test_page.html',
    control_name: 'vps.img',
    image_src: './images/vps.png',
  },
  fbt: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/fake_bold_text_test_page.html',
    control_name: 'fbt.img',
    image_src: './images/fbt.png',
  },
  visibilitystate: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/visibility_state.html',
    control_name: 'visibilitystate.img',
    image_src: './images/vs.png',
  },
  leftgroup: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/group.html?x=0&w=520&bk=orange',
    control_name: 'leftgroup.img',
    image_src: './images/leftgroup.png',
  },
  rightgroup: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/group.html?x=520&w=1400&bk=green',
    control_name: 'rightgroup.img',
    image_src: './images/rightgroup.png',
  },
  websocket: {
    default_url:
        'http://@@EMULATOR_SERVER_IP@@:@@EMULATOR_SERVER_PORT@@/websocket.html',
    control_name: 'websocket.img',
    image_src: './images/ws.png',
  },
  cursor: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/cursor_test_page.html',
    control_name: 'cursor.img',
    image_src: './images/cursor.png',
  },
  quota: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/test_quota.html',
    control_name: 'quota.img',
    image_src: './images/quota.png',
  },
  use_unlimited_media_policy: {
    default_url:
        'http://@@EMULATOR_SERVER_ADDRESS@@/use_unlimited_media_policy.html',
    control_name: 'use_unlimited_media_policy.img',
    image_src: './images/use_unlimited_media_policy.png',
  },
  inputtype: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/input_type.html',
    control_name: 'inputtype.img',
    image_src: './images/input_type.png',
  },
  iframe: {
    default_url: 'http://@@EMULATOR_SERVER_ADDRESS@@/iframe_injection_test.html',
    control_name: 'iframe.img',
    image_src: './images/iframe.png',
  },
  'com.palm.app.settings': {
    default_url:
        'file:///usr/palm/applications/com.palm.app.settings/index.html',
    control_name: 'webos_settings.img',
    image_src: './images/webos_settings.png',
  }
}

{
  let x = 100;
  let y = 100;
  for (let node in appsDescription) {
    appsDescription[node].width = 640;
    appsDescription[node].height = 480;
    appsDescription[node].pos_x = x;
    appsDescription[node].pos_y = y;
    appsDescription[node].url = appsDescription[node].default_url;
    appsDescription[node].zoom = 100;
    appsDescription[node].pid = 'n/a';
    appsDescription[node].injections = '';
    appsDescription[node].keyboard_visibility = 'n/a';
    appsDescription[node].viewport_width = 0;
    appsDescription[node].viewport_height = 0;
    x += 50;
    y += 50;
  }
}

window.onload = onLoad;

document.addEventListener("DOMContentLoaded", setButtonsDisabled);
function setButtonsDisabled() {
  var buttons = document.getElementsByTagName('button');
  for (var i = 0; i < buttons.length; i++) {
    buttons[i].disabled = true;
  }
}

function getNodeByUrl(url) {
  for (let node in appsDescription) {
    if (appsDescription[node].url == url) {
      return node;
    }
  }
  return null;
}

function callFunc(command, node) {
  let pos = command.indexOf(':');
  let arg;
  if (pos > 0) {
    arg = command.substring(pos + 1, command.length);
    command = command.substring(0, pos);
  }
  if (!(node in appsDescription)) {
    console.log("Command <", command, "> has incorrect URL");
    return;
  }
  if (command == 'appClosed') {
    document.getElementById(appsDescription[node].control_name).
        style.borderBottom = NO_FRAME;
    appsDescription[node].pid = 'n/a';
    appsDescription[node].keyboard_visibility = 'n/a';
    if (node == activeNode) {
      document.getElementById('render_process_pid').value = 'n/a';
      document.getElementById('get_render_process_pid').value = '';
      document.getElementById('keyboard_visibility').value = 'n/a';
    }
  } else if (command == 'appStarted') {
    document.getElementById(appsDescription[node].control_name).
        style.borderBottom = GREEN_FRAME;
  } else if (command == 'zoomUpdated') {
    if (typeof(arg) != 'undefined') {
      appsDescription[node].zoom = arg;
      if (node == activeNode)
        document.getElementById('zoom_factor').value = arg + '%';
    }
  } else if (command == 'keyboardVisibility') {
    appsDescription[node].keyboard_visibility = arg;
    if (node == activeNode) {
      document.getElementById('keyboard_visibility').value = arg;
    }
  } else if (command == 'canGoBackAbility') {
    if (node == activeNode) {
      document.getElementById('can_go_back').value = arg;
    }
  } else if (command == 'documentTitle') {
    if (node == activeNode) {
      document.getElementById('document_title').value = arg;
    }
  } else if (command == 'documentUrl') {
    if (node == activeNode) {
      document.getElementById('document_url').value = arg;
    }
  } else if (command == 'profileCreated') {
    if (node == activeNode) {
      document.getElementById('is_profile_created').value = arg;
    }
  } else if (command == 'devToolsEndpoint') {
    if (node == activeNode) {
      var [host, port] = arg.split(",");
      if (parseInt(port) > 0) {
        host = (host == "unknown") ? kHost : host;
        document.getElementById("dev_tools_link").href = "http://" + host + ":" + port;
        document.getElementById("dev_tools_link").text = "http://" + host + ":" + port;
        document.getElementById('dev_tools_port').value = port;
      } else
        document.getElementById('dev_tools_port').value = "none";
    }
  } else if (command == 'pidUpdated') {
    if (typeof(arg) != 'undefined') {
      appsDescription[node].pid = arg;
      if (node == activeNode) {
        document.getElementById('render_process_pid').value = arg;
        document.getElementById('get_render_process_pid').value = '';
      }
    }
  } else if (command == 'pidRequested') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('get_render_process_pid').value = arg;
  } else if (command == 'userAgentIs') {
    if (typeof(arg) != 'undefined' && node == activeNode)
    document.getElementById('user_agent_output').value = arg;
  } else if (command == 'processGone') {
    document.getElementById(appsDescription[node].control_name).
        style.borderBottom = RED_FRAME;
    appsDescription[node].pid = 'n/a';
    if (node == activeNode) {
      document.getElementById('render_process_pid').value = 'n/a';
      document.getElementById('get_render_process_pid').value = '';
    }
  } else if (command == 'loadFailed') {
    document.getElementById(appsDescription[node].control_name).
        style.borderBottom = YELLOW_FRAME;
  } else if (command == 'loadFinished') {
    document.getElementById(appsDescription[node].control_name).
        style.borderBottom = GREEN_FRAME;
  } else if (command == 'windowStateRequested') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('window_state_output').value = arg;
  } else if (command == 'windowStateAboutToChangeRequested') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('window_state_about_to_change_output').value = arg;
  } else if (command == 'cursorUpdated') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('cursor_visibility_state').value = arg;
  } else if (command == 'loadingEndTime') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('loading_end_time').value = arg;
  } else if (command == 'firstPaintTime') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('first_paint_time').value = arg;
  } else if (command == 'firstContentfulPaintTime') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('first_contentful_paint_time').value = arg;
  } else if (command == 'firstImagePaintTime') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('first_image_paint_time').value = arg;
  } else if (command == 'firstMeaningfulPaintTime') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('first_meaningful_paint_time').value = arg;
  } else if (command == 'largestContentfulPaintTime') {
    if (typeof(arg) != 'undefined' && node == activeNode)
      document.getElementById('largest_contentful_paint_time').value = arg;
  } else if (command == 'pageLifecycleState') {
    if (node == activeNode) {
      document.getElementById('pagelifecycle_state').value = arg;
    }
  }
}

function callFuncCallback(params_JSON) {
  let args = JSON.parse(params_JSON);
  if (args.hasOwnProperty('arg1') && args.hasOwnProperty('arg2')) {
    callFunc(args.arg1, args.arg2);
  } else {
    console.error("Invalid params for 'callFuncCallback()'.")
  }
}

function initEmulatorPage() {
  addURLForPolling(kWAM_callFunc, callFuncCallback);
}

function getFocusedElementURL() {
  if (!(activeNode in appsDescription)) {
    console.log("Element is not correct");
    return '';
  }
  return appsDescription[activeNode].url;
}

function getFocusedElementDefaultURL() {
  if (!(activeNode in appsDescription)) {
    console.log("Element is not correct");
    return '';
  }
  return appsDescription[activeNode].default_url;
}

function getFocusedElementDefaultImgSrc() {
  if (!(activeNode in appsDescription)) {
    console.log("Element is not correct");
    return '';
  }
  return appsDescription[activeNode].image_src;
}

function getFocusedElementAppId() {
  if (!(activeNode in appsDescription)) {
    console.log("Element is not correct");
    return '';
  }
  return activeNode;
}

function checkChangedURL() {
  if (getFocusedElementURL() != getFocusedElementDefaultURL()) {
    let nodes = document.getElementById(activeNode).getElementsByTagName("img");
    for (let i=0; i<nodes.length; i++) {
      nodes[i].src = './images/urlchanged.png';
    }
  } else {
    let nodes = document.getElementById(activeNode).getElementsByTagName("img");
    for (let i=0; i<nodes.length; i++) {
      nodes[i].src = getFocusedElementDefaultImgSrc();
    }
  }
}

function updateAppWindow() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    width: document.getElementById('launch_width_input').value,
    height: document.getElementById('launch_height_input').value,
    pos_x: appsDescription[activeNode].pos_x.toString(),
    pos_y: appsDescription[activeNode].pos_y.toString(),
    cmd: 'updateAppWindow'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function updateWindowBackgroundColor() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    red: document.getElementById('red_channel_input').value,
    green: document.getElementById('green_channel_input').value,
    blue: document.getElementById('blue_channel_input').value,
    cmd: 'setBackgroundColor'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setUserAgent(agent) {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    user_agent: document.getElementById('user_agent_input').value,
    cmd: 'setUserAgent'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setFontFamily(fontFamily) {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    font_family: fontFamily,
    cmd: 'setFontFamily'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setVisibilityState(visibilityState) {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    visibility_state: visibilityState,
    cmd: 'setVisibilityState'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setWindowProperty() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    name: document.getElementById("set_window_property_name_input").value,
    value: document.getElementById("set_window_property_value_input").value,
    cmd: 'setWindowProperty'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function disableBackButton() {
  var disable = document.getElementById('disable_back_button').checked;
  runDirectCommandWithArgs(
      'disableBackButton', {'disable': disable.toString()});
}

function enableProxyServer() {
  var enable = document.getElementById('proxy_server_enabled').checked;
  document.getElementById("proxy_server_name").disabled = !enable;
  document.getElementById("proxy_server_port").disabled = !enable;
  document.getElementById("proxy_server_login").disabled = !enable;
  document.getElementById("proxy_server_password").disabled = !enable;
  document.getElementById("proxy_server_bypass_list").disabled = !enable;
}

function resetToDefaultURL() {
  appsDescription[activeNode].url = getFocusedElementDefaultURL();
  document.getElementById("url_data_value_input").value =
      getFocusedElementDefaultURL();

  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'changeURL'
  };
  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
  checkChangedURL();
}

function changeURL() {
  if (getFocusedElementURL() == document.getElementById("url_data_value_input").value) {
    return;
  }

  appsDescription[activeNode].url = document.getElementById("url_data_value_input").value;
  let url = appsDescription[activeNode].url;
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'changeURL'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
  checkChangedURL();
}

function replaceBaseURL() {
  appsDescription[activeNode].url = document.getElementById("url_data_value_input").value;
  let url = appsDescription[activeNode].url;
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'replaceBaseURL'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
  checkChangedURL();
}

function enableSizePos(enable) {
  disabled = !enable;
  let names = [
    'launch_width_input', 'launch_height_input'
  ];
  for (let name of names) {
    document.getElementById(name).disabled = disabled;
  }
}

function onFullScreenChanged()  {
  enableSizePos(!document.getElementById('full_screen_cb').checked);
}

function prepareLaunchAppParams(command) {
  command.width = document.getElementById('launch_width_input').value;
  command.height = document.getElementById('launch_height_input').value;
  let node = getNodeByUrl(command.url);
  command.pos_x = appsDescription[node].pos_x.toString();
  command.pos_y = appsDescription[node].pos_y.toString();
  command.full_screen =
    document.getElementById('full_screen_cb').checked.toString();
  command.frameless_window =
    document.getElementById('frameless_window_cb').checked.toString();
  command.transparent_background =
    document.getElementById('transparent_background_cb').checked.toString();
  command.injections =
    document.getElementById('injections_input').value;
  command.viewport_width = document.getElementById('viewport_width_input').value;
  command.viewport_height = document.getElementById('viewport_height_input').value;
  document.getElementById('loading_end_time').value = "";
  document.getElementById('first_paint_time').value = "";
  document.getElementById('first_contentful_paint_time').value = "";
  document.getElementById('first_image_paint_time').value = "";
  document.getElementById('first_meaningful_paint_time').value = "";
  document.getElementById('largest_contentful_paint_time').value = "";
}

function runDirectCommand(cmd, appId) {
  return runDirectCommandWithArgs(cmd, {}, appId);
}

function runDirectCommandWithArgs(cmd, args, appId) {
  let url;

  if (typeof(appId) == 'undefined')
    appId = getFocusedElementAppId();

  if (appId in appsDescription) {
    url = appsDescription[appId].url;
  } else {
    console.log("Element is not correct");
    return;
  }

  let command = args;
  command.app_id = appId;
  command.url = url;
  command.cmd = cmd;

  if (cmd == 'launchApp' || cmd == 'launchHiddenApp') {
    prepareLaunchAppParams(command);
  }
  console.log('command = ', command);
  console.log('JL:: command = ', command);
  console.log('JL:: JSON.stringify = ', JSON.stringify(command));
  createExpectation(kWAM_commandSet, JSON.stringify(command));
  checkChangedURL();
}

function onLoad() {
  initEmulatorPage();
  setTimeout(periodicPoll, poll_period);
  onClick('duckduckgo');
  document.getElementById("url_data_value_input").value = getFocusedElementURL();
  document.getElementById('zoom_factor').value = appsDescription[activeNode].zoom + '%';
  document.getElementById('render_process_pid').value = appsDescription[activeNode].pid;
  document.getElementById('keyboard_visibility').value = appsDescription[activeNode].keyboard_visibility;

  var xhr = new XMLHttpRequest();
  xhr.open('GET', kEmulatorBaseURL + "__IP__", true);

  xhr.onload = function() {
    if (this.status == 200) {
      var localIPAddress = this.responseText;
      var localIPSocket = localIPAddress + ":" + kPort;
      for (let node in appsDescription) {
        if (appsDescription[node].default_url.indexOf(
                "@@EMULATOR_SERVER_ADDRESS@@") != -1) {
          appsDescription[node].url = appsDescription[node].default_url =
              appsDescription[node].default_url.replace(
                  "@@EMULATOR_SERVER_ADDRESS@@", localIPSocket);
        }
        if (appsDescription[node].default_url.indexOf(
                "@@EMULATOR_SERVER_IP@@") != -1) {
          appsDescription[node].url = appsDescription[node].default_url =
              appsDescription[node].default_url.replace(
                  "@@EMULATOR_SERVER_IP@@", localIPAddress);
        }
      }
      document.getElementById("proxy_server_name").value = localIPAddress;
      var buttons = document.getElementsByTagName('button');
      for (var i = 0; i < buttons.length; i++) {
        buttons[i].disabled = !buttons[i].disabled;
      }
    }
  };

  xhr.onerror = function() { reject(new Error("Network Error")); };

  xhr.send();

  var xhr2 = new XMLHttpRequest();
  xhr2.open('GET', kEmulatorBaseURL + "__PORT__", true);

  xhr2.onload = function() {
    if (this.status == 200) {
      var localPort = this.responseText;
      for (let node in appsDescription) {
        if (appsDescription[node].default_url.indexOf(
                "@@EMULATOR_SERVER_PORT@@") != -1) {
          appsDescription[node].url = appsDescription[node].default_url =
              appsDescription[node].default_url.replace(
                  "@@EMULATOR_SERVER_PORT@@", localPort);
        }
      }
      document.getElementById("proxy_server_port").value = localPort;
    }
  };

  xhr2.onerror = function() { reject(new Error("Network Error")); };

  xhr2.send();
}

function groupCmd(cmd) {
  for (let node in appsDescription) {
    runDirectCommand(cmd, node);
  }
}

function onClick(id) {
  document.getElementById(activeNode).style.border = NO_FRAME;
  activeNode = id;
  activeApp = appsDescription[id].control_name;
  document.getElementById(activeNode).style.border = BLUE_FRAME;
  document.getElementById('launch_width_input').value =
      appsDescription[activeNode].width;
  document.getElementById('launch_height_input').value =
      appsDescription[activeNode].height;
  console.log("new element focused:" + id);
  document.getElementById("url_data_value_input").value = getFocusedElementURL();
  document.getElementById('zoom_factor').value = appsDescription[activeNode].zoom + '%';
  document.getElementById('render_process_pid').value = appsDescription[activeNode].pid;
  document.getElementById('keyboard_visibility').value = appsDescription[activeNode].keyboard_visibility;
  document.getElementById('get_render_process_pid').value = '';
  document.getElementById('red_channel_input').value = 29;
  document.getElementById('green_channel_input').value = 29;
  document.getElementById('blue_channel_input').value = 29;
  document.getElementById('injections_input').value =
      appsDescription[activeNode].injections;
  document.getElementById('viewport_width_input').value =
      appsDescription[activeNode].viewport_width;
  document.getElementById('viewport_height_input').value =
      appsDescription[activeNode].viewport_height;
}

function onUpdateWidth() {
  appsDescription[activeNode].width = document.getElementById('launch_width_input').value;
}

function onUpdateHeight() {
  appsDescription[activeNode].height = document.getElementById('launch_height_input').value;
}

function onZoomDown() {
  if (appsDescription[activeNode].zoom >= 40) {
    var zoom_factor = parseInt(appsDescription[activeNode].zoom, 10)
    zoom_factor -= 20;
    let command = {
      app_id: getFocusedElementAppId(),
      url: appsDescription[activeNode].url,
      zoom: zoom_factor.toString(),
      cmd: 'updateZoom'
    };

    console.log('command = ', command);
    createExpectation(kWAM_commandSet, JSON.stringify(command));
  }
}

function onZoomUp() {
  if (appsDescription[activeNode].zoom <= 280) {
    var zoom_factor = parseInt(appsDescription[activeNode].zoom, 10)
    zoom_factor += 20;
    let command = {
      app_id: getFocusedElementAppId(),
      url: appsDescription[activeNode].url,
      zoom: zoom_factor.toString(),
      cmd: 'updateZoom'
    };

    console.log('command = ', command);
    createExpectation(kWAM_commandSet, JSON.stringify(command));
  }
}

function loadInjections() {
  if (getFocusedElementURL() == document.getElementById("injections_input").value) {
    return;
  }

  appsDescription[activeNode].injections = document.getElementById("injections_input").value;
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    injections: appsDescription[activeNode].injections,
    cmd: 'loadInjections'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function runJavaScript() {
  runDirectCommandWithArgs('runJavaScript', {
    'javascript_code': document.getElementById('javascript_code').value
  })
}

function runJSInAllFrames() {
  runDirectCommandWithArgs('runJSInAllFrames', {
    'javascript_code': document.getElementById('javascript_code').value
  })
}

function setInspectable(enable) {
  runDirectCommandWithArgs('setInspectable', { 'enable': enable.toString() });

  if (!enable) {
    document.getElementById('dev_tools_port').value = 'none';
    document.getElementById('dev_tools_link').href = '#';
    document.getElementById('dev_tools_link').text = '...';
  }
}

function setWindowState(state) {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    window_state: state,
    cmd: 'setWindowState'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setProxyServer() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    proxy_enabled:
        document.getElementById('proxy_server_enabled').checked.toString(),
    proxy_server: document.getElementById("proxy_server_name").value,
    proxy_port: document.getElementById("proxy_server_port").value,
    proxy_login: document.getElementById("proxy_server_login").value,
    proxy_password: document.getElementById("proxy_server_password").value,
    proxy_bypass_list:
        document.getElementById("proxy_server_bypass_list").value,
    cmd: 'setProxyServer'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setExtraWebSocketHeader() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    header_name: document.getElementById("header_name").value,
    header_value: document.getElementById("header_value").value,
    cmd: 'setExtraWebSocketHeader'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setProfile() {
  runDirectCommandWithArgs('setProfile', {
    'profile': document.getElementById('profile').value
  });
}

function setBoardType() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    board_type: document.getElementById("set_board_type_input").value,
    cmd: 'setBoardType'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function notifyMemoryPressure(level) {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    memory_pressure_level: level,
    cmd: 'notifyMemoryPressure'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function addUserStyleSheet() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    user_stylesheet: document.getElementById("user_style_sheet").value,
    cmd: 'addUserStyleSheet'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function clearBrowsingData() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    browsing_data_mask:
        document.getElementById('remove_browsing_data_mask').value,
    cmd: 'clearBrowsingData'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function resizeWindow() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    window_width: document.getElementById('window_width_input').value,
    window_height: document.getElementById('window_height_input').value,
    cmd: 'resizeWindow'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function onXInputActivate() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'xinputActivate'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function onXInputDeactivate() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'xinputDeactivate'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function onXInputInvokeAction() {
  let key = document.getElementById('XInputSel').value;
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'xinputInvokeAction',
    key_sym: key
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setOpacity() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    opacity: document.getElementById('opacity_input').value,
    cmd: 'setOpacity'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setScaleFactor() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    scale_factor: document.getElementById('scale_factor_input').value,
    cmd: 'setScaleFactor'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setMediaCodecCapability() {
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    media_codec_capability: document.getElementById('media_codec_capability_json').value,
    cmd: 'setMediaCodecCapability'
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setGroupKeyMask() {
  let selector = document.getElementById('group_key_mask_selector'), selector_option;
  let masks = 0;

  for (let i = 0; i < selector.length; i++) {
    selector_option = selector.options[i];
    if (selector_option.selected) {
      masks = masks | selector_option.value;
    }
  }

  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'setGroupKeyMask',
    key_mask: masks.toString()
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setKeyMask(set) {
  let mask = document.getElementById('key_mask_selector').value;
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'setKeyMask',
    key_mask: mask,
    set: set
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function setInputRegion() {
  let region = document.getElementById('input_region_selector').value;
  let command = {
    app_id: getFocusedElementAppId(),
    url: appsDescription[activeNode].url,
    cmd: 'setInputRegion',
    input_region: region
  };

  console.log('command = ', command);
  createExpectation(kWAM_commandSet, JSON.stringify(command));
}

function launchOwner() {
  runDirectCommandWithArgs('launchApp', {
    'group_name': document.getElementById('groupName').value,
    'is_owner': 'true'
  });
}

function launchLayer() {
  runDirectCommandWithArgs('launchApp', {
    'group_name': document.getElementById('groupName').value,
    'is_owner': 'false'
  });
}

function setAcceptLanguages() {
  runDirectCommandWithArgs('setAcceptLanguages', {
    'accept_languages': document.getElementById('accept_languages').value
  });
}
