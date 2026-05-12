# wam_demo.bash
## Neva space page
- http://collab.lge.com/main/pages/viewpage.action?pageId=2365117491#HowtorunWAMdemoonPC/ubuntu-ExecutionscriptforLinux-wam_demo.bash

wam_demo.bash script is prepared for easy execution of wam_demo together with
wam_demo_emulator_new.html. User can change default values prepared by variables
of this scirpt by setting environmental variables with the same name.

## Default values are used if the variable is not set

```console
WAM_EMULATOR_PAGE=\${WAM_EMULATOR_PAGE-http://localhost:8888/wam_emulator_page_new.html}
USER_DATA_DIR=\${USER_DATA_DIR-\$(mktemp -d /tmp/\$\$.XXXXXX)}
PIPE=\${PIPE-\${USER_DATA_DIR}/pipe}
WAM_DEMO=\${WAM_DEMO-\${PWD}/wam_example/wam_demo}
CHROMIUM=\${CHROMIUM-\${PWD}/chrome_gcc/chrome}
MOCK_SERVER=\${MOCK_SERVER-\${PWD}/testing/mock_server/run_emulator_server.bash}
TESTS_SERVER=\${TESTS_SERVER-\${PWD}/testing/mock_server/tests_server.py}
```

### Example
```console
// --full-screen is passed to command-line argument(optional) of wam_demo
// via handle argument \$@.
// The file path of CHROMIUM is used for launching wam_demo_emulator_page_new.html.
testbed$ CHROMIUM=/usr/bin/google-chrome testing/scripts/wam_demo.bash --full-screen
```

## How to prepare testbed environment for this script

```console
$ mkdir testbed
$ cd testbed

// Get Neva PC build tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/pc-build/20717/venus.lge.net-pc-build-20717.bin.tar.gz
testbed$ tar xvf venus.lge.net-pc-build-20717.bin.tar.gz

// Get Neva PC testing tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/pc-build/20717/venus.lge.net-pc-build-20717.testing.tar.gz
testbed$ tar xvf venus.lge.net-pc-build-20717.testing.tar.gz neva/pc_impl/testing --strip-components=2

// Run weston
testbed$ weston &

// Run script
testbed$ testing/scripts/wam_demo.bash
```

# enact_browser@browser_shell.bash
## Neva space page
- http://collab.lge.com/main/pages/viewpage.action?pageId=605210208#HowtorunNevaBrowseronPC/ubuntu-ExecutionscriptforLinux-enact_browser@browser_shell.bash

This script is prepared for easy execution of enact browser for browser_shell.
User can change default values prepared by variables of this scirpt by
setting environmental variables with the same name.

## Default values are used if the variable is not set

```console
BROWSER_SHELL=\${BROWSER_SHELL-\${PWD}/browser_shell/browser_shell}
ENACT_BROWSER=\${ENACT_BROWSER-\${PWD}/dist/index.html}
APP_ID=\${APP_ID-com.webos.app.test.browsershell}
SHELL_WEB_APIS=\${SHELL_WEB_APIS-v8/browser_shell,v8/chrome_extensions}
USER_DATA_DIR=\${USER_DATA_DIR-/tmp/browser_shell}
```

### Example
```console
// --shell-fullscreen is passed to command-line argument(optional) of browser_shell via
// handle argument \$@.
// The file path of BROWSER_SHELL is the executable used for launching Enact browser.
$ BROWSER_SHELL="/home/neva/testbed2/browser_shell/browser_shell" SHELL_WEB_APIS="v8/browser_shell" testing/scripts/enact_browser@browser_shell.bash --shell-fullscreen
```

## How to prepare testbed environment for this script

```console
$ mkdir testbed
$ cd testbed

// Get Neva PC build tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/pc-build/20717/venus.lge.net-pc-build-20717.bin.tar.gz
testbed$ tar xvf venus.lge.net-pc-build-20717.bin.tar.gz

// Get Neva PC testing tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/pc-build/20717/venus.lge.net-pc-build-20717.testing.tar.gz
testbed$ tar xvf venus.lge.net-pc-build-20717.testing.tar.gz neva/pc_impl/testing --strip-components=2

// Get Neva Browser tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/neva-browser/3745/venus.lge.net-neva-browser-3745.dist.tar.gz
testbed$ tar xvf venus.lge.net-neva-browser-3745.dist.tar.gz

// Run weston
testbed$ weston &

// Run script
testbed$ testing/scripts/enact_browser@browser_shell.bash
```

# enact_browser@app_shell.bash
## Neva space page
- http://collab.lge.com/main/pages/viewpage.action?pageId=605210208#HowtorunNevaBrowseronPC/ubuntu-ExecutionscriptforLinux-enact_browser@app_shell.bash

enact_browser@app_shell.bash is prepared for easy execution of enact
browser for app_shell. User can change default values prepared by
variables of this scirpt by setting environmental variables with the
same name.

## Default values are used if the variable is not set

```console
APP_SHELL=\${APP_SHELL-\${PWD}/app_shell/app_shell}
ENACT_BROWSER=\${ENACT_BROWSER-\${PWD}/enact_browser_app_shell/dist/}
APP_ID=\${APP_ID-com.webos.app.test.appshell}
USER_DATA_DIR=\${USER_DATA_DIR-/tmp/app_shell}
```

### Example
```console
// --no-sandbox is passed to command-line argument(optional) via handle argument \$@.
// The file path of APP_SHELL is the executable used for launching Enact browser.
$ APP_SHELL="/home/neva/testbed2/app_shell/app_shell" testing/scripts/enact_browser@app_shell.bash --no-sandbox
```

## How to prepare testbed environment for this script

```console
$ mkdir testbed
$ cd testbed

// Get Neva PC build tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/pc-build/20717/venus.lge.net-pc-build-20717.bin.tar.gz
testbed$ tar xvf venus.lge.net-pc-build-20717.bin.tar.gz

// Get Neva PC testing tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/pc-build/20717/venus.lge.net-pc-build-20717.testing.tar.gz
testbed$ tar xvf venus.lge.net-pc-build-20717.testing.tar.gz neva/pc_impl/testing --strip-components=2

// Get Neva Browser tarball
testbed$ wget http://neva-release.lge.com/neva/jenkins-artifacts/venus.lge.net/NEVA/neva-browser/3744/venus.lge.net-neva-browser-3744.dist.tar.gz
testbed$ mkdir enact_browser_app_shell
testbed$ tar xvf venus.lge.net-neva-browser-3745.dist.tar.gz -C enact_browser_app_shell

// Run weston
testbed$ weston &

// Run script
testbed$ testing/scripts/enact_browser@app_shell.bash
```
