// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;

import androidx.test.core.app.ApplicationProvider;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.shadows.ShadowLooper;

import org.chromium.base.UserDataHost;
import org.chromium.base.supplier.ObservableSuppliers;
import org.chromium.base.supplier.SettableNullableObservableSupplier;
import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.base.test.RobolectricUtil;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabObserver;
import org.chromium.chrome.browser.tab.TabSelectionType;
import org.chromium.chrome.browser.tab.state.PersistedTabDataConfiguration;
import org.chromium.chrome.browser.tab.state.SendTabToSelfTabCardLabelData;
import org.chromium.chrome.browser.tabmodel.TabModel;

import java.util.Collections;
import java.util.Map;

/** Unit tests for {@link SendTabToSelfTabLabeller}. */
@RunWith(BaseRobolectricTestRunner.class)
public class SendTabToSelfTabLabellerUnitTest {
    @Rule public final MockitoRule mMockitoRule = MockitoJUnit.rule();

    private static final int TAB_ID = 1;
    private static final String SENDER_DEVICE_NAME = "Example Phone";

    @Mock private TabListNotificationHandler mTabListNotificationHandler;
    @Mock private TabModel mTabModel;
    @Mock private Tab mTab;

    @Captor private ArgumentCaptor<Map<Integer, TabCardLabelData>> mLabelDataCaptor;
    @Captor private ArgumentCaptor<TabObserver> mTabObserverCaptor;

    private final SettableNullableObservableSupplier<TabModel> mTabModelSupplier =
            ObservableSuppliers.createNullable();

    private Context mContext;
    private UserDataHost mUserDataHost;
    private SendTabToSelfTabLabeller mLabeller;

    private SendTabToSelfTabCardLabelData createAndSetLabelData() {
        SendTabToSelfTabCardLabelData sttsData =
                new SendTabToSelfTabCardLabelData(
                        mTab, SENDER_DEVICE_NAME, System.currentTimeMillis());
        mUserDataHost.setUserData(SendTabToSelfTabCardLabelData.class, sttsData);
        return sttsData;
    }

    private void verifyTabCardLabelUpdated(String expectedText, int verifyTimes) {
        verify(mTabListNotificationHandler, times(verifyTimes))
                .updateTabCardLabels(mLabelDataCaptor.capture());
        Map<Integer, TabCardLabelData> labelDataMap = mLabelDataCaptor.getValue();

        assertTrue(labelDataMap.containsKey(TAB_ID));
        TabCardLabelData labelData = labelDataMap.get(TAB_ID);
        if (expectedText == null) {
            assertNull(labelData);
        } else {
            assertNotNull(labelData);
            assertEquals(expectedText, labelData.textResolver.resolve(mContext));
        }
    }

    @Before
    public void setUp() {
        // Required to allow SendTabToSelfTabCardLabelData to be initialized.
        PersistedTabDataConfiguration.setUseTestConfig(true);

        mContext = ApplicationProvider.getApplicationContext();
        mUserDataHost = new UserDataHost();

        when(mTab.getId()).thenReturn(TAB_ID);
        when(mTab.isInitialized()).thenReturn(true);
        when(mTab.getUserDataHost()).thenReturn(mUserDataHost);

        mTabModelSupplier.set(mTabModel);
        when(mTabModel.getCount()).thenReturn(1);
        when(mTabModel.getTabAt(0)).thenReturn(mTab);

        mLabeller = new SendTabToSelfTabLabeller(mTabListNotificationHandler, mTabModelSupplier);
    }

    @Test
    public void testShowAll_ValidLabelPushed() {
        createAndSetLabelData();

        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated("From Example Phone", 1);
    }

    @Test
    public void testShowAll_LabelLoadedFromDB() {
        SendTabToSelfTabCardLabelData sttsData = createAndSetLabelData();
        // Assume the data exists on the db and not in the UserDataHost.
        mUserDataHost.removeUserData(SendTabToSelfTabCardLabelData.class);
        sttsData.save();
        RobolectricUtil.runAllBackgroundAndUi();

        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated("From Example Phone", 1);

        // To avoid leaking the data to other tests, delete it from the db.
        PersistedTabDataConfiguration.TEST_CONFIG
                .getStorage()
                .delete(
                        TAB_ID,
                        PersistedTabDataConfiguration.SEND_TAB_TO_SELF_TAB_CARD_LABEL_DATA.getId());
    }

    @Test
    public void testShowAll_NoLabel() {
        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated(null, 1);
    }

    @Test
    public void testShowAll_Expired() {
        SendTabToSelfTabCardLabelData sttsData = createAndSetLabelData();
        sttsData.setAdditionTimestampMsForTesting(
                System.currentTimeMillis() - 6L * 24 * 60 * 60 * 1000); // 6 days old

        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated(null, 1);
    }

    @Test
    public void testShowAll_LabelClearedOnUpdate() {
        createAndSetLabelData();
        verify(mTab).addObserver(mTabObserverCaptor.capture());

        // Initial showAll pushes mTab's label.
        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated("From Example Phone", 1);

        // Simulate user interaction on mTab, which deletes the label data from mUserDataHost.
        mTabObserverCaptor.getValue().onShown(mTab, TabSelectionType.FROM_USER);

        // Re-run showAll. mTab now has no data, so it pushes null to clear the label.
        reset(mTabListNotificationHandler);
        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated(null, 1);
    }

    @Test
    public void testShowAll_LabelPreservedIfDifferentTabInteracted() {
        createAndSetLabelData();
        verify(mTab).addObserver(mTabObserverCaptor.capture());

        // Initial showAll pushes mTab's label.
        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated("From Example Phone", 1);

        // Simulate user interaction on a different tab. mTab's label data remains active.
        Tab otherTab = mock(Tab.class);
        when(otherTab.getUserDataHost()).thenReturn(new UserDataHost());
        mTabObserverCaptor.getValue().onShown(otherTab, TabSelectionType.FROM_USER);

        // Re-run showAll. mTab's label is perfectly preserved.
        mLabeller.showAll(Collections.singletonList(mTab));
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated("From Example Phone", 2);
    }

    @Test
    public void testShowAll_NullList() {
        createAndSetLabelData();

        // Passing null triggers the fallback to mCurrentTabModel/mTabModelSupplier.
        mLabeller.showAll(null);
        RobolectricUtil.runAllBackgroundAndUi();

        verifyTabCardLabelUpdated("From Example Phone", 1);
    }

    @Test
    public void testShowAll_NegativeCache_Synchronous() {
        // Create and set negative cache in memory.
        SendTabToSelfTabCardLabelData sttsData =
                new SendTabToSelfTabCardLabelData(
                        mTab, /* senderDeviceName= */ "", /* additionTimestampMs= */ 0);
        mUserDataHost.setUserData(SendTabToSelfTabCardLabelData.class, sttsData);

        mLabeller.showAll(Collections.singletonList(mTab));

        // Idle the main looper to execute the posted showAll task.
        // Since it is synchronous, it should complete and update the UI immediately.
        ShadowLooper.idleMainLooper();

        // Verify the label was updated to null (cleared) synchronously.
        verifyTabCardLabelUpdated(null, 1);
    }

    @Test
    public void testDestroy() {
        mLabeller.destroy();
        verify(mTabModel).removeObserver(mLabeller);
    }

    @Test
    public void testOnTabModelChange() {
        TabModel newTabModel = mock(TabModel.class);
        mTabModelSupplier.set(newTabModel);

        verify(mTabModel).removeObserver(mLabeller);
        verify(newTabModel).addObserver(mLabeller);
    }
}
