// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management.vertical_tabs;

import static org.chromium.build.NullUtil.assumeNonNull;

import android.content.Context;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;

import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabSelectionType;
import org.chromium.chrome.browser.tabmodel.TabModel;
import org.chromium.chrome.browser.tasks.tab_management.TabListItemTouchHelperCallback;
import org.chromium.chrome.browser.tasks.tab_management.TabListModel;
import org.chromium.chrome.browser.tasks.tab_management.TabProperties;
import org.chromium.chrome.tab_ui.R;
import org.chromium.ui.modelutil.SimpleRecyclerViewAdapter.ViewHolder;
import org.chromium.ui.recyclerview.widget.ItemTouchHelper2;

import java.util.function.Supplier;

/**
 * A {@link TabListItemTouchHelperCallback} implementation to host the logic for swipe and drag
 * related actions in vertical tab list layout.
 */
@NullMarked
public class VerticalTabListItemTouchHelperCallback extends TabListItemTouchHelperCallback {
    private final int mMouseDragThresholdSquared;

    /**
     * @param context The Android context.
     * @param model The {@link TabListModel} for the tab list.
     * @param currentTabModelSupplier Supplier for the current {@link TabModel}.
     */
    public VerticalTabListItemTouchHelperCallback(
            Context context, TabListModel model, Supplier<TabModel> currentTabModelSupplier) {
        super(context, model, currentTabModelSupplier);
        int touchSlop = ViewConfiguration.get(context).getScaledTouchSlop() / 4;
        mMouseDragThresholdSquared = touchSlop * touchSlop;
    }

    /**
     * Returns the movement flags for the given view holder. Regular and child tabs can move
     * vertically, while pinned tabs can move horizontally as well.
     */
    @Override
    public int getMovementFlags(RecyclerView recyclerView, RecyclerView.ViewHolder viewHolder) {
        if (!hasTabPropertiesModel(viewHolder)) return 0;

        // Regular and child tabs can move vertically.
        int dragFlags = ItemTouchHelper.UP | ItemTouchHelper.DOWN;
        // Pinned tabs can also move horizontally.
        if (isPinnedRegularTab(viewHolder)) {
            dragFlags |= ItemTouchHelper.LEFT | ItemTouchHelper.RIGHT;
        }
        return makeMovementFlags(dragFlags, 0);
    }

    /**
     * Checks whether a dragged tab can be dropped over a target tab. Prevents drops across pinned
     * and unpinned boundaries.
     */
    @Override
    public boolean canDropOver(
            RecyclerView recyclerView,
            RecyclerView.ViewHolder current,
            RecyclerView.ViewHolder target) {
        if (!hasTabPropertiesModel(target)) {
            return false;
        }

        // A pinned tab cannot be dropped in the unpinned section and vice versa.
        if (isPinnedRegularTab(current) != isPinnedRegularTab(target)) {
            return false;
        }
        return super.canDropOver(recyclerView, current, target);
    }

    /**
     * Called when a tab is moved. Updates the underlying {@link TabModel} to reflect the
     * reordering.
     */
    @Override
    public boolean onMove(
            RecyclerView recyclerView,
            RecyclerView.ViewHolder fromViewHolder,
            RecyclerView.ViewHolder toViewHolder) {
        if (!hasTabPropertiesModel(fromViewHolder) || !hasTabPropertiesModel(toViewHolder)) {
            return false;
        }

        int currentTabId = getTabId(fromViewHolder);
        int destinationTabId = getTabId(toViewHolder);

        TabModel tabModel = mCurrentTabModelSupplier.get();
        if (tabModel == null) return false;

        // Calculate destination index and restrict it based on pinning boundaries.
        int destinationIndex = tabModel.indexOf(tabModel.getTabById(destinationTabId));
        destinationIndex = adjustIndexBasedOnPinning(tabModel, currentTabId, destinationIndex);

        // Track the current UI position to correctly clean up visual selection on drop
        mSelectedTabIndex = toViewHolder.getBindingAdapterPosition();

        // Perform basic list reordering by updating the TabModel immediately.
        // - Standalone tabs & group headers use moveRelatedTabs() to fire didMoveTabGroup(),
        //   which TabListMediator observes to update top-level UI rows.
        // - Child tabs use moveTab() because they move within their group, firing
        //   didMoveWithinGroup() which TabListMediator observes.
        boolean isGroupHeader = fromViewHolder.getItemViewType() == TabProperties.UiType.TAB_GROUP;
        Tab currentTab = tabModel.getTabById(currentTabId);
        boolean isStandaloneTab = currentTab != null && currentTab.getTabGroupId() == null;

        if (isGroupHeader || isStandaloneTab) {
            // TODO(crbug.com/518307037): Needs to handle grouping when a standalone tab is dragged
            // into the index span
            // of an expanded group.
            tabModel.moveRelatedTabs(currentTabId, destinationIndex);
        } else {
            // TODO(crbug.com/518307037): Needs to handle ungrouping when a child tab is dragged
            // outside
            // its parent group's boundaries.
            tabModel.moveTab(currentTabId, destinationIndex);
        }
        return true;
    }

    /** Called when a tab is swiped. Swiping is not supported for vertical tabs. */
    @Override
    public void onSwiped(RecyclerView.ViewHolder viewHolder, int direction) {
        // Empty/default
    }

    /**
     * Returns whether long press to drag is enabled. Disabled for mouse input to allow instant
     * dragging.
     */
    @Override
    public boolean isLongPressDragEnabled() {
        return !mIsMouseInputSource;
    }

    @Override
    public boolean isDragSweepingEnabled() {
        // Enable drag sweeping for Vertical Tabs to ensure fast mouse drags
        // don't skip over swap targets.
        return true;
    }

