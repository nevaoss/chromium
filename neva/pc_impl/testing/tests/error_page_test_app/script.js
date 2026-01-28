window.onload = () => {
  setTimeout(() => {
    var webview = document.getElementById("webview1");
    webview.src='https://badssl.com';
  }, 500);
}
