/**
 * Copyright (c) 2017-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <string>
#include <vector>

#include <cetcd.h>

#include "gloo/config.h"
#include "gloo/rendezvous/store.h"

// Check that configuration header was properly generated
#if !GLOO_USE_ETCD
#error "Expected GLOO_USE_ETCD to be defined"
#endif

namespace gloo {
namespace rendezvous {

class EtcdStore : public Store {
 public:
  explicit EtcdStore(const std::vector<std::string>& hosts);
  virtual ~EtcdStore();

  virtual void set(const std::string& key, const std::vector<char>& data)
      override;

  virtual std::vector<char> get(const std::string& key) override;

  virtual void wait(const std::vector<std::string>& keys) override {
    wait(keys, Store::kDefaultTimeout);
  }

  virtual void wait(
      const std::vector<std::string>& keys,
      const std::chrono::milliseconds& timeout) override;

 protected:
  cetcd_client etcd_;
};

} // namespace rendezvous
} // namespace gloo
