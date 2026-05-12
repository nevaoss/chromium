// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_FORM_PARSING_PASSWORD_FIELD_PREDICTION_H_
#define COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_FORM_PARSING_PASSWORD_FIELD_PREDICTION_H_

#include <stdint.h>

#include <vector>

#include "base/containers/flat_map.h"
#include "build/build_config.h"
#include "components/autofill/core/browser/autofill_type.h"
#include "components/autofill/core/browser/field_types.h"
#include "components/autofill/core/common/signatures.h"
#include "components/autofill/core/common/unique_ids.h"

namespace password_manager {

enum class CredentialFieldType {
  kNone,
  kUsername,
  kSingleUsername,
  kCurrentPassword,
  kNewPassword,
  kConfirmationPassword,
  kNonCredential
};

// Transforms the general field type to the information useful for password
// forms.
CredentialFieldType DeriveFromFieldType(autofill::FieldType type);

// Contains server predictions for a field.
struct PasswordFieldPrediction {
  PasswordFieldPrediction(autofill::FieldRendererId renderer_id,
                          autofill::FieldSignature signature,
                          autofill::FieldType type,
                          bool may_use_prefilled_placeholder,
                          bool is_override);
  PasswordFieldPrediction(const PasswordFieldPrediction&);
  PasswordFieldPrediction& operator=(const PasswordFieldPrediction&);
  PasswordFieldPrediction(PasswordFieldPrediction&&);
  PasswordFieldPrediction& operator=(PasswordFieldPrediction&&);
  ~PasswordFieldPrediction();

  autofill::FieldRendererId renderer_id;
  autofill::FieldSignature signature;
  autofill::FieldType type;
  bool may_use_prefilled_placeholder;
  bool is_override;

// TODO(neva): Remove this when Neva GCC starts supporting C++20.
#if (__cplusplus < 202002L)
  friend bool operator==(const PasswordFieldPrediction& lhs,
                         const PasswordFieldPrediction& rhs) {
    return lhs.renderer_id == rhs.renderer_id &&
           lhs.signature == rhs.signature && lhs.type == rhs.type &&
           lhs.may_use_prefilled_placeholder ==
               rhs.may_use_prefilled_placeholder;
  }
#else   // (__cplusplus < 202002L)
  friend bool operator==(const PasswordFieldPrediction& lhs,
                         const PasswordFieldPrediction& rhs) = default;
#endif  // !(__cplusplus < 202002L)
};

// Contains server predictions for a form.
struct FormPredictions {
  FormPredictions();
  FormPredictions(const FormPredictions&);
  FormPredictions& operator=(const FormPredictions&);
  FormPredictions(FormPredictions&&);
  FormPredictions& operator=(FormPredictions&&);
  ~FormPredictions();

  // Id of PasswordManagerDriver which corresponds to the frame of this form.
  int driver_id = 0;

  autofill::FormSignature form_signature;
  std::vector<PasswordFieldPrediction> fields;

// TODO(neva): Remove this when Neva GCC starts supporting C++20.
#if (__cplusplus < 202002L)
  friend bool operator==(const FormPredictions& lhs,
                         const FormPredictions& rhs) {
    return lhs.driver_id == rhs.driver_id &&
           lhs.form_signature == rhs.form_signature && lhs.fields == rhs.fields;
  }
#else   // (__cplusplus < 202002L)
  friend bool operator==(const FormPredictions& lhs,
                         const FormPredictions& rhs) = default;
#endif  // !(__cplusplus < 202002L)
};

// Extracts password related server predictions from `form` and `predictions`.
FormPredictions ConvertToFormPredictions(
    int driver_id,
    const autofill::FormData& form,
    const base::flat_map<autofill::FieldGlobalId,
                         autofill::AutofillType::ServerPrediction>&
        predictions);

}  // namespace password_manager

#endif  // COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_FORM_PARSING_PASSWORD_FIELD_PREDICTION_H_
