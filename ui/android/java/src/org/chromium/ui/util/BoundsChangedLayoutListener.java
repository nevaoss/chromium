// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.ui.util;

import android.view.View;
import android.view.View.OnLayoutChangeListener;

import androidx.annotation.Px;

import org.chromium.build.annotations.NullMarked;

/**
 * A utility class that wraps a {@link Runnable} or a {@link OnBoundsChanged} callback into a {@link
 * View.OnLayoutChangeListener} which only fires when the new layout bounds are different from the
 * previous layout bounds.
 */
@NullMarked
public class BoundsChangedLayoutListener implements OnLayoutChangeListener {
    /** A functional interface to be invoked when the layout bounds of a view change. */
    @FunctionalInterface
    public interface OnBoundsChanged {
        /**
         * Called when the layout bounds shift.
         *
         * @param v The view whose bounds changed.
         * @param left The new left boundary position.
         * @param top The new top boundary position.
         * @param right The new right boundary position.
         * @param bottom The new bottom boundary position.
         */
        void onBoundsChanged(View v, @Px int left, @Px int top, @Px int right, @Px int bottom);
    }

    private final OnBoundsChanged mOnBoundsChanged;

    private BoundsChangedLayoutListener(OnBoundsChanged callback) {
        mOnBoundsChanged = callback;
    }

    /**
     * Creates a {@link View.OnLayoutChangeListener} that executes the given {@link Runnable} when
     * the view's layout bounds change. Duplicate layout events with unchanged bounds are filtered.
     *
     * @param runnable The runnable to run when layout bounds change.
     */
    public static OnLayoutChangeListener create(Runnable runnable) {
        return new BoundsChangedLayoutListener((v, l, t, r, b) -> runnable.run());
    }

    /**
     * Creates a {@link View.OnLayoutChangeListener} that executes the given {@link OnBoundsChanged}
     * callback when the view's layout bounds change. Duplicate layout events with unchanged bounds
     * are filtered.
     *
     * @param callback The callback to execute when layout bounds change.
     */
    public static OnLayoutChangeListener create(OnBoundsChanged callback) {
        return new BoundsChangedLayoutListener(callback);
    }

    @Override
    public void onLayoutChange(
            View v,
            int left,
            int top,
            int right,
            int bottom,
            int oldLeft,
            int oldTop,
            int oldRight,
            int oldBottom) {
        if (left == oldLeft && top == oldTop && right == oldRight && bottom == oldBottom) {
            return;
        }
        mOnBoundsChanged.onBoundsChanged(v, left, top, right, bottom);
    }
}
