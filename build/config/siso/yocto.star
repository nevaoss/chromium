# -*- bazel-starlark -*-
# Copyright 2025 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
"""Siso configuration for yocto."""

load("@builtin//path.star", "path")
load("@builtin//struct.star", "module")

load("./gn_logs.star", "gn_logs")
load("./platform.star", "platform")

def __filegroups(ctx):
    logs = gn_logs.read(ctx)
    target_sys = logs.get("yocto_target_sys")
    target_cc = logs.get("yocto_target_cc")
    target_cxx = logs.get("yocto_target_cxx")
    fg = {
        "out/recipe-sysroot:headers": {
            "type": "glob",
            "includes": [
                "usr/include/**",
                "usr/include/**/**",
                "usr/include/**/**/**",
                "usr/include/**/**/**/**",
                "usr/include/**/**/**/**/**",
                "usr/lib/**/include/**",
                "usr/lib/**/include/**/**",
                "usr/lib/%s/**/**" % target_sys,
                "usr/lib/gcc/%s/**" % target_sys,
                "usr/lib/gcc/%s/**/**" % target_sys,
                "usr/lib/gcc/%s/**/**/**" % target_sys,
            ],
        },
        "out/recipe-sysroot-native:headers": {
            "type": "glob",
            "includes": [
                "usr/include/**",
                "usr/include/**/**",
                "usr/include/**/**/**",
                "usr/include/**/**/**/**",
                "usr/lib/**/gcc/**/**/include-fixed/**",
                "usr/lib/**/gcc/**/**/include/**",
                "usr/lib/**/include/**",
                "usr/lib/**/include/**/**",
                "usr/lib/clang/**/include/**",
                "usr/lib/clang/**/include/**/**",
            ],
        },
        "out/recipe-sysroot-native:tools": {
            "type": "glob",
            "includes": [
               "usr/bin/%s/%s" % (target_sys, target_cc),
                "usr/bin/%s/%s" % (target_sys, target_cxx),
                "usr/bin/**/*-as",
                "usr/bin/**/*-g++",
                "usr/bin/**/*-gcc",
                "usr/bin/clang",
                "usr/bin/clang++",
                "usr/bin/clang-[0-9]*",
                "usr/bin/llvm-as",
                "usr/lib/libLLVM.so.**",
                "usr/lib/libclang-cpp.so**",
                "usr/lib/libedit.so.**",
                "usr/lib/libtinfo.so.**",
                "usr/libexec/**/gcc/**/**/as",
                "usr/libexec/**/gcc/**/**/cc1",
                "usr/libexec/**/gcc/**/**/cc1plus",
            ],
        },
    }
    return fg

def __yocto_compiler(ctx, cmd):
    logs = gn_logs.read(ctx)
    target_sys = logs.get("yocto_target_sys")
    target_cc = logs.get("yocto_target_cc")
    target_cxx = logs.get("yocto_target_cxx")
    compile_command = []
    for arg in list(cmd.args):
        if arg.startswith("--sysroot="):
            sysroot = arg.removeprefix("--sysroot=")
            if path.isabs(sysroot):
                # exec_root : out/Release
                # sysroot   : out/recipe-sysroot
                sysroot = path.rel(cmd.exec_root, sysroot)
            else:
                sysroot = ctx.fs.canonpath(sysroot)
            compile_command.append("--sysroot=../../" + sysroot)
        elif arg in [target_cc, target_cxx]:
            compile_command.extend([
                "../recipe-sysroot-native/usr/bin/%s/%s" % (target_sys, arg)
            ])
        else:
            compile_command.append(arg)

    ctx.actions.fix(args = compile_command)

# to prevent siso from adding host compiler path to sysroot
def __yocto_host_compiler(ctx, cmd):
    compile_command = []
    for arg in list(cmd.args):
        if arg == "g++" or arg == "gcc":
            compile_command.append('cxx' if arg == 'g++' else 'cc')
        elif arg.startswith("--sysroot="):
            #ignore sysroot for host compiler
            continue
        else:
            compile_command.append(arg)

    compile_command = ["../../build/neva/host.sh"] + compile_command
    ctx.actions.fix(args = compile_command)

