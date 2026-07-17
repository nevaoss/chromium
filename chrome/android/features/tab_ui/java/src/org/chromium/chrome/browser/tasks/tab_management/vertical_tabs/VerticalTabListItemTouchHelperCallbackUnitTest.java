// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management.vertical_tabs;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;

import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.RecyclerView;
import androidx.test.core.app.ApplicationProvider;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.annotation.Config;

import org.chromium.base.Token;
import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabSelectionType;
import org.chromium.chrome.browser.tabmodel.TabModel;
import org.chromium.chrome.browser.tasks.tab_management.TabListModel;
import org.chromium.chrome.browser.tasks.tab_management.TabProperties;
import org.chromium.chrome.tab_ui.R;
import org.chromium.ui.modelutil.MVCListAdapter.ListItem;
import org.chromium.ui.modelutil.PropertyModel;
import org.chromium.ui.modelutil.SimpleRecyclerViewAdapter;
import org.chromium.ui.recyclerview.widget.ItemTouchHelper2;

import java.util.function.Supplier;

/** Unit tests for {@link VerticalTabListItemTouchHelperCallback}. */
@RunWith(BaseRobolectricTestRunner.class)
@Config(
        manifest = Config.NONE,
        instrumentedPackages = {
            "androidx.recyclerview.widget.RecyclerView" // required to mock final
        })
public class VerticalTabListItemTouchHelperCallbackUnitTest {
    @Mock private Supplier<TabModel> mCurrentTabModelSupplier;
    @Mock private TabModel mTabModel;
    @Mock private RecyclerView mRecyclerView;

    private TabListModel mModel;
    private SimpleRecyclerViewAdapter.ViewHolder mViewHolder;
    private SimpleRecyclerViewAdapter.ViewHolder mTargetViewHolder;

    private VerticalTabListItemTouchHelperCallback mCallback;
    private PropertyModel mPropertyModel;
    private PropertyModel mTargetPropertyModel;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        Context context = ApplicationProvider.getApplicationContext();

        when(mCurrentTabModelSupplier.get()).thenReturn(mTabModel);
        when(mRecyclerView.getContext()).thenReturn(context);

        // Set up the mocked property model for the dragged view holder.
        mPropertyModel =
                new PropertyModel.Builder(TabProperties.ALL_KEYS_TAB_GRID)
                        .with(
                                TabListModel.CardProperties.CARD_TYPE,
                                TabListModel.CardProperties.ModelType.TAB)
                        .with(TabProperties.TAB_ID, 1)
                        .build();

        View itemView = mock(View.class);
        mViewHolder = spy(new SimpleRecyclerViewAdapter.ViewHolder(itemView, /* binder= */ null));
        mViewHolder.model = mPropertyModel;

        // Set up the mocked property model for the target drop view holder.
        mTargetPropertyModel =
                new PropertyModel.Builder(TabProperties.ALL_KEYS_TAB_GRID)
                        .with(
                                TabListModel.CardProperties.CARD_TYPE,
                                TabListModel.CardProperties.ModelType.TAB)
                        .with(TabProperties.TAB_ID, 2)
                        .build();
        View targetItemView = mock(View.class);
        mTargetViewHolder =
                spy(new SimpleRecyclerViewAdapter.ViewHolder(targetItemView, /* binder= */ null));
        mTargetViewHolder.model = mTargetPropertyModel;

        mModel = new TabListModel();
        mModel.add(new ListItem(TabProperties.UiType.TAB, mPropertyModel));
        mModel.add(new ListItem(TabProperties.UiType.TAB, mTargetPropertyModel));

