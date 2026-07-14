// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ui.autofill;

import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetProperties.IS_LOADING;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetProperties.ON_QUERY_SUBMITTED_CALLBACK;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetProperties.VISIBLE;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetSuggestionProperties.ALL_PROPERTIES;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetSuggestionProperties.DETAILS;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetSuggestionProperties.ICON;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetSuggestionProperties.ON_FLYOUT_CLICKED;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetSuggestionProperties.ON_SUGGESTION_CLICKED;
import static org.chromium.chrome.browser.ui.autofill.AtMemoryBottomSheetSuggestionProperties.TITLE;

import org.chromium.build.annotations.NullMarked;
import org.chromium.components.autofill.AutofillSuggestion;
import org.chromium.ui.modelutil.MVCListAdapter.ListItem;
import org.chromium.ui.modelutil.MVCListAdapter.ModelList;
import org.chromium.ui.modelutil.PropertyModel;

import java.util.List;

/** Contains the business logic for the AtMemoryBottomSheet. */
@NullMarked
class AtMemoryBottomSheetMediator {
    private final PropertyModel mModel;
    private final ModelList mModelList;
    private final AtMemoryBottomSheetCoordinator.Delegate mDelegate;

    AtMemoryBottomSheetMediator(
            AtMemoryBottomSheetCoordinator.Delegate delegate,
            PropertyModel model,
            ModelList modelList) {
        mModel = model;
        mModelList = modelList;
        mDelegate = delegate;

        mModel.set(ON_QUERY_SUBMITTED_CALLBACK, this::onQuerySubmitted);
    }

    void show(List<AutofillSuggestion> suggestions) {
        setSuggestions(suggestions);
        // When an async search begins, the backend clears old results by sending an empty list.
        // We only reset the loading indicator when non-empty suggestions arrive. Completed searches
        // always return non-empty lists (using fallback items when no data is found).
        if (!suggestions.isEmpty()) {
            mModel.set(IS_LOADING, false);
        }
        mModel.set(VISIBLE, true);
    }

    void onDismissed() {
        mModelList.clear();
        mModel.set(IS_LOADING, false);
        mModel.set(VISIBLE, false);
        mDelegate.onDismissed();
    }

    private void setSuggestions(List<AutofillSuggestion> suggestions) {
        mModelList.clear();

        for (int i = 0; i < suggestions.size(); i++) {
            AutofillSuggestion suggestion = suggestions.get(i);
            int position = i;
            PropertyModel itemModel =
                    new PropertyModel.Builder(ALL_PROPERTIES)
                            .with(ICON, suggestion.getIconId())
                            .with(TITLE, suggestion.getLabel())
                            .with(DETAILS, suggestion.getSublabel())
                            .with(ON_SUGGESTION_CLICKED, () -> onSuggestionClicked(position))
                            .with(ON_FLYOUT_CLICKED, () -> onFlyoutClicked(suggestion))
                            .build();
            ListItem listItem =
                    new ListItem(AtMemoryBottomSheetCoordinator.ITEM_TYPE_SUGGESTION, itemModel);
            mModelList.add(listItem);
        }
    }

    private void onSuggestionClicked(int position) {
        mDelegate.onSuggestionClicked(position);
    }

    private void onFlyoutClicked(AutofillSuggestion suggestion) {
        mDelegate.onFlyoutClicked(suggestion);
    }

    void onQuerySubmitted(String query) {
        mModel.set(IS_LOADING, true);
        mDelegate.onQuerySubmitted(query);
    }
}
