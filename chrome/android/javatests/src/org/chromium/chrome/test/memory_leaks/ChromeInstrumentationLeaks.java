// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.test.memory_leaks;

import org.chromium.base.test.util.LeakCanaryChecker.LeakCanaryConfigProvider;
import org.chromium.build.annotations.IdentifierNameString;
import org.chromium.build.annotations.ServiceImpl;

import java.util.List;
import java.util.Map;

@SuppressWarnings("FieldCanBeFinal") // @IdentifierNameString requires non-final
@ServiceImpl(LeakCanaryConfigProvider.class)
public class ChromeInstrumentationLeaks implements LeakCanaryConfigProvider {
    // This class is a collection of known leaks in Chrome Instrumentation tests (eg.
    // chrome_public_test_apk). The goal is to  burn this class down to nothing by fixing leaks.
    // Please include a bug for each leak.

    // crbug.com/462704925
    @IdentifierNameString
    private static String sClass462704925 = "org.chromium.ui.KeyboardVisibilityDelegate";

    @IdentifierNameString
    private static String sField462704925 = "org.chromium.ui.KeyboardVisibilityDelegate#sInstance";

    // In the rare case that the cleanup task hasn't finished yet, ignore the "leak" - it gets
    // cleaned up once the cleanup task happens.
    @IdentifierNameString
    private static String sClassPersistencePolicyCleanup =
            "org.chromium.chrome.browser.tabmodel.TabbedModeTabPersistencePolicy";

    @IdentifierNameString
    private static String sFieldPersistencePolicyCleanup =
            "org.chromium.chrome.browser.tabmodel.TabbedModeTabPersistencePolicy#sCleanupTask";

    // crbug.com/512489883
    @IdentifierNameString
    private static String sClassTabBottomSheet =
            "org.chromium.chrome.browser.tab_bottom_sheet.TabBottomSheetNativeInterface";

    @IdentifierNameString
    private static String sFieldTabBottomSheet =
            "org.chromium.chrome.browser.tab_bottom_sheet.TabBottomSheetNativeInterface#mTab";

    // crbug.com/527131033
    @IdentifierNameString
    private static String sClassShoppingData =
            "org.chromium.chrome.browser.tab.state.ShoppingPersistedTabData";

    @IdentifierNameString
    private static String sFieldShoppingData =
            "org.chromium.chrome.browser.tab.state.ShoppingPersistedTabData#sShoppingDataRequests";

    @Override
    public Map<String, String> getStaticFieldLeaks() {
        return Map.of(
                sClass462704925, sField462704925,
                sClassPersistencePolicyCleanup, sFieldPersistencePolicyCleanup,
                sClassShoppingData, sFieldShoppingData);
    }

    @Override
    public Map<String, String> getInstanceFieldLeaks() {
        return Map.of(sClassTabBottomSheet, sFieldTabBottomSheet);
    }

    @Override
    public List<String> getJavaLocalLeaks() {
        // crbug.com/465145691
        return List.of("AsyncLayoutInflator");
    }
}
