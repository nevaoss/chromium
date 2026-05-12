// Copyright 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_NEVA_VIEWS_DELEGATE_STUB_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_NEVA_VIEWS_DELEGATE_STUB_H_

#include "ui/views/views_delegate.h"

namespace views {

class ViewsDelegateStub : public ViewsDelegate {
 public:
  ViewsDelegateStub() = default;
  ViewsDelegateStub(const ViewsDelegateStub&) = delete;
  ViewsDelegateStub& operator=(const ViewsDelegateStub&) = delete;
  ~ViewsDelegateStub() override = default;
};

}  // namespace views

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_NEVA_VIEWS_DELEGATE_STUB_H_