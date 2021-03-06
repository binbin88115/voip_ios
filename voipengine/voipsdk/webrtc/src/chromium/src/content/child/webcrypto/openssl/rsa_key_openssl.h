// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CHILD_WEBCRYPTO_OPENSSL_RSA_KEY_OPENSSL_H_
#define CONTENT_CHILD_WEBCRYPTO_OPENSSL_RSA_KEY_OPENSSL_H_

#include "content/child/webcrypto/algorithm_implementation.h"

namespace content {

namespace webcrypto {

// Base class for an RSA algorithm whose keys additionaly have a hash parameter
// bound to them. Provides functionality for generating, importing, and
// exporting keys.
class RsaHashedAlgorithm : public AlgorithmImplementation {
 public:
  // |all_public_key_usages| and |all_private_key_usages| are the set of
  // WebCrypto key usages that are valid for created keys (public and private
  // respectively).
  //
  // For instance if public keys support encryption and wrapping, and private
  // keys support decryption and unwrapping callers should set:
  //    all_public_key_usages = UsageEncrypt | UsageWrap
  //    all_private_key_usages = UsageDecrypt | UsageUnwrap
  // This information is used when importing or generating keys, to enforce
  // that valid key usages are allowed.
  RsaHashedAlgorithm(blink::WebCryptoKeyUsageMask all_public_key_usages,
                     blink::WebCryptoKeyUsageMask all_private_key_usages)
      : all_public_key_usages_(all_public_key_usages),
        all_private_key_usages_(all_private_key_usages) {}

  // For instance "RSA-OAEP-256".
  virtual const char* GetJwkAlgorithm(
      const blink::WebCryptoAlgorithmId hash) const = 0;

  Status GenerateKey(const blink::WebCryptoAlgorithm& algorithm,
                     bool extractable,
                     blink::WebCryptoKeyUsageMask usages,
                     GenerateKeyResult* result) const override;

  Status VerifyKeyUsagesBeforeImportKey(
      blink::WebCryptoKeyFormat format,
      blink::WebCryptoKeyUsageMask usages) const override;

  Status ImportKeyPkcs8(const CryptoData& key_data,
                        const blink::WebCryptoAlgorithm& algorithm,
                        bool extractable,
                        blink::WebCryptoKeyUsageMask usages,
                        blink::WebCryptoKey* key) const override;

  Status ImportKeySpki(const CryptoData& key_data,
                       const blink::WebCryptoAlgorithm& algorithm,
                       bool extractable,
                       blink::WebCryptoKeyUsageMask usages,
                       blink::WebCryptoKey* key) const override;

  Status ImportKeyJwk(const CryptoData& key_data,
                      const blink::WebCryptoAlgorithm& algorithm,
                      bool extractable,
                      blink::WebCryptoKeyUsageMask usages,
                      blink::WebCryptoKey* key) const override;

  Status ExportKeyPkcs8(const blink::WebCryptoKey& key,
                        std::vector<uint8_t>* buffer) const override;

  Status ExportKeySpki(const blink::WebCryptoKey& key,
                       std::vector<uint8_t>* buffer) const override;

  Status ExportKeyJwk(const blink::WebCryptoKey& key,
                      std::vector<uint8_t>* buffer) const override;

  Status SerializeKeyForClone(
      const blink::WebCryptoKey& key,
      blink::WebVector<uint8_t>* key_data) const override;

  Status DeserializeKeyForClone(const blink::WebCryptoKeyAlgorithm& algorithm,
                                blink::WebCryptoKeyType type,
                                bool extractable,
                                blink::WebCryptoKeyUsageMask usages,
                                const CryptoData& key_data,
                                blink::WebCryptoKey* key) const override;

 private:
  blink::WebCryptoKeyUsageMask all_public_key_usages_;
  blink::WebCryptoKeyUsageMask all_private_key_usages_;
};

}  // namespace webcrypto

}  // namespace content

#endif  // CONTENT_CHILD_WEBCRYPTO_OPENSSL_RSA_KEY_OPENSSL_H_
