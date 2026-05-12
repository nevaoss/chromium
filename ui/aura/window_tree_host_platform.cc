// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/window_tree_host_platform.h"

#include <memory>
#include <optional>
#include <utility>

#include "base/check_is_test.h"
#include "base/functional/bind.h"
#include "base/observer_list.h"
#include "base/run_loop.h"
#include "base/trace_event/trace_event.h"
#include "build/build_config.h"
#include "ui/aura/client/cursor_client.h"
#include "ui/aura/env.h"
#include "ui/aura/host_frame_rate_throttler.h"
#include "ui/aura/native_window_occlusion_tracker.h"
#include "ui/aura/window.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/aura/window_tree_host_observer.h"
#include "ui/base/cursor/mojom/cursor_type.mojom-shared.h"
#include "ui/base/layout.h"
#include "ui/base/view_prop.h"
#include "ui/compositor/compositor.h"
#include "ui/compositor/layer.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/events/event.h"
#include "ui/events/keyboard_hook.h"
#include "ui/events/keycodes/dom/dom_code.h"
#include "ui/platform_window/platform_window.h"
#include "ui/platform_window/platform_window_init_properties.h"

#if BUILDFLAG(IS_OZONE)
#include "ui/events/keycodes/dom/dom_keyboard_layout_map.h"
#include "ui/events/ozone/events_ozone.h"
#include "ui/ozone/public/ozone_platform.h"
#endif

#if BUILDFLAG(IS_WIN)
#include "ui/platform_window/win/win_window.h"
#endif

///@name USE_NEVA_APPRUNTIME
///@{
#include "base/command_line.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/neva/input_method_common.h"
#include "ui/base/ui_base_neva_switches.h"
#include "ui/platform_window/neva/ui_utils.h"
///@}

namespace aura {

namespace {
WindowTreeHostPlatform::PlatformWindowFactoryDelegateForTesting*
    g_platform_window_factory_delegate_for_testing = nullptr;

const char kWindowTreeHostPlatformForAcceleratedWidget[] =
    "__AURA_WINDOW_TREE_HOST_PLATFORM_ACCELERATED_WIDGET__";
}

// static
std::unique_ptr<WindowTreeHost> WindowTreeHost::Create(
    ui::PlatformWindowInitProperties properties) {
  return std::make_unique<WindowTreeHostPlatform>(
      std::move(properties),
      std::make_unique<aura::Window>(nullptr, client::WINDOW_TYPE_UNKNOWN));
}

WindowTreeHostPlatform::WindowTreeHostPlatform(
    ui::PlatformWindowInitProperties properties,
    std::unique_ptr<Window> window)
    ///@name USE_NEVA_APPRUNTIME
    ///@{
    //: WindowTreeHost(std::move(window)),
    : neva::WindowTreeHostPlatform(std::move(window), this),
    ///@}
      widget_(gfx::kNullAcceleratedWidget),
      current_cursor_(ui::mojom::CursorType::kNull) {
  size_in_pixels_ = properties.bounds.size();
  CreateCompositor(false, false, properties.enable_compositing_based_throttling,
                   properties.compositor_memory_limit_mb);
  CreateAndSetPlatformWindow(std::move(properties));
}

WindowTreeHostPlatform::WindowTreeHostPlatform(std::unique_ptr<Window> window)
    ///@name USE_NEVA_APPRUNTIME
    ///@{
    //: WindowTreeHost(std::move(window)),
    : neva::WindowTreeHostPlatform(std::move(window), this),
    ///@}
      widget_(gfx::kNullAcceleratedWidget),
      current_cursor_(ui::mojom::CursorType::kNull) {}

// static
WindowTreeHostPlatform* WindowTreeHostPlatform::GetHostForWindow(
    aura::Window* window) {
  return reinterpret_cast<WindowTreeHostPlatform*>(
      ui::ViewProp::GetValue(window->GetHost()->GetAcceleratedWidget(),
                             kWindowTreeHostPlatformForAcceleratedWidget));
}

void WindowTreeHostPlatform::CreateAndSetPlatformWindow(
    ui::PlatformWindowInitProperties properties) {
  // Cache initial size used to create |platform_window_| so that it does not
  // end up propagating unneeded bounds change event when it is first notified
  // through OnBoundsChanged, which may lead to unneeded re-layouts, etc.
  size_in_pixels_ = properties.bounds.size();
  platform_window_ = CreatePlatformWindow(std::move(properties));
#if BUILDFLAG(IS_WEBOS)
  platform_window_->SetContentsSize(size_in_pixels_);
#endif  // BUILDFLAG(IS_WEBOS)
  ///@name USE_NEVA_APPRUNTIME
  ///@{
  bool ime_enabled = base::CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kEnableNevaIme);
  if (ime_enabled)
    GetInputMethod()->AddObserver(this);
  SetImeEnabled(ime_enabled);
  ///@}
}

