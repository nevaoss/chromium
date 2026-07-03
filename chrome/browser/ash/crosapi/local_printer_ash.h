// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_CROSAPI_LOCAL_PRINTER_ASH_H_
#define CHROME_BROWSER_ASH_CROSAPI_LOCAL_PRINTER_ASH_H_

#include "base/memory/scoped_refptr.h"
#include "chromeos/crosapi/mojom/local_printer.mojom.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver_set.h"

class Profile;

namespace chromeos {
class PpdProvider;
}  // namespace chromeos

namespace crosapi {

// Implements the crosapi interface for LocalPrinter. Lives in Ash-Chrome on the
// UI thread.
class LocalPrinterAsh : public mojom::LocalPrinter {
 public:
  LocalPrinterAsh();
  LocalPrinterAsh(const LocalPrinterAsh&) = delete;
  LocalPrinterAsh& operator=(const LocalPrinterAsh&) = delete;

  // As it observes browser context keyed services, this object must
  // be destroyed after said services are destroyed (which occurs in
  // ChromeBrowserMainParts::PostMainMessageLoopRun()). CrosapiAsh
  // is destroyed in ~ChromeBrowserMainParts() which occurs after
  // all browser context keyed services have been destroyed.
  ~LocalPrinterAsh() override;

  void BindReceiver(mojo::PendingReceiver<mojom::LocalPrinter> receiver);

  // crosapi::mojom::LocalPrinter:
  void GetPrinterTypeDenyList(GetPrinterTypeDenyListCallback callback) override;

 private:
  // Exposed so that unit tests can override them.
  virtual Profile* GetProfile();
  virtual scoped_refptr<chromeos::PpdProvider> CreatePpdProvider(
      Profile* profile);

  // This class supports any number of connections. This allows the client to
  // have multiple, potentially thread-affine, remotes.
  mojo::ReceiverSet<mojom::LocalPrinter> receivers_;
};

}  // namespace crosapi

#endif  // CHROME_BROWSER_ASH_CROSAPI_LOCAL_PRINTER_ASH_H_
