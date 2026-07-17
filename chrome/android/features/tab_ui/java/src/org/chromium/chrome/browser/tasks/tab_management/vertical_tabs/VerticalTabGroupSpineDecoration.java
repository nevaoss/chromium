// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management.vertical_tabs;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Handler;
import android.os.Looper;
import android.view.View;

import androidx.recyclerview.widget.RecyclerView;

import org.chromium.base.Callback;
import org.chromium.base.Token;
import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;
import org.chromium.chrome.browser.tabmodel.TabGroupObserver;
import org.chromium.chrome.browser.tabmodel.TabModel;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.chrome.browser.tasks.tab_management.TabListItemAnimator;
import org.chromium.chrome.browser.tasks.tab_management.TabListModel;
import org.chromium.chrome.browser.tasks.tab_management.TabProperties;
import org.chromium.chrome.tab_ui.R;
import org.chromium.components.tab_groups.TabGroupColorId;
import org.chromium.components.tab_groups.TabGroupColorPickerUtils;
import org.chromium.ui.base.LocalizationUtils;
import org.chromium.ui.modelutil.PropertyModel;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/** Draws a vertical line to the side of tab items that belong to a tab group. */
@NullMarked
class VerticalTabGroupSpineDecoration extends RecyclerView.ItemDecoration {
    // TabListItemAnimator sequences removals before moves, so the spine's collapse duration
    // must account for both.
    private static final long COLLAPSE_DURATION_MS =
            TabListItemAnimator.DEFAULT_REMOVE_DURATION + TabListItemAnimator.DEFAULT_MOVE_DURATION;

    private final TabListModel mModel;
    private final TabModelSelector mTabModelSelector;
    private final Runnable mInvalidationTrigger;
    private final Set<Token> mDrawnGroupIds = new HashSet<>();
    private final Set<Token> mCollapsingGroupIds = new HashSet<>();
    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private final Callback<TabModel> mCurrentTabModelObserver;
    private final Paint mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final RectF mRectF = new RectF();
    private final int mSpineRadius;
    private final float mSpineWidth;
    private final int mMarginBottom;
    private final List<View> mSortedChildren = new ArrayList<>();
    private final ChildPositionComparator mChildPositionComparator = new ChildPositionComparator();

    private @Nullable TabModel mCurrentTabModel;

    private final TabGroupObserver mTabGroupObserver =
            new TabGroupObserver() {
                @Override
                public void didChangeTabGroupColor(
                        Token tabGroupId, @TabGroupColorId int newColor) {
                    mInvalidationTrigger.run();
                }

                @Override
                public void didChangeTabGroupCollapsed(
                        Token tabGroupId, boolean isCollapsed, boolean animate) {
                    if (isCollapsed) {
                        mCollapsingGroupIds.add(tabGroupId);
                        mHandler.postDelayed(
                                () -> {
                                    mCollapsingGroupIds.remove(tabGroupId);
                                    mInvalidationTrigger.run();
                                },
                                COLLAPSE_DURATION_MS);
                    }
                }
            };

    /**
     * Constructor for {@link VerticalTabGroupSpineDecoration}.
     *
     * @param context The {@link Context} used to retrieve dimension resources.
     * @param invalidationTrigger A {@link Runnable} triggered to invalidate the recycler view.
     * @param model The {@link TabListModel} containing tab properties.
     * @param tabModelSelector The {@link TabModelSelector} used to fetch tab models.
     */
    public VerticalTabGroupSpineDecoration(
            Context context,
            Runnable invalidationTrigger,
            TabListModel model,
            TabModelSelector tabModelSelector) {
        mInvalidationTrigger = invalidationTrigger;
        mModel = model;
        mTabModelSelector = tabModelSelector;
        var res = context.getResources();
        mSpineWidth = res.getDimensionPixelSize(R.dimen.vertical_tab_spine_width);
        mSpineRadius = res.getDimensionPixelSize(R.dimen.vertical_tab_spine_radius);
        mMarginBottom = res.getDimensionPixelSize(R.dimen.vertical_tab_item_margin_bottom);

        mCurrentTabModelObserver = this::onCurrentTabModelChanged;
        mTabModelSelector
                .getCurrentTabModelSupplier()
                .addSyncObserverAndCallIfNonNull(mCurrentTabModelObserver);
    }