void WindowTreeHostPlatform::SetPlatformWindow(
    std::unique_ptr<ui::PlatformWindow> window) {
  platform_window_ = std::move(window);
}

WindowTreeHostPlatform::~WindowTreeHostPlatform() {
  ///@name USE_NEVA_APPRUNTIME
  ///@{
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableNevaIme))
    GetInputMethod()->RemoveObserver(this);
  ///@}
  DestroyCompositor();
  DestroyDispatcher();

  // |platform_window_| may not exist yet.
  if (platform_window_)
    platform_window_->Close();
}

ui::EventSource* WindowTreeHostPlatform::GetEventSource() {
  return this;
}

gfx::AcceleratedWidget WindowTreeHostPlatform::GetAcceleratedWidget() {
  return widget_;
}

void WindowTreeHostPlatform::ShowImpl() {
  platform_window_->Show();
}

void WindowTreeHostPlatform::HideImpl() {
  platform_window_->Hide();
}

gfx::Rect WindowTreeHostPlatform::GetBoundsInPixels() const {
  return platform_window_->GetBoundsInPixels();
}

void WindowTreeHostPlatform::SetBoundsInPixels(const gfx::Rect& bounds) {
  platform_window_->SetBoundsInPixels(bounds);
}

void WindowTreeHostPlatform::SetCapture() {
#if BUILDFLAG(IS_OZONE)
  if (ui::IsNativeUiEventDispatchDisabled()) {
    return;
  }
#endif
  platform_window_->SetCapture();
}

void WindowTreeHostPlatform::ReleaseCapture() {
  platform_window_->ReleaseCapture();
}

gfx::Point WindowTreeHostPlatform::GetLocationOnScreenInPixels() const {
  return platform_window_->GetBoundsInPixels().origin();
}

bool WindowTreeHostPlatform::CaptureSystemKeyEventsImpl(
    std::optional<base::flat_set<ui::DomCode>> dom_codes) {
  // Only one KeyboardHook should be active at a time, otherwise there will be
  // problems with event routing (i.e. which Hook takes precedence) and
  // destruction ordering.
  DCHECK(!keyboard_hook_);
  keyboard_hook_ = ui::KeyboardHook::CreateModifierKeyboardHook(
      std::move(dom_codes), GetAcceleratedWidget(),
      base::BindRepeating(
          [](ui::PlatformWindowDelegate* delegate, ui::KeyEvent* event) {
            delegate->DispatchEvent(event);
          },
          base::Unretained(this)));

  return keyboard_hook_ != nullptr;
}

void WindowTreeHostPlatform::ReleaseSystemKeyEventCapture() {
  keyboard_hook_.reset();
}

bool WindowTreeHostPlatform::IsKeyLocked(ui::DomCode dom_code) {
  return keyboard_hook_ && keyboard_hook_->IsKeyLocked(dom_code);
}

base::flat_map<std::string, std::string>
WindowTreeHostPlatform::GetKeyboardLayoutMap() {
#if BUILDFLAG(IS_OZONE)
  return ui::GenerateDomKeyboardLayoutMap();
#else
  NOTIMPLEMENTED();
  return {};
#endif
}

void WindowTreeHostPlatform::OnVideoCaptureLockCreated() {
  if (platform_window_) {
    platform_window_->SetVideoCapture();
  }
}

void WindowTreeHostPlatform::OnVideoCaptureLockDestroyed() {
  if (platform_window_) {
    platform_window_->ReleaseVideoCapture();
  }
}

void WindowTreeHostPlatform::SetCursorNative(gfx::NativeCursor cursor) {
  if (cursor == current_cursor_)
    return;
  current_cursor_ = cursor;

#if BUILDFLAG(IS_WEBOS)
  // Pointer cursor is considered as default system cursor.
  // So, for pointer cursor, method SetCustomCursor with kNotUse argument
  // is called instead of SetCursor to substitute default pointer cursor
  // (black arrow) to default LSM cursor (pink plectrum).
  ui::mojom::CursorType native_type = cursor.type();
  if (native_type == ui::mojom::CursorType::kPointer) {
    platform_window_->SetCustomCursor(neva_app_runtime::CustomCursorType::kNotUse,
                                      "", 0, 0, false);
    return;
  }
#endif  // BUILDFLAG(IS_WEBOS)

  platform_window_->SetCursor(cursor.platform());
}

