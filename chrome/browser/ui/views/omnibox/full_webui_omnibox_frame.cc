// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/omnibox/full_webui_omnibox_frame.h"

#include <memory>

#include "chrome/browser/ui/views/omnibox/rounded_omnibox_results_frame.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/geometry/rounded_corners_f.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/view.h"

FullWebUIOmniboxFrame::FullWebUIOmniboxFrame(views::View* contents,
                                             LocationBar* location_bar,
                                             bool forward_mouse_events)
    : RoundedOmniboxResultsFrame(contents, location_bar, forward_mouse_events) {
}

FullWebUIOmniboxFrame::~FullWebUIOmniboxFrame() = default;

void FullWebUIOmniboxFrame::SetElevation(int elevation) {
  const int corner_radius = views::LayoutProvider::Get()->GetCornerRadiusMetric(
      views::ShapeContextTokens::kOmniboxExpandedRadius);
  auto border = std::make_unique<views::BubbleBorder>(
      views::BubbleBorder::Arrow::NONE,
      elevation == 0 ? views::BubbleBorder::Shadow::NO_SHADOW
                     : views::BubbleBorder::Shadow::STANDARD_SHADOW);
  border->set_rounded_corners(gfx::RoundedCornersF(corner_radius));
  if (elevation > 0) {
    border->set_md_shadow_elevation(elevation);
  }
  SetBorder(std::move(border));
}

BEGIN_METADATA(FullWebUIOmniboxFrame)
END_METADATA
