/**
 * Copyright (c) 2017-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "gloo/rendezvous/etcd_store.h"

#include <cstring>
#include <thread>

#include "gloo/common/error.h"
#include "gloo/common/logging.h"
#include "gloo/common/string.h"

#include <iostream>
#include <cstring>

namespace gloo {
namespace rendezvous {

static const std::chrono::seconds kWaitTimeout = std::chrono::seconds(60);

EtcdStore::EtcdStore(const std::vector<std::string>& hosts) {
  cetcd_array_init(&addrs_, hosts.size() * 10);
  for (auto host : hosts) {
    char* addr = (char*)malloc(sizeof(char)*host.size());
    ::strncpy(addr, host.c_str(), host.size());
    std::cout << host << " -> " << addr << std::endl;
    cetcd_array_append(&addrs_, addr);
  }
  cetcd_client_init(&etcd_, &addrs_);
  /*  
  struct timeval timeout = {.tv_sec = 2};
  etcd_ = etcdConnectWithTimeout(host.c_str(), port, timeout);
  GLOO_ENFORCE(etcd_ != nullptr);
  if (etcd_->err != 0) {
    GLOO_THROW_IO_EXCEPTION("Connecting to Etcd: ", etcd_->errstr);
  }
  */
}

EtcdStore::~EtcdStore() {
  std::cerr << "destroy~~~~" << std::endl;
  cetcd_client_destroy(&etcd_);
  cetcd_array_destroy(&addrs_);
}

void EtcdStore::set(const std::string& key, const std::vector<char>& data) {
  std::cerr << __LINE__ << "trying to set " << key << std::endl;
  CURL * curl = curl_easy_init();
  if (!curl) {
    // throw exception here
    std::cerr << __LINE__ << "cannot alloc curl" << key << std::endl;
    return;
  }
  char value[data.size()];
  for (size_t s = 0; s < data.size(); s++) {
    value[s] = data[s];
  }
  char* tmp = curl_easy_escape(curl, value, data.size());
  
  if (!tmp) {
    std::cerr << __LINE__ << "cannot escape value at " << key << std::endl;
    // throw exception here
    return;
    //GLOO_THROW_IO_EXCEPTION(etcd_->errstr);
  }

  std::cout << "key:" << key << "\turl-encoded: " << tmp << std::endl;
  cetcd_response * res = cetcd_set(&etcd_, key.c_str(), tmp, 0);

  if (res->err) {
    printf("error :%d, %s (%s)\n", res->err->ecode, res->err->message,
           res->err->cause);
    GLOO_THROW_IO_EXCEPTION("Error: ", res->err->message);
  }
  
  curl_free(tmp);
  curl_easy_cleanup(curl);
  cetcd_response_release(res);
  /*
  etcdReply* reply = static_cast<etcdReply*>(ptr);
  if (reply->type == ETCD_REPLY_ERROR) {
    GLOO_THROW_IO_EXCEPTION("Error: ", reply->str);
  }
  GLOO_ENFORCE_EQ(reply->type, ETCD_REPLY_INTEGER);
  GLOO_ENFORCE_EQ(reply->integer, 1, "Key '", key, "' already set");
  freeReplyObject(reply);
  */
}

std::vector<char> EtcdStore::get(const std::string& key) {
  // Block until key is set
  std::cerr << __LINE__ << "trying to get " << key << std::endl;
  wait({key});

  cetcd_response * res = cetcd_get(&etcd_, key.c_str());

  std::vector<char> ret;
  if (res->err) {
    printf("error :%d, %s (%s)\n", res->err->ecode, res->err->message,
           res->err->cause);
  } else {
    if (res->node) {
      CURL * curl = curl_easy_init();
      printf("key:%s\n", res->node->key);

      int len = strlen(res->node->value);
      int outlen;
      char * value = curl_easy_unescape(curl, res->node->value, len, &outlen);
      printf("value:%s\n", value);
      for (char* p = value; *p != '\0'; p++) {
        // Buffer overflow'ish code><
        ret.push_back(*p);
      }
      curl_free(value);
      curl_easy_cleanup(curl);
    }
  }
  cetcd_response_release(res);
  return ret;
  /*
  // Get value
  void* ptr = etcdCommand(etcd_, "GET %b", key.c_str(), (size_t)key.size());
  if (ptr == nullptr) {
    GLOO_THROW_IO_EXCEPTION(etcd_->errstr);
  }
  etcdReply* reply = static_cast<etcdReply*>(ptr);
  if (reply->type == ETCD_REPLY_ERROR) {
    GLOO_THROW_IO_EXCEPTION("Error: ", reply->str);
  }
  GLOO_ENFORCE_EQ(reply->type, ETCD_REPLY_STRING);
  std::vector<char> result(reply->str, reply->str + reply->len);
  freeReplyObject(reply);
  return result;
  */
}

void EtcdStore::wait(
    const std::vector<std::string>& keys,
    const std::chrono::milliseconds& timeout) {
  // Polling is fine for the typical rendezvous use case, as it is
  // only done at initialization time and  not at run time.
  const auto start = std::chrono::steady_clock::now();
  for ( auto key : keys ) {
    std::cerr << __LINE__ << "trying to wait for " << key << std::endl;
    cetcd_response * res = cetcd_watch(&etcd_, key.c_str(), 2345);
    cetcd_response_print(res);
    cetcd_response_release(res);
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start);
    if (timeout != kNoTimeout && elapsed > timeout) {
      GLOO_THROW_IO_EXCEPTION(GLOO_ERROR_MSG(
          "Wait timeout for key(s): ", ::gloo::MakeString(keys)));
    }
  }
  /*
  while (!check(keys)) {
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start);
    if (timeout != kNoTimeout && elapsed > timeout) {
      GLOO_THROW_IO_EXCEPTION(GLOO_ERROR_MSG(
          "Wait timeout for key(s): ", ::gloo::MakeString(keys)));
    }
    // sleep override 
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  */
}

} // namespace rendezvous
} // namespace gloo
