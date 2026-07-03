// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/crosapi/local_printer_ash.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ash/constants/ash_features.h"
#include "ash/constants/chrome_pref_names.h"
#include "ash/webui/settings/public/constants/routes.mojom.h"
#include "base/check.h"
#include "base/check_deref.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/functional/callback_helpers.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/metrics/histogram_functions.h"
#include "base/values.h"
#include "chrome/browser/ash/printing/cups_printers_manager.h"
#include "chrome/browser/ash/printing/cups_printers_manager_factory.h"
#include "chrome/browser/ash/printing/history/print_job_info.pb.h"
#include "chrome/browser/ash/printing/oauth2/authorization_zones_manager.h"
#include "chrome/browser/ash/printing/oauth2/authorization_zones_manager_factory.h"
#include "chrome/browser/ash/printing/oauth2/status_code.h"
#include "chrome/browser/ash/printing/ppd_provider_factory.h"
#include "chrome/browser/ash/printing/print_management/printing_manager.h"
#include "chrome/browser/ash/printing/print_management/printing_manager_factory.h"
#include "chrome/browser/ash/printing/print_server.h"
#include "chrome/browser/ash/printing/print_servers_manager.h"
#include "chrome/browser/ash/printing/printer_authenticator.h"
#include "chrome/browser/ash/printing/printer_setup_util.h"
#include "chrome/browser/ash/profiles/profile_helper.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/printing/local_printer_utils_chromeos.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chromeos/ash/experiences/settings_ui/settings_app_manager.h"
#include "chromeos/crosapi/mojom/local_printer.mojom.h"
#include "chromeos/printing/ppd_provider.h"
#include "chromeos/printing/printer_configuration.h"
#include "components/prefs/pref_service.h"
#include "components/session_manager/core/session.h"
#include "components/session_manager/core/session_manager.h"
#include "components/user_manager/user.h"
#include "components/user_manager/user_manager.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "printing/backend/print_backend.h"
#include "printing/mojom/print.mojom.h"
#include "printing/print_job_constants.h"
#include "printing/print_settings.h"
#include "printing/printing_features.h"
#include "printing/printing_utils.h"
#include "url/gurl.h"

namespace crosapi {

LocalPrinterAsh::LocalPrinterAsh() = default;

LocalPrinterAsh::~LocalPrinterAsh() = default;

void LocalPrinterAsh::BindReceiver(
    mojo::PendingReceiver<mojom::LocalPrinter> pending_receiver) {
  receivers_.Add(this, std::move(pending_receiver));
}

void LocalPrinterAsh::GetPrinterTypeDenyList(
    GetPrinterTypeDenyListCallback callback) {
  Profile* profile = GetProfile();
  PrefService* prefs = profile->GetPrefs();

  std::vector<printing::mojom::PrinterType> deny_list;
  if (!prefs->HasPrefPath(ash::chrome_prefs::kPrinterTypeDenyList)) {
    std::move(callback).Run(deny_list);
    return;
  }

  const base::Value& deny_list_from_prefs =
      prefs->GetValue(ash::chrome_prefs::kPrinterTypeDenyList);

  deny_list.reserve(deny_list_from_prefs.GetList().size());
  for (const base::Value& deny_list_value : deny_list_from_prefs.GetList()) {
    const std::string& deny_list_str = deny_list_value.GetString();
    printing::mojom::PrinterType printer_type;
    if (deny_list_str == "extension") {
      printer_type = printing::mojom::PrinterType::kExtension;
    } else if (deny_list_str == "pdf") {
      printer_type = printing::mojom::PrinterType::kPdf;
    } else if (deny_list_str == "local") {
      printer_type = printing::mojom::PrinterType::kLocal;
    } else {
      continue;
    }

    deny_list.push_back(printer_type);
  }
  std::move(callback).Run(deny_list);
}

Profile* LocalPrinterAsh::GetProfile() {
  if (!user_manager::UserManager::IsInitialized() ||
      !user_manager::UserManager::Get()->IsUserLoggedIn()) {
    return nullptr;
  }
  return ProfileManager::GetPrimaryUserProfile();
}

scoped_refptr<chromeos::PpdProvider> LocalPrinterAsh::CreatePpdProvider(
    Profile* profile) {
  return ash::CreatePpdProvider(profile);
}


}  // namespace crosapi
