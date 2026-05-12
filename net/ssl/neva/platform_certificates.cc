// Copyright 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "net/ssl/neva/platform_certificates.h"

#include <mutex>
#include <vector>

#include "crypto/nss_key_util.h"
#include "crypto/rsa_private_key.h"
#include "net/cert/x509_util_nss.h"
#include "third_party/boringssl/src/include/openssl/bio.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/pem.h"

namespace net {

namespace {

std::once_flag import_certs_flag;

void ImportClientCertPrivateKey(const X509Certificate* cert,
                                const X509Certificate* ca_cert,
                                EVP_PKEY* private_key,
                                PK11SlotInfo* slot) {
  if (!cert || !private_key || !slot)
    return;

  std::unique_ptr<crypto::RSAPrivateKey> rsa_private_key =
      crypto::RSAPrivateKey::CreateFromKey(private_key);
  if (!rsa_private_key)
    return;

  std::vector<uint8_t> key_vector;
  if (!rsa_private_key->ExportPrivateKey(&key_vector))
    return;

  crypto::ScopedSECKEYPrivateKey sec_key =
      crypto::ImportNSSKeyFromPrivateKeyInfo(slot, key_vector, true);
  if (!sec_key)
    return;

  ScopedCERTCertificate nss_cert =
      x509_util::CreateCERTCertificateFromX509Certificate(cert);
  if (!nss_cert)
    return;

  std::string nickname =
      x509_util::GetDefaultUniqueNickname(nss_cert.get(), USER_CERT, slot);
  PK11_ImportCert(slot, nss_cert.get(), sec_key->pkcs11ID, nickname.c_str(),
                  PR_FALSE);

  if (!ca_cert)
    return;

  ScopedCERTCertificate ca_nss_cert =
      x509_util::CreateCERTCertificateFromX509Certificate(ca_cert);
  if (!ca_nss_cert)
    return;

  nickname =
      x509_util::GetDefaultUniqueNickname(ca_nss_cert.get(), CA_CERT, slot);
  PK11_ImportCert(slot, ca_nss_cert.get(), CK_INVALID_HANDLE, nickname.c_str(),
                  PR_FALSE);
}

void ImportMemoryAllocatedClientCertificates() {
  int nCertificates = 0;
  char **certificates = NULL, **privateKeys = NULL, **ca_certificates = NULL;
  int result = neva::MultipleReadClientKey(&certificates, &privateKeys,
                                           &ca_certificates, &nCertificates);
  if (result == 0) {
    crypto::ScopedPK11Slot slot(PK11_GetInternalKeySlot());
    for (int i = 0; i < nCertificates; i++) {
      CertificateList certs = X509Certificate::CreateCertificateListFromBytes(
          base::as_bytes(
              base::make_span(certificates[i], strlen(certificates[i]))),
          X509Certificate::FORMAT_AUTO);
      if (!certs.empty()) {
        CertificateList ca_certs =
            ca_certificates[i]
                ? X509Certificate::CreateCertificateListFromBytes(
                      base::as_bytes(base::make_span(
                                         ca_certificates[i],
                                         strlen(ca_certificates[i]))),
                      X509Certificate::FORMAT_AUTO)
                : CertificateList();
        bssl::UniquePtr<EVP_PKEY> pkey;
        bssl::UniquePtr<BIO> bio(BIO_new_mem_buf(
            privateKeys[i], static_cast<int>(strlen(privateKeys[i]))));
        if (bio.get()) {
          EVP_PKEY* evp_pkey =
              PEM_read_bio_PrivateKey(bio.get(), NULL, NULL, NULL);
          if (evp_pkey != NULL) {
            pkey.reset(evp_pkey);
            ImportClientCertPrivateKey(
                certs[0].get(),
                !ca_certs.empty() ? ca_certs[0].get() : nullptr,
                pkey.get(), slot.get());
          }
        }
      }
      free(certificates[i]);
      free(privateKeys[i]);
      free(ca_certificates[i]);
    }
    free(certificates);
    free(privateKeys);
    free(ca_certificates);
  }
}

}  // namespace

void EnsurePlatformCerts() {
  std::call_once(import_certs_flag, ImportMemoryAllocatedClientCertificates);
}

}  // namespace net