void WindowTreeHostPlatform::MoveCursorToScreenLocationInPixels(
    const gfx::Point& location_in_pixels) {
#if BUILDFLAG(IS_OZONE)
  if (ui::IsNativeUiEventDispatchDisabled()) {
    // Unit tests should not test or rely on the native cursor position because
    // it is shared between multiple tests.
    return;
  }
#endif
  platform_window_->MoveCursorTo(location_in_pixels);
}

void WindowTreeHostPlatform::OnCursorVisibilityChangedNative(bool show) {
  NOTIMPLEMENTED_LOG_ONCE();
}

///@name USE_NEVA_APPRUNTIME
///@{
void WindowTreeHostPlatform::SetCustomCursor(
    neva_app_runtime::CustomCursorType type,
    const std::string& path,
    int hotspot_x,
    int hotspot_y) {
  platform_window_->SetCustomCursor(type, path, hotspot_x, hotspot_y, true);
}

void WindowTreeHostPlatform::SetCursorVisibility(bool visible) {
  platform_window_->SetCursorVisibility(visible);
}

void WindowTreeHostPlatform::SetGroupKeyMask(ui::KeyMask key_mask) {
  platform_window_->SetGroupKeyMask(key_mask);
}

void WindowTreeHostPlatform::SetKeyMask(ui::KeyMask key_mask, bool set) {
  platform_window_->SetKeyMask(key_mask, set);
}

void WindowTreeHostPlatform::SetInputRegion(
    const std::vector<gfx::Rect>& region) {
  platform_window_->SetInputRegion(region);
}

void WindowTreeHostPlatform::SetUseVirtualKeyboard(bool enable) {
  SetImeEnabled(enable);
  if (!enable)
    OnHideIme();
}

void WindowTreeHostPlatform::SetWindowProperty(const std::string& name,
                                               const std::string& value) {
  platform_window_->SetWindowProperty(name, value);
}

void WindowTreeHostPlatform::SetFullscreen(bool fullscreen,
                                           int64_t target_display_id) {
  platform_window_->SetFullscreen(fullscreen, target_display_id);
}

void WindowTreeHostPlatform::CreateGroup(
    const ui::WindowGroupConfiguration& config) {
  platform_window_->CreateGroup(config);
}

void WindowTreeHostPlatform::AttachToGroup(const std::string& group_name,
                                           const std::string& layer_name) {
  platform_window_->AttachToGroup(group_name, layer_name);
}

void WindowTreeHostPlatform::FocusGroupOwner() {
  platform_window_->FocusGroupOwner();
}

void WindowTreeHostPlatform::FocusGroupLayer() {
  platform_window_->FocusGroupLayer();
}

void WindowTreeHostPlatform::DetachGroup() {
  platform_window_->DetachGroup();
}

void WindowTreeHostPlatform::XInputActivate(const std::string& type) {
  platform_window_->XInputActivate(type);
}

void WindowTreeHostPlatform::XInputDeactivate() {
  platform_window_->XInputDeactivate();
}

void WindowTreeHostPlatform::XInputInvokeAction(
    std::uint32_t keysym,
    ui::XInputKeySymbolType symbol_type,
    ui::XInputEventType event_type) {
  platform_window_->XInputInvokeAction(keysym, symbol_type, event_type);
}
///@}

void WindowTreeHostPlatform::LockMouse(Window* window) {
  window->SetCapture();
  WindowTreeHost::LockMouse(window);
}

std::unique_ptr<ui::PlatformWindow>
WindowTreeHostPlatform::CreatePlatformWindow(
    ui::PlatformWindowInitProperties properties) {
  if (g_platform_window_factory_delegate_for_testing) {
    return g_platform_window_factory_delegate_for_testing->Create(this);
  }
#if BUILDFLAG(IS_OZONE)
  return ui::OzonePlatform::GetInstance()->CreatePlatformWindow(
      this, std::move(properties));
#elif BUILDFLAG(IS_WIN)
  return std::make_unique<ui::WinWindow>(this, properties.bounds);
#else
  NOTIMPLEMENTED();
  return nullptr;
#endif
}

// static
void WindowTreeHostPlatform::SetPlatformWindowFactoryDelegateForTesting(
    PlatformWindowFactoryDelegateForTesting* delegate) {
  CHECK_IS_TEST();
  g_platform_window_factory_delegate_for_testing = delegate;
}