    /**
     * Called when the selected state of a tab changes, such as when dragging starts or stops.
     * Updates the visual state and tab model selection.
     */
    @Override
    public void onSelectedChanged(RecyclerView.@Nullable ViewHolder viewHolder, int actionState) {
        super.onSelectedChanged(viewHolder, actionState);

        // TODO(crbug.com/518307037): Check for a TAB_GROUP header, cache its IS_COLLAPSED state
        // and:
        // (a) Trigger a collapse if needed when a drag starts.
        // (b) Trigger an expand if needed when dropped.

        if (actionState == ItemTouchHelper.ACTION_STATE_DRAG) {
            if (!hasTabPropertiesModel(viewHolder)) return;
            assumeNonNull(viewHolder);
            mSelectedTabIndex = viewHolder.getBindingAdapterPosition();
            mModel.updateSelectedCardForSelection(mSelectedTabIndex, true);

            selectTab(viewHolder);
        } else if (actionState == ItemTouchHelper.ACTION_STATE_IDLE) {
            if (mSelectedTabIndex != TabModel.INVALID_TAB_INDEX) {
                mModel.updateSelectedCardForSelection(mSelectedTabIndex, false);
                mSelectedTabIndex = TabModel.INVALID_TAB_INDEX;
            }
        }
    }

    private int getTabId(RecyclerView.ViewHolder viewHolder) {
        return assumeNonNull(((ViewHolder) viewHolder).model).get(TabProperties.TAB_ID);
    }

    private void selectTab(RecyclerView.ViewHolder viewHolder) {
        if (viewHolder.getItemViewType() == TabProperties.UiType.TAB_GROUP) {
            return;
        }
        TabModel tabModel = mCurrentTabModelSupplier.get();
        if (tabModel == null) return;

        int tabId = getTabId(viewHolder);
        Tab tab = tabModel.getTabById(tabId);
        if (tab == null) return;

        int index = tabModel.indexOf(tab);
        if (index != TabModel.INVALID_TAB_INDEX && index != tabModel.index()) {
            tabModel.setIndex(index, TabSelectionType.FROM_USER);
        }
    }

    /**
     * Creates an {@link RecyclerView.OnItemTouchListener} that detects mouse drags and initiates
     * instant dragging.
     *
     * @param itemTouchHelper The {@link ItemTouchHelper2} to trigger drags on.
     * @return A new {@link RecyclerView.OnItemTouchListener} instance.
     */
    public RecyclerView.OnItemTouchListener createMouseDragDetector(
            ItemTouchHelper2 itemTouchHelper) {
        return new RecyclerView.SimpleOnItemTouchListener() {
            private float mStartX;
            private float mStartY;
            private RecyclerView.@Nullable ViewHolder mActiveViewHolder;
            private boolean mTrackingMouseDrag;

            @Override
            public boolean onInterceptTouchEvent(RecyclerView rv, MotionEvent e) {
                if (!e.isFromSource(InputDevice.SOURCE_MOUSE)) {
                    return false;
                }
                int action = e.getActionMasked();
                if (action == MotionEvent.ACTION_DOWN) {
                    // First frame that mouse was pressed.
                    // Reset state.
                    mTrackingMouseDrag = false;
                    mActiveViewHolder = null;

                    // Only respond to the primary button (left click) for selection and dragging.
                    if (e.getButtonState() != MotionEvent.BUTTON_PRIMARY) {
                        return false;
                    }
                    View child = rv.findChildViewUnder(e.getX(), e.getY());
                    if (child != null) {
                        // Check if click was on action button (close button)
                        View actionButton = child.findViewById(R.id.action_button);
                        if (actionButton != null && actionButton.getVisibility() == View.VISIBLE) {
                            int[] buttonPos = new int[2];
                            actionButton.getLocationInWindow(buttonPos);
                            int[] rvPos = new int[2];
                            rv.getLocationInWindow(rvPos);

                            float relativeX = e.getX() - (buttonPos[0] - rvPos[0]);
                            float relativeY = e.getY() - (buttonPos[1] - rvPos[1]);

                            if (relativeX >= 0
                                    && relativeX < actionButton.getWidth()
                                    && relativeY >= 0
                                    && relativeY < actionButton.getHeight()) {
                                // Clicked on close button, don't drag.
                                return false;
                            }
                        }

                        // NOTE: getChildViewHolder() can return Tab Group Headers
                        // (UiType.TAB_GROUP). Headers don't have their own tab ID; they use a
                        // child's tab ID to represent themselves. setIndex() could end up switching
                        // the active web page when a user just clicks a header. If we observe this
                        // happening, we should filter this out for group headers.
                        //
                        mActiveViewHolder = rv.getChildViewHolder(child);
                        if (mActiveViewHolder != null) {
                            mStartX = e.getX();
                            mStartY = e.getY();
                            mTrackingMouseDrag = true;

                            // Select the tab immediately.
                            selectTab(mActiveViewHolder);
                        }
                    }
                } else if (action == MotionEvent.ACTION_MOVE) {
                    // Track a drag.
                    if (mTrackingMouseDrag && mActiveViewHolder != null) {
                        float dx = e.getX() - mStartX;
                        float dy = e.getY() - mStartY;
                        float distanceSquared = dx * dx + dy * dy;
                        if (distanceSquared > mMouseDragThresholdSquared) {
                            itemTouchHelper.startDrag(mActiveViewHolder);
                            mTrackingMouseDrag = false;
                            mActiveViewHolder = null;
                        }
                    }
                } else if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL) {
                    // Mouse was released.
                    mTrackingMouseDrag = false;
                    mActiveViewHolder = null;
                }
                return false;
            }
        };
    }
}