    @Override
    public void onDraw(Canvas c, RecyclerView parent, RecyclerView.State state) {
        TabModel tabModel = mTabModelSelector.getCurrentModel();
        if (tabModel == null || mModel.isEmpty()) return;

        boolean isIncognito = tabModel.isIncognitoBranded();
        boolean isRtl = LocalizationUtils.isLayoutRtl();
        float left;
        float right;
        if (isRtl) {
            right = parent.getWidth() - parent.getPaddingRight();
            left = right - mSpineWidth;
        } else {
            left = parent.getPaddingLeft();
            right = left + mSpineWidth;
        }

        mDrawnGroupIds.clear();
        collectAndSortChildren(parent);

        int sortedChildCount = mSortedChildren.size();
        for (int i = 0; i < sortedChildCount; i++) {
            View child = mSortedChildren.get(i);
            int pos = parent.getChildAdapterPosition(child);

            PropertyModel childModel = mModel.get(pos).model;
            Token headerId = childModel.get(TabProperties.TAB_GROUP_HEADER_ID);
            boolean isHeader = headerId != null;
            Token groupId = isHeader ? headerId : childModel.get(TabProperties.TAB_GROUP_ID);
            if (groupId == null || !mDrawnGroupIds.add(groupId)) {
                continue;
            }

            float top = calculateTop(child, isHeader);
            boolean isCollapsed = isHeader && childModel.get(TabProperties.IS_COLLAPSED);
            float bottom = calculateBottom(parent, groupId, sortedChildCount, i, isCollapsed);
            if (bottom <= top) continue;

            @Nullable Integer cardColorId = childModel.get(TabProperties.TAB_GROUP_CARD_COLOR);
            int colorId =
                    cardColorId != null
                            ? cardColorId
                            : tabModel.getTabGroupColorWithFallback(groupId);
            int color =
                    TabGroupColorPickerUtils.getTabGroupColorPickerItemColor(
                            parent.getContext(), colorId, isIncognito);
            mPaint.setColor(color);

            mRectF.set(left, top, right, bottom);
            c.drawRoundRect(mRectF, mSpineRadius, mSpineRadius, mPaint);
        }
    }

    public void destroy() {
        mHandler.removeCallbacksAndMessages(null);
        mTabModelSelector.getCurrentTabModelSupplier().removeObserver(mCurrentTabModelObserver);
        if (mCurrentTabModel != null) {
            mCurrentTabModel.removeTabGroupObserver(mTabGroupObserver);
            mCurrentTabModel = null;
        }
    }

    private void onCurrentTabModelChanged(@Nullable TabModel tabModel) {
        if (mCurrentTabModel != null) {
            mCurrentTabModel.removeTabGroupObserver(mTabGroupObserver);
        }
        mCurrentTabModel = tabModel;
        if (mCurrentTabModel != null) {
            mCurrentTabModel.addTabGroupObserver(mTabGroupObserver);
        }
    }

    private void collectAndSortChildren(RecyclerView parent) {
        mSortedChildren.clear();
        int childCount = parent.getChildCount();
        for (int i = 0; i < childCount; i++) {
            View child = parent.getChildAt(i);
            int pos = parent.getChildAdapterPosition(child);
            if (pos != RecyclerView.NO_POSITION && pos < mModel.size()) {
                mSortedChildren.add(child);
            }
        }

        // RecyclerView heavily recycles and reorders views during fast scrolling and
        // group expand/collapse animations. Because of this, `parent.getChildAt(i)`
        // often returns views out of their logical adapter order (e.g. newly bound
        // child tabs are appended to the end of the ViewGroup). We must sort the
        // attached views by their adapter position to guarantee we can accurately
        // identify the "first" and "last" visible tabs for each group and properly
        // connect the spine to subsequent items.
        mChildPositionComparator.setParent(parent);
        mSortedChildren.sort(mChildPositionComparator);
        mChildPositionComparator.setParent(null);
    }

