// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.ui.util;

import static org.junit.Assert.assertEquals;

import android.view.View;
import android.view.View.OnLayoutChangeListener;

import androidx.test.core.app.ApplicationProvider;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.BaseRobolectricTestRunner;

import java.util.concurrent.atomic.AtomicInteger;

/** Unit tests for {@link BoundsChangedLayoutListener}. */
@RunWith(BaseRobolectricTestRunner.class)
public class BoundsChangedLayoutListenerTest {
    private View mView;

    @Before
    public void setUp() {
        mView = new View(ApplicationProvider.getApplicationContext());
    }

    @Test
    public void testLayoutBoundsChanged() {
        AtomicInteger callCount = new AtomicInteger(0);
        OnLayoutChangeListener listener =
                BoundsChangedLayoutListener.create(callCount::incrementAndGet);

        // Run when bounds changed:
        listener.onLayoutChange(mView, 0, 0, 100, 100, 0, 0, 50, 50);
        assertEquals(1, callCount.get());

        // Run again when bounds are different:
        listener.onLayoutChange(mView, 0, 0, 200, 100, 0, 0, 100, 100);
        assertEquals(2, callCount.get());
    }

    @Test
    public void testLayoutBoundsUnchanged() {
        AtomicInteger callCount = new AtomicInteger(0);
        OnLayoutChangeListener listener =
                BoundsChangedLayoutListener.create(callCount::incrementAndGet);

        // Does not run when bounds are exactly the same:
        listener.onLayoutChange(mView, 0, 0, 100, 100, 0, 0, 100, 100);
        assertEquals(0, callCount.get());

        // Run when bounds changed:
        listener.onLayoutChange(mView, 0, 0, 100, 100, 0, 0, 50, 50);
        assertEquals(1, callCount.get());

        // Does not run when bounds match the new coordinates:
        listener.onLayoutChange(mView, 0, 0, 100, 100, 0, 0, 100, 100);
        assertEquals(1, callCount.get());
    }

    @Test
    public void testOnBoundsChangedCallback() {
        AtomicInteger callCount = new AtomicInteger(0);
        int[] capturedBounds = new int[4];
        final View[] capturedView = new View[1];

        OnLayoutChangeListener listener =
                BoundsChangedLayoutListener.create(
                        (v, left, top, right, bottom) -> {
                            callCount.incrementAndGet();
                            capturedView[0] = v;
                            capturedBounds[0] = left;
                            capturedBounds[1] = top;
                            capturedBounds[2] = right;
                            capturedBounds[3] = bottom;
                        });

        // Does not run when bounds are exactly the same:
        listener.onLayoutChange(mView, 0, 0, 100, 100, 0, 0, 100, 100);
        assertEquals(0, callCount.get());

        // Run when bounds changed:
        listener.onLayoutChange(mView, 10, 20, 110, 120, 0, 0, 50, 50);
        assertEquals(1, callCount.get());
        assertEquals(mView, capturedView[0]);
        assertEquals(10, capturedBounds[0]);
        assertEquals(20, capturedBounds[1]);
        assertEquals(110, capturedBounds[2]);
        assertEquals(120, capturedBounds[3]);
    }
}
