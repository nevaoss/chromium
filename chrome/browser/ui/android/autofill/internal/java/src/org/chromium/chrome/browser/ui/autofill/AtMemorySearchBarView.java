// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.ui.autofill;

import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;

import org.chromium.base.Callback;
import org.chromium.base.ui.KeyboardUtils;
import org.chromium.build.annotations.NullMarked;
import org.chromium.build.annotations.Nullable;
import org.chromium.chrome.browser.ui.autofill.internal.R;
import org.chromium.ui.widget.LoadingView;

/** Custom View representing the reusable search bar for AtMemory components. */
@NullMarked
public class AtMemorySearchBarView extends LinearLayout {
    private EditText mSearchEditText;
    private ImageView mSearchIcon;
    private LoadingView mSearchSpinner;
    private View mClearButton;

    public AtMemorySearchBarView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public AtMemorySearchBarView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mSearchEditText = findViewById(R.id.search_query_input);
        mSearchIcon = findViewById(R.id.search_icon);
        mSearchSpinner = findViewById(R.id.search_spinner);
        mClearButton = findViewById(R.id.search_clear_button);

        mClearButton.setOnClickListener(v -> clearSearchText());
        mSearchEditText.addTextChangedListener(
                new TextWatcher() {
                    @Override
                    public void beforeTextChanged(
                            CharSequence s, int start, int count, int after) {}

                    @Override
                    public void onTextChanged(CharSequence s, int start, int before, int count) {}

                    @Override
                    public void afterTextChanged(Editable s) {
                        mClearButton.setVisibility(s.length() > 0 ? View.VISIBLE : View.GONE);
                    }
                });
    }

    public void focusSearchArea() {
        // TODO(crbug.com/512802813): Fix cursor not blinking on subsequent openings of the bottom
        // sheet.
        mSearchEditText.requestFocus();
        KeyboardUtils.showKeyboard(mSearchEditText);
    }

    public void clearSearchText() {
        mSearchEditText.setText("");
    }

    public void setOnQuerySubmittedCallback(Callback<String> callback) {
        mSearchEditText.setOnEditorActionListener(
                (v, actionId, event) -> {
                    if (actionId == EditorInfo.IME_ACTION_SEARCH
                            || (event != null
                                    && event.getAction() == KeyEvent.ACTION_DOWN
                                    && event.getKeyCode() == KeyEvent.KEYCODE_ENTER)) {
                        KeyboardUtils.hideAndroidSoftKeyboard(mSearchEditText);
                        callback.onResult(v.getText().toString());
                        return true;
                    }
                    return false;
                });
    }

    public void setIsLoading(boolean isLoading) {
        mSearchIcon.setVisibility(isLoading ? View.GONE : View.VISIBLE);
        mSearchSpinner.setVisibility(isLoading ? View.VISIBLE : View.GONE);
    }
}