    private float calculateTop(View child, boolean isHeader) {
        if (!isHeader) {
            // For a normal child tab (meaning the group header scrolled off-screen),
            // start the vertical line directly from the top edge of this card.
            return child.getTop() + child.getTranslationY();
        }

        // For a group header, start the line right below the header card (plus trailing margin).
        return child.getBottom() + child.getTranslationY() + mMarginBottom;
    }

    private float calculateBottom(
            RecyclerView parent,
            Token groupId,
            int sortedChildCount,
            int childIndex,
            boolean isCollapsed) {
        View child = mSortedChildren.get(childIndex);
        View lastGroupView = child;
        View lastStableGroupView = child;
        View nextSiblingView = null;

        // Scan subsequent visible items in the sorted array to find:
        // 1. lastGroupView: The last visible child tab belonging to this group.
        // 2. lastStableGroupView: The last fully visible tab, for expanding/adding tab animation if
        // the tab group is the last one on the screen
        // 3. nextSiblingView: The first visible tab belonging to a DIFFERENT group.
        for (int j = childIndex + 1; j < sortedChildCount; j++) {
            View v = mSortedChildren.get(j);
            int pos = parent.getChildAdapterPosition(v);

            PropertyModel nextModel = mModel.get(pos).model;
            @Nullable Token headerId = nextModel.get(TabProperties.TAB_GROUP_HEADER_ID);
            Token nextGroupId =
                    headerId != null ? headerId : nextModel.get(TabProperties.TAB_GROUP_ID);
            if (groupId.equals(nextGroupId)) {
                if (v.getAlpha() >= 1f) {
                    lastStableGroupView = v;
                }
                lastGroupView = v;
            } else {
                nextSiblingView = v;
                break;
            }
        }

        // Case 1: Fully collapsed or actively collapsing group.
        if (isCollapsed) {
            // If this group is in mCollapsingGroupIds, it is actively collapsing. Connect its spine
            // bottom to the next group.
            if (mCollapsingGroupIds.contains(groupId) && nextSiblingView != null) {
                return nextSiblingView.getTop() + nextSiblingView.getTranslationY() - mMarginBottom;
            }
            return child.getBottom() + child.getTranslationY();
        }

        // Case 2: Middle group with items below it.
        // Always connect the spine bottom to the top of the next group.
        if (nextSiblingView != null) {
            return nextSiblingView.getTop() + nextSiblingView.getTranslationY() - mMarginBottom;
        }

        // Case 3: The last group shown on the list.
        float targetBottom = lastGroupView.getBottom() + lastGroupView.getTranslationY();

        int lastPos = parent.getChildAdapterPosition(lastGroupView);
        boolean hasSubsequentItems = lastPos < mModel.size() - 1;
        if (hasSubsequentItems) {
            // If there are more items in the adapter that are pushed off-screen, the spine should
            // seamlessly continue to the bottom without shrinking.
            return targetBottom;
        }

        float startBottom = lastStableGroupView.getBottom() + lastStableGroupView.getTranslationY();
        float t = lastGroupView.getAlpha();
        return startBottom + ((targetBottom - startBottom) * t);
    }

    private static class ChildPositionComparator implements Comparator<View> {
        private @Nullable RecyclerView mParent;

        public void setParent(@Nullable RecyclerView parent) {
            mParent = parent;
        }

        @Override
        public int compare(View v1, View v2) {
            assert mParent != null;
            return Integer.compare(
                    mParent.getChildAdapterPosition(v1), mParent.getChildAdapterPosition(v2));
        }
    }
}