# Ignore sysroot if the path is "/"
def __clang_host_compiler(ctx, cmd):
    compile_command = []
    for arg in list(cmd.args):
        if arg.startswith("--sysroot="):
            sysroot = arg.removeprefix("--sysroot=")
            sysroot = path.join(cmd.exec_root, sysroot)
            sysroot = ctx.fs.canonpath(sysroot)
            if sysroot == "/":
              continue
        compile_command.append(arg)
    ctx.actions.fix(args = compile_command)

__handlers = {
    "yocto_compiler": __yocto_compiler,
    "yocto_host_compiler": __yocto_host_compiler,
    "clang_host_compiler": __clang_host_compiler,
}

def __rules(ctx):
    logs = gn_logs.read(ctx)
    target_sys = logs.get("yocto_target_sys")
    target_cc = logs.get("yocto_target_cc")
    target_cxx = logs.get("yocto_target_cxx")

    input_root_absolute_path = False
    canonicalize_dir = not input_root_absolute_path

    rules = []

    rules.extend([
        {
            "name": "yocto_host/cxx",
            "action": "(.*_)?host_cxx",
            "command_prefix": "g++ ",
            "inputs": [
                "out/recipe-sysroot:headers",
                "out/recipe-sysroot-native:tools",
                "build/neva/host.sh",
            ],
            "handler": "yocto_host_compiler",
            "exclude_input_patterns": ["*.stamp"],
            "remote": True,
            "input_root_absolute_path": input_root_absolute_path,
            "use_system_input": True,
            "canonicalize_dir": canonicalize_dir,
            "timeout": "2m",
        },
        {
            "name": "yocto_host/cc",
            "action": "(.*_)?host_cc",
            "command_prefix": "gcc ",
            "inputs": [
                "out/recipe-sysroot:headers",
                "out/recipe-sysroot-native:tools",
                "build/neva/host.sh",
            ],
            "handler": "yocto_host_compiler",
            "exclude_input_patterns": ["*.stamp"],
            "remote": True,
            "input_root_absolute_path": input_root_absolute_path,
            "use_system_input": True,
            "canonicalize_dir": canonicalize_dir,
            "timeout": "2m",
        },
        {
            "name": "yocto/cxx",
            "action": "(.*_)?cxx",
            "command_prefix": "%s " % target_cxx,
            "inputs": [
                "out/recipe-sysroot:headers",
                "out/recipe-sysroot-native:tools",
                "out/recipe-sysroot-native:headers",
            ],
            "handler": "yocto_compiler",
            "exclude_input_patterns": ["*.stamp"],
            "remote": True,
            "input_root_absolute_path": input_root_absolute_path,
            "use_system_input": True,
            "canonicalize_dir": canonicalize_dir,
            "timeout": "2m",
        },
        {
            "name": "yocto/cc",
            "action": "(.*_)?cc",
            "command_prefix": "%s " % target_cc,
            "inputs": [
                "out/recipe-sysroot:headers",
                "out/recipe-sysroot-native:tools",
                "out/recipe-sysroot-native:headers",
            ],
            "handler": "yocto_compiler",
            "exclude_input_patterns": ["*.stamp"],
            "remote": True,
            "input_root_absolute_path": input_root_absolute_path,
            "use_system_input": True,
            "canonicalize_dir": canonicalize_dir,
            "timeout": "2m",
        },
    ])
    return rules

def __step_config(ctx, step_config):
    new_rules = []
    for rule in step_config["rules"]:
        arg0 = rule.get("command_prefix", "").split(" ")[0].strip("\"")
        if arg0 == platform.python_bin:
            r = {}
            r.update(rule)
            r["remote_command"] = "/usr/bin/%s" % arg0
            new_rules.append(r)
        elif rule["name"] in ["clang/cxx", "clang/cc", "clang/asm" ]:
            r = {}
            r.update(rule)
            r["use_system_input"] = True
            r["handler"] = "clang_host_compiler"
            new_rules.append(r)
        else:
            new_rules.append(rule)
    new_rules.extend(__rules(ctx))
    step_config["rules"] = new_rules
    return step_config

yocto = module(
    "yocto",
    filegroups = __filegroups,
    handlers = __handlers,
    step_config = __step_config,
)