void WindowTreeHostPlatform::OnBoundsChanged(const BoundsChange& change) {
  // It's possible this function may be called recursively. Only notify
  // observers on initial entry. This way observers can safely assume that
  // OnHostDidProcessBoundsChange() is called when all bounds changes have
  // completed.
  if (++on_bounds_changed_recursion_depth_ == 1) {
    observers().Notify(&WindowTreeHostObserver::OnHostWillProcessBoundsChange,
                       this);
  }

  const auto preferred_scale =
      display::Screen::GetScreen()->GetPreferredScaleFactorForWindow(window());
  float current_scale = compositor()->device_scale_factor();
  float new_scale = preferred_scale.value_or(1.0f);
  auto weak_ref = GetWeakPtr();
  auto new_size = GetBoundsInPixels().size();
  bool size_changed = size_in_pixels_ != new_size;
  size_in_pixels_ = new_size;
  if (change.origin_changed) {
    OnHostMovedInPixels();
    // Changing the bounds may destroy this.
    if (!weak_ref)
      return;
  }
  if (size_changed || current_scale != new_scale) {
    OnHostResizedInPixels(new_size);
    // Changing the size may destroy this.
    if (!weak_ref)
      return;
  }
  DCHECK_GT(on_bounds_changed_recursion_depth_, 0);
  if (--on_bounds_changed_recursion_depth_ == 0) {
    observers().Notify(&WindowTreeHostObserver::OnHostDidProcessBoundsChange,
                       this);
  }
}

void WindowTreeHostPlatform::OnDamageRect(const gfx::Rect& damage_rect) {
  compositor()->ScheduleRedrawRect(damage_rect);
}

void WindowTreeHostPlatform::DispatchEvent(ui::Event* event) {
  TRACE_EVENT0("input", "WindowTreeHostPlatform::DispatchEvent");
  ui::EventDispatchDetails details = SendEventToSink(event);
  if (details.dispatcher_destroyed)
    event->SetHandled();
}

void WindowTreeHostPlatform::OnCloseRequest() {
  OnHostCloseRequested();
}

void WindowTreeHostPlatform::OnClosed() {}

void WindowTreeHostPlatform::OnWindowStateChanged(
    ui::PlatformWindowState old_state,
    ui::PlatformWindowState new_state) {
  ///@name USE_NEVA_APPRUNTIME
  ///@{
  WindowTreeHost::OnWindowHostStateChanged(ui::ToWidgetState(new_state));
  ///@}
}

///@name USE_NEVA_APPRUNTIME
///@{
void WindowTreeHostPlatform::OnWindowHostStateChanged(
    ui::WidgetState new_state) {
  WindowTreeHost::OnWindowHostStateChanged(new_state);
}

ui::LinuxInputMethodContext* WindowTreeHostPlatform::GetInputMethodContext() {
  return GetInputMethod()->GetInputMethodContext();
}

#if BUILDFLAG(IS_WEBOS)
void WindowTreeHostPlatform::OnInputPanelVisibilityChanged(bool visibility) {
  WindowTreeHost::OnInputPanelVisibilityChanged(visibility);
}

void WindowTreeHostPlatform::OnInputPanelRectChanged(int32_t x,
                                                     int32_t y,
                                                     uint32_t width,
                                                     uint32_t height) {
  WindowTreeHost::OnInputPanelRectChanged(x, y, width, height);
}
#endif  // BUILDFLAG(IS_WEBOS)

void WindowTreeHostPlatform::OnShowIme() {
#if defined(USE_OZONE)
  platform_window_->ShowInputPanel();
#endif
}

void WindowTreeHostPlatform::OnHideIme() {
#if defined(USE_OZONE)
  platform_window_->HideInputPanel();
#endif
}

void WindowTreeHostPlatform::OnTextInputInfoChanged(
    const ui::TextInputInfo& text_input_info) {
#if defined(USE_OZONE)
  if (text_input_info.type != ui::InputContentType::kNone)
    platform_window_->SetTextInputInfo(text_input_info);
#endif
}

void WindowTreeHostPlatform::SetSurroundingText(const std::string& text,
                                                size_t cursor_position,
                                                size_t anchor_position) {
#if defined(USE_OZONE)
  platform_window_->SetSurroundingText(text, cursor_position, anchor_position);
#endif
}
///@}

void WindowTreeHostPlatform::OnLostCapture() {
  OnHostLostWindowCapture();
}