        mCallback =
                new VerticalTabListItemTouchHelperCallback(
                        context, mModel, mCurrentTabModelSupplier);
    }

    @Test
    public void testGetMovementFlags_RegularTab() {
        // Regular tabs can only move UP or DOWN.
        mPropertyModel.set(TabProperties.IS_PINNED, false);

        int flags = mCallback.getMovementFlags(mRecyclerView, mViewHolder);
        int dragFlags = ItemTouchHelper.UP | ItemTouchHelper.DOWN;
        assertEquals(ItemTouchHelper2.Callback.makeMovementFlags(dragFlags, 0), flags);
    }

    @Test
    public void testGetMovementFlags_PinnedTab() {
        // Pinned tabs can move UP, DOWN, LEFT, and RIGHT.
        mPropertyModel.set(TabProperties.IS_PINNED, true);

        int flags = mCallback.getMovementFlags(mRecyclerView, mViewHolder);
        int dragFlags =
                ItemTouchHelper.UP
                        | ItemTouchHelper.DOWN
                        | ItemTouchHelper.LEFT
                        | ItemTouchHelper.RIGHT;
        assertEquals(ItemTouchHelper2.Callback.makeMovementFlags(dragFlags, 0), flags);
    }

    @Test
    public void testCanDropOver_SameType() {
        // Both tabs are regular: drop allowed.
        mPropertyModel.set(TabProperties.IS_PINNED, false);
        mTargetPropertyModel.set(TabProperties.IS_PINNED, false);

        assertTrue(mCallback.canDropOver(mRecyclerView, mViewHolder, mTargetViewHolder));

        // Both tabs are pinned: drop allowed.
        mPropertyModel.set(TabProperties.IS_PINNED, true);
        mTargetPropertyModel.set(TabProperties.IS_PINNED, true);

        assertTrue(mCallback.canDropOver(mRecyclerView, mViewHolder, mTargetViewHolder));
    }

    @Test
    public void testCanDropOver_MixedType() {
        // Pinned dragging over regular: drop denied.
        mPropertyModel.set(TabProperties.IS_PINNED, true);
        mTargetPropertyModel.set(TabProperties.IS_PINNED, false);

        assertFalse(mCallback.canDropOver(mRecyclerView, mViewHolder, mTargetViewHolder));

        // Regular dragging over pinned: drop denied.
        mPropertyModel.set(TabProperties.IS_PINNED, false);
        mTargetPropertyModel.set(TabProperties.IS_PINNED, true);

        assertFalse(mCallback.canDropOver(mRecyclerView, mViewHolder, mTargetViewHolder));
    }

    @Test
    public void testOnMove_StandaloneTab() {
        // Verify onMove appropriately moves the tab in the TabModel based on bounds constraints.
        mPropertyModel.set(TabProperties.TAB_ID, 1);
        mTargetPropertyModel.set(TabProperties.TAB_ID, 2);

        Tab tab1 = mock(Tab.class);
        Tab tab2 = mock(Tab.class);
        when(tab1.getIsPinned()).thenReturn(false);
        when(tab2.getIsPinned()).thenReturn(false);
        doReturn(tab1).when(mTabModel).getTabById(1);
        doReturn(tab2).when(mTabModel).getTabById(2);

        when(mTabModel.indexOf(tab2)).thenReturn(5);
        when(mTabModel.findFirstNonPinnedTabIndex()).thenReturn(0);

        assertTrue(mCallback.onMove(mRecyclerView, mViewHolder, mTargetViewHolder));

        verify(mTabModel).moveRelatedTabs(1, 5);
    }

    @Test
    public void testOnMove_ChildTab() {
        // Verify onMove appropriately moves the tab in the TabModel based on bounds constraints.
        mPropertyModel.set(TabProperties.TAB_ID, 1);
        mTargetPropertyModel.set(TabProperties.TAB_ID, 2);

        Tab tab1 = mock(Tab.class);
        Tab tab2 = mock(Tab.class);
        when(tab1.getIsPinned()).thenReturn(false);
        when(tab2.getIsPinned()).thenReturn(false);

        // Set a group ID to make it a child tab
        Token groupId = new Token(1L, 2L);
        when(tab1.getTabGroupId()).thenReturn(groupId);

        doReturn(tab1).when(mTabModel).getTabById(1);
        doReturn(tab2).when(mTabModel).getTabById(2);

        when(mTabModel.indexOf(tab2)).thenReturn(5);
        when(mTabModel.findFirstNonPinnedTabIndex()).thenReturn(0);

        assertTrue(mCallback.onMove(mRecyclerView, mViewHolder, mTargetViewHolder));

        verify(mTabModel).moveTab(1, 5);
    }

    @Test
    public void testIsLongPressDragEnabled() {
        // Mouse input disables long press requirement for instant dragging.
        mCallback.setIsMouseInputSource(true);
        assertFalse(mCallback.isLongPressDragEnabled());

        // Touch input requires long press.
        mCallback.setIsMouseInputSource(false);
        assertTrue(mCallback.isLongPressDragEnabled());
    }

    @Test
    public void testOnSelectedChanged_Drag() {
        // Dragging highlights the selected card and activates it.
        when(mViewHolder.getBindingAdapterPosition()).thenReturn(0);

        Tab tab = mock(Tab.class);
        doReturn(tab).when(mTabModel).getTabById(1);
        when(mTabModel.indexOf(tab)).thenReturn(0);
        when(mTabModel.index()).thenReturn(1);

        mCallback.onSelectedChanged(mViewHolder, ItemTouchHelper.ACTION_STATE_DRAG);

        assertEquals(
                TabListModel.AnimationStatus.SELECTED_CARD_ZOOM_IN,
                mPropertyModel.get(TabListModel.CardProperties.CARD_ANIMATION_STATUS));
        assertEquals(0.8f, mPropertyModel.get(TabListModel.CardProperties.CARD_ALPHA), 0.01f);
        verify(mTabModel).setIndex(0, TabSelectionType.FROM_USER);
    }

    @Test
    public void testOnSelectedChanged_Idle() {
        // Setup initial drag state.
        when(mViewHolder.getBindingAdapterPosition()).thenReturn(0);

        Tab tab = mock(Tab.class);
        doReturn(tab).when(mTabModel).getTabById(1);
        when(mTabModel.indexOf(tab)).thenReturn(0);
        when(mTabModel.index()).thenReturn(1);

        mCallback.onSelectedChanged(mViewHolder, ItemTouchHelper.ACTION_STATE_DRAG);

        // Transition to idle clears the highlight.
        mCallback.onSelectedChanged(null, ItemTouchHelper.ACTION_STATE_IDLE);

        assertEquals(
                TabListModel.AnimationStatus.SELECTED_CARD_ZOOM_OUT,
                mPropertyModel.get(TabListModel.CardProperties.CARD_ANIMATION_STATUS));
        assertEquals(1.0f, mPropertyModel.get(TabListModel.CardProperties.CARD_ALPHA), 0.01f);
    }

    @Test
    public void testOnMove_updatesSelectedTabIndex() {
        // Setup initial drag state.
        when(mViewHolder.getBindingAdapterPosition()).thenReturn(0);

        Tab tab1 = mock(Tab.class);
        doReturn(tab1).when(mTabModel).getTabById(1);
        when(mTabModel.indexOf(tab1)).thenReturn(0);
        when(mTabModel.index()).thenReturn(1);

        mCallback.onSelectedChanged(mViewHolder, ItemTouchHelper.ACTION_STATE_DRAG);

        // Move to a new position.
        when(mTargetViewHolder.getBindingAdapterPosition()).thenReturn(1);
        Tab tab2 = mock(Tab.class);
        when(tab2.getIsPinned()).thenReturn(false);
        doReturn(tab2).when(mTabModel).getTabById(2);
        when(mTabModel.indexOf(tab2)).thenReturn(1);
        when(mTabModel.findFirstNonPinnedTabIndex()).thenReturn(0);

        mCallback.onMove(mRecyclerView, mViewHolder, mTargetViewHolder);

        // Transition to idle should clear the highlight at the NEW position.
        mCallback.onSelectedChanged(null, ItemTouchHelper.ACTION_STATE_IDLE);

        assertEquals(
                TabListModel.AnimationStatus.SELECTED_CARD_ZOOM_OUT,
                mTargetPropertyModel.get(TabListModel.CardProperties.CARD_ANIMATION_STATUS));
        assertEquals(1.0f, mTargetPropertyModel.get(TabListModel.CardProperties.CARD_ALPHA), 0.01f);
    }

    @Test
    public void testCreateMouseDragDetector_ActionDownSelectsTab() {
        ItemTouchHelper2 itemTouchHelper = mock(ItemTouchHelper2.class);
        RecyclerView.OnItemTouchListener listener =
                mCallback.createMouseDragDetector(itemTouchHelper);

        MotionEvent event = createMouseEvent(MotionEvent.ACTION_DOWN, 10f, 10f);

        View childView = mock(View.class);
        when(mRecyclerView.findChildViewUnder(10f, 10f)).thenReturn(childView);
        when(mRecyclerView.getChildViewHolder(childView)).thenReturn(mViewHolder);

        Tab tab = mock(Tab.class);
        doReturn(tab).when(mTabModel).getTabById(1);
        when(mTabModel.indexOf(tab)).thenReturn(0);
        when(mTabModel.index()).thenReturn(1);

        boolean intercepted = listener.onInterceptTouchEvent(mRecyclerView, event);

        assertFalse(intercepted);
        verify(mTabModel).setIndex(0, TabSelectionType.FROM_USER);

        event.recycle();
    }

    private MotionEvent createMouseEvent(int action, float x, float y) {
        return createMouseEvent(action, x, y, MotionEvent.BUTTON_PRIMARY);
    }

    private MotionEvent createMouseEvent(int action, float x, float y, int buttonState) {
        long time = 1000L;
        MotionEvent.PointerProperties[] properties = new MotionEvent.PointerProperties[1];
        properties[0] = new MotionEvent.PointerProperties();
        properties[0].id = 0;
        properties[0].toolType = MotionEvent.TOOL_TYPE_MOUSE;

        MotionEvent.PointerCoords[] coords = new MotionEvent.PointerCoords[1];
        coords[0] = new MotionEvent.PointerCoords();
        coords[0].x = x;
        coords[0].y = y;
        coords[0].pressure = 1.0f;
        coords[0].size = 1.0f;

        MotionEvent event =
                MotionEvent.obtain(
                        /* downTime= */ time,
                        /* eventTime= */ time,
                        /* action= */ action,
                        /* pointerCount= */ 1,
                        /* pointerProperties= */ properties,
                        /* pointerCoords= */ coords,
                        /* metaState= */ 0,
                        /* buttonState= */ buttonState,
                        /* xPrecision= */ 1.0f,
                        /* yPrecision= */ 1.0f,
                        /* deviceId= */ 0,
                        /* edgeFlags= */ 0,
                        /* source= */ InputDevice.SOURCE_MOUSE,
                        /* flags= */ 0);
        return event;
    }

    @Test
    public void testCreateMouseDragDetector_ActionMoveTriggersDrag() {
        Context context = ApplicationProvider.getApplicationContext();
        int touchSlop = ViewConfiguration.get(context).getScaledTouchSlop();

        ItemTouchHelper2 itemTouchHelper = mock(ItemTouchHelper2.class);
        RecyclerView.OnItemTouchListener listener =
                mCallback.createMouseDragDetector(itemTouchHelper);

        // 1. ACTION_DOWN
        MotionEvent downEvent = createMouseEvent(MotionEvent.ACTION_DOWN, 10f, 10f);
        View childView = mock(View.class);
        when(mRecyclerView.findChildViewUnder(10f, 10f)).thenReturn(childView);
        when(mRecyclerView.getChildViewHolder(childView)).thenReturn(mViewHolder);

        // Stub tab model to avoid NPE during selection in ACTION_DOWN
        Tab tab = mock(Tab.class);
        doReturn(tab).when(mTabModel).getTabById(1);
        when(mTabModel.indexOf(tab)).thenReturn(0);
        when(mTabModel.index()).thenReturn(1);

        listener.onInterceptTouchEvent(mRecyclerView, downEvent);

        // 2. ACTION_MOVE (exceeding slop)
        float moveY = 10f + (touchSlop / 4f) + 5f;
        MotionEvent moveEvent = createMouseEvent(MotionEvent.ACTION_MOVE, 10f, moveY);

        boolean intercepted = listener.onInterceptTouchEvent(mRecyclerView, moveEvent);

        assertFalse(intercepted);
        verify(itemTouchHelper).startDrag(mViewHolder);

        downEvent.recycle();
        moveEvent.recycle();
    }

    @Test
    public void testCreateMouseDragDetector_CloseButtonClickNoDragNoSelect() {
        ItemTouchHelper2 itemTouchHelper = mock(ItemTouchHelper2.class);
        RecyclerView.OnItemTouchListener listener =
                mCallback.createMouseDragDetector(itemTouchHelper);

        // Setup views
        View childView = mock(View.class);
        View actionButton = mock(View.class);
        when(childView.findViewById(R.id.action_button)).thenReturn(actionButton);
        when(actionButton.getVisibility()).thenReturn(View.VISIBLE);

        // Stub dimensions and locations
        doAnswer(
                        invocation -> {
                            int[] pos = invocation.getArgument(0);
                            pos[0] = 100;
                            pos[1] = 100;
                            return null;
                        })
                .when(actionButton)
                .getLocationInWindow(any(int[].class));

        doAnswer(
                        invocation -> {
                            int[] pos = invocation.getArgument(0);
                            pos[0] = 0;
                            pos[1] = 0;
                            return null;
                        })
                .when(mRecyclerView)
                .getLocationInWindow(any(int[].class));

        when(actionButton.getWidth()).thenReturn(50);
        when(actionButton.getHeight()).thenReturn(50);

        // Click at (120, 120) relative to RecyclerView (inside the close button)
        MotionEvent downEvent = createMouseEvent(MotionEvent.ACTION_DOWN, 120f, 120f);

        when(mRecyclerView.findChildViewUnder(120f, 120f)).thenReturn(childView);
        when(mRecyclerView.getChildViewHolder(childView)).thenReturn(mViewHolder);

        // ACTION_DOWN
        boolean intercepted = listener.onInterceptTouchEvent(mRecyclerView, downEvent);
        assertFalse(intercepted);

        // Verify NO tab selection occurred
        verify(mTabModel, never()).setIndex(anyInt(), anyInt());

        // ACTION_MOVE (should not drag)
        Context context = ApplicationProvider.getApplicationContext();
        int touchSlop = ViewConfiguration.get(context).getScaledTouchSlop();
        MotionEvent moveEvent = createMouseEvent(MotionEvent.ACTION_MOVE, 120f, 120f + touchSlop);
        listener.onInterceptTouchEvent(mRecyclerView, moveEvent);

        verify(itemTouchHelper, never()).startDrag(any());

        downEvent.recycle();
        moveEvent.recycle();
    }

    @Test
    public void testCreateMouseDragDetector_GroupHeaderNoSelectButDrags() {
        Context context = ApplicationProvider.getApplicationContext();
        int touchSlop = ViewConfiguration.get(context).getScaledTouchSlop();

        ItemTouchHelper2 itemTouchHelper = mock(ItemTouchHelper2.class);
        RecyclerView.OnItemTouchListener listener =
                mCallback.createMouseDragDetector(itemTouchHelper);

        // Set up ViewHolder as TAB_GROUP header
        PropertyModel groupHeaderModel =
                new PropertyModel.Builder(TabProperties.ALL_KEYS_TAB_GRID).build();
        View headerView = mock(View.class);
        SimpleRecyclerViewAdapter.ViewHolder headerViewHolder =
                spy(new SimpleRecyclerViewAdapter.ViewHolder(headerView, /* binder= */ null));
        headerViewHolder.model = groupHeaderModel;
        when(headerViewHolder.getItemViewType()).thenReturn(TabProperties.UiType.TAB_GROUP);

        // ACTION_DOWN
        MotionEvent downEvent = createMouseEvent(MotionEvent.ACTION_DOWN, 10f, 10f);
        when(mRecyclerView.findChildViewUnder(10f, 10f)).thenReturn(headerView);
        when(mRecyclerView.getChildViewHolder(headerView)).thenReturn(headerViewHolder);

        listener.onInterceptTouchEvent(mRecyclerView, downEvent);

        // Verify NO tab selection occurred
        verify(mTabModel, never()).setIndex(anyInt(), anyInt());

        // ACTION_MOVE (should still drag)
        float moveY = 10f + (touchSlop / 4f) + 5f;
        MotionEvent moveEvent = createMouseEvent(MotionEvent.ACTION_MOVE, 10f, moveY);

        listener.onInterceptTouchEvent(mRecyclerView, moveEvent);

        // Verify drag WAS triggered
        verify(itemTouchHelper).startDrag(headerViewHolder);

        downEvent.recycle();
        moveEvent.recycle();
    }

    @Test
    public void testCreateMouseDragDetector_RightClickIgnored() {
        ItemTouchHelper2 itemTouchHelper = mock(ItemTouchHelper2.class);
        RecyclerView.OnItemTouchListener listener =
                mCallback.createMouseDragDetector(itemTouchHelper);

        // Simulate a RIGHT click (BUTTON_SECONDARY)
        MotionEvent event =
                createMouseEvent(MotionEvent.ACTION_DOWN, 10f, 10f, MotionEvent.BUTTON_SECONDARY);

        View childView = mock(View.class);
        when(mRecyclerView.findChildViewUnder(10f, 10f)).thenReturn(childView);
        when(mRecyclerView.getChildViewHolder(childView)).thenReturn(mViewHolder);

        Tab tab = mock(Tab.class);
        doReturn(tab).when(mTabModel).getTabById(1);
        when(mTabModel.indexOf(tab)).thenReturn(0);
        when(mTabModel.index()).thenReturn(1);

        boolean intercepted = listener.onInterceptTouchEvent(mRecyclerView, event);

        assertFalse(intercepted);
        verify(mTabModel, never()).setIndex(anyInt(), anyInt());

        event.recycle();
    }
}
