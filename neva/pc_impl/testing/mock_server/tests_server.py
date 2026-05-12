#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2016-2018 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

import os
import sys
import mimetypes
import json
import socket

try:
    import uwsgi
except ImportError:
    print("Please run it under uwsgi server", file=sys.stderr)
    exit(-1)


expectation_paths = (
    # Native Controls API
    # Color chooser
    "/NativeControls/changedColorChooser",
    "/NativeControls/closeColorChooser",
    "/NativeControls/closedColorChooser",
    "/NativeControls/openColorChooser",
    # File chooser
    "/NativeControls/retFileChooser",
    "/NativeControls/runFileChooser",
    # Native Dialogs API
    "/NativeDialogs/cancelJavaScriptDialog",
    "/NativeDialogs/retJavaScriptDialog",
    "/NativeDialogs/runJavaScriptDialog",
    # Sample Dialogs API
    "/Sample/callFunc",
    "/Sample/getPlatformValue",
    "/Sample/processDataResponse",
    "/Sample/processDataReq",
    "/Sample/sampleUpdate",
    # WAM API
    "/WAM/callFunc",
    "/WAM/commandSet",
    "/BrowserControl/callFunction",
    "/BrowserControl/sendCommand"
)

expectations = {i:[] for i in expectation_paths}


try:
    root = sys.argv[1]
except IndexError:
    print("\nPlease provide a server root as --pyargv command line switch to"
          "uwsgi server", file=sys.stderr)
    exit(-1)


def response(errcode, hdrs=None):
    def _(start_response, body='',
          ct='text/plain; charset=UTF-8',
          ce=None):
        headers = [('Content-Type', ct)]
        if ce:
            headers.append(('Content-Encoding', ce))
        if hdrs:
            headers.extend(hdrs)

        start_response(errcode, headers)
        if hasattr(body, 'encode'):
            return [body.encode('utf-8')]
        return [body]

    return _


ok = response("200 Ok")
created = response("201 Created")
bad = response("400 Bad request")
notfound = response("404 Not found")
proxy_auth = response("407 Proxy Authentication Required",
                      [("Proxy-Authenticate", "Basic realm=proxytester")])


def debug_print(*args):
    print("\n", *args, file=sys.stderr, flush=True)


def print_expectations():
    expectations_str = json.dumps(expectations, indent=True)
    debug_print("expectations = \n", expectations_str)


def content_type_magic(filename):
    mime, encoding = mimetypes.guess_type(filename)

    if not mime:
        mime = 'text/plain'

    return (mime, encoding)


def add_expectation(environ):
    data = environ["wsgi.input"].read()

    message_json = json.loads(data)
    data_request = message_json["httpRequest"]
    request_expectations = data_request["path"]
    data_response = json.dumps(message_json["httpResponse"])
    body_response = json.loads(data_response)["body"]
    debug_print("add_expectations to ", request_expectations)

    debug_print("\n\tbody_response =", body_response)
    expectations.setdefault(request_expectations, []).append(body_response)
    print_expectations()


def clear_expectations():
    debug_print("clear_expectations:")
    for k in expectations.keys():
        expectations[k] = []
    print_expectations()


def send_expect_response(path, start_response, response_type):
    debug_print("[send_expect_response] self.path = {}, response_type = {}".format(path, response_type))
    if expectations[path]:
        if not response_type:
            debug_print("expectation is in the set (no response_type):\n\t", json.dumps(expectations[path], indent=True))
            debug_print("send as a response 0th element:\n\t", expectations[path][0])
            return ok(start_response,
                      expectations[path].pop(0),
                      "application/json")
        else:
            filtered_expecations = list(filter(lambda x: response_type in x, expectations[path]))
            if not filtered_expecations:
                debug_print("expectation has not been yet set")
                return notfound(start_response, "text/plain")

            for el in filtered_expecations:
                debug_print("expectation is in the set:\n", el)
                expectations[path].remove(el)
                return ok(start_response, el, "application/json")
    else:
        debug_print("expectation has not been yet set")
        return notfound(start_response, "text/plain")


def application(environ, start_response):
    request_method = environ['REQUEST_METHOD']
    path_info = environ['PATH_INFO']
    query_string = environ['QUERY_STRING']
    resource = os.path.join(root, path_info.lstrip('/'))

    def http_headers(environ):
        return '\n'.join(
            '%s=%s' % (k, v) for k, v in iter(environ.items())
            if k.startswith("HTTP_")
        )

    if path_info == "/ws/echo":
        uwsgi.websocket_handshake(
            environ['HTTP_SEC_WEBSOCKET_KEY'], environ.get('HTTP_ORIGIN', ''))

        msg = uwsgi.websocket_recv().decode()
        uwsgi.websocket_send(json.dumps(
            {
                'headers': {
                    k: str(v) for k, v in iter(environ.items())
                    if k.startswith("HTTP_")
                },
                'message': msg
            },
            indent=True
        ))
        uwsgi.websocket_send(msg)
        return []

    if(path_info.startswith("http://") or
       path_info.startswith("https://") or
       request_method == "CONNECT"):
        if query_string == "auth":
            if "HTTP_PROXY_AUTHORIZATION" not in environ:
                return proxy_auth(start_response)

        return ok(start_response,
                  "Proxy request ({}):\n{}".format(request_method, http_headers(environ)),
                  "text/plain")

    if request_method == "PUT":
        debug_print("[do_PUT] path =", path_info)

        if path_info == "/clear":
            clear_expectations()
        else:
            add_expectation(environ)
        return created(start_response, "application/json")

    if request_method != "GET":
        return bad(start_response, '{"result": "Invalid request"}')

    if path_info == "/echo":
        return ok(start_response, http_headers(environ), "text/plain")

    if path_info == "/__PORT__":
        return ok(start_response, environ["SERVER_PORT"], "text/plain")

    if path_info == "/__IP__":
        server_ip = os.environ.get('NEVA_EMULATOR_SERVER_ADDRESS')
        if not server_ip:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('a.root-servers.net.', 0))
            server_ip, _ = s.getsockname()

        return ok(start_response, server_ip, "text/plain")

    if os.path.exists(resource):
        if os.path.isfile(resource):
            return ok(start_response,
                      open(resource, 'rb').read(),
                      *content_type_magic(resource))

        if os.path.isdir(resource):
            return ok(start_response,
                      '\n'.join(
                          '<p><a href="%(fn)s">%(fn)s</a></p>' % {'fn': fn}
                          for fn in os.listdir(resource)
                      ), "text/html")

    if path_info in expectation_paths:
        # This part will be changed when we resolve the issue with enormous
        # amount of LCP messages (NEVA-7036)
        response_type = None
        if query_string:
            debug_print("environ = [{}]\n".format(environ))
            debug_print("query_string =", query_string)
            _, response_type = query_string.split("=", 1)
        return send_expect_response(path_info, start_response, response_type)

    return notfound(start_response)