void WindowTreeHostPlatform::OnAcceleratedWidgetAvailable(
    gfx::AcceleratedWidget widget) {
  prop_ = std::make_unique<ui::ViewProp>(
      widget, kWindowTreeHostPlatformForAcceleratedWidget, this);
  widget_ = widget;
  // This may be called before the Compositor has been created.
  if (compositor())
    WindowTreeHost::OnAcceleratedWidgetAvailable();
}

void WindowTreeHostPlatform::OnWillDestroyAcceleratedWidget() {}

void WindowTreeHostPlatform::OnAcceleratedWidgetDestroyed() {
  gfx::AcceleratedWidget widget = compositor()->ReleaseAcceleratedWidget();
  DCHECK_EQ(widget, widget_);
  widget_ = gfx::kNullAcceleratedWidget;
}

void WindowTreeHostPlatform::OnActivationChanged(bool active) {}

void WindowTreeHostPlatform::OnMouseEnter() {
  client::CursorClient* cursor_client = client::GetCursorClient(window());
  if (cursor_client) {
    auto display =
        display::Screen::GetScreen()->GetDisplayNearestWindow(window());
    DCHECK(display.is_valid());
    cursor_client->SetDisplay(display);
  }
}

void WindowTreeHostPlatform::OnOcclusionStateChanged(
    ui::PlatformWindowOcclusionState occlusion_state) {
  auto aura_occlusion_state = Window::OcclusionState::UNKNOWN;
  switch (occlusion_state) {
    case ui::PlatformWindowOcclusionState::kUnknown:
      aura_occlusion_state = Window::OcclusionState::UNKNOWN;
      break;
    case ui::PlatformWindowOcclusionState::kVisible:
      aura_occlusion_state = Window::OcclusionState::VISIBLE;
      break;
    case ui::PlatformWindowOcclusionState::kOccluded:
      aura_occlusion_state = Window::OcclusionState::OCCLUDED;
      break;
    case ui::PlatformWindowOcclusionState::kHidden:
      aura_occlusion_state = Window::OcclusionState::HIDDEN;
      break;
  }
  SetNativeWindowOcclusionState(aura_occlusion_state, {});
}

int64_t WindowTreeHostPlatform::OnStateUpdate(
    const PlatformWindowDelegate::State& old,
    const PlatformWindowDelegate::State& latest) {
  if (old.window_state != latest.window_state) {
    OnWindowStateChanged(old.window_state, latest.window_state);
  }

  if (old.bounds_dip != latest.bounds_dip || old.size_px != latest.size_px ||
      old.window_scale != latest.window_scale) {
    bool origin_changed = old.bounds_dip.origin() != latest.bounds_dip.origin();
    OnBoundsChanged({origin_changed});
  }

  if (old.raster_scale != latest.raster_scale) {
    compositor()->SetExternalPageScaleFactor(latest.raster_scale);
  }

  bool needs_frame = latest.WillProduceFrameOnUpdateFrom(old);
  if (old.occlusion_state != latest.occlusion_state &&
      NativeWindowOcclusionTracker::
          IsNativeWindowOcclusionTrackingAlwaysEnabled(this)) {
    const bool visible_before = compositor()->IsVisible();
    OnOcclusionStateChanged(latest.occlusion_state);
    if (!visible_before && compositor()->IsVisible()) {
      // If the compositor has become visible, make sure to wait for a frame.
      needs_frame = true;
    }
  }

  // Only set the sequence ID if this change will produce a frame.
  // If it won't, we may wait indefinitely for a frame that will never come.
  // If the compositor is not visible, we will not get a frame, so don't wait.
  if (!needs_frame || !compositor()->IsVisible()) {
    return -1;
  }

  // Update window()'s LocalSurfaceId. This will ensure that the parent ID is
  // updated both here and for LayerTreeHostImpl. So, the CompositorFrame sent
  // by LayerTreeHostImpl will include the updated parent ID for
  // synchronization. Some operations may have already updated the
  // LocalSurfaceId, but this only modifies pending commit state, so it's not
  // expensive.
  window()->AllocateLocalSurfaceId();
  compositor()->SetLocalSurfaceIdFromParent(window()->GetLocalSurfaceId());

  return window()->GetLocalSurfaceId().parent_sequence_number();
}

void WindowTreeHostPlatform::SetFrameRateThrottleEnabled(bool enabled) {
  if (enabled)
    HostFrameRateThrottler::GetInstance().AddHost(this);
  else
    HostFrameRateThrottler::GetInstance().RemoveHost(this);
}

void WindowTreeHostPlatform::DisableNativeWindowOcclusion() {
  SetNativeWindowOcclusionEnabled(false);
}

}  // namespace aura
