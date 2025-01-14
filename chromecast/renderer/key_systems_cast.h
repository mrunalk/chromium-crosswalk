// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_RENDERER_KEY_SYSTEMS_CAST_H_
#define CHROMECAST_RENDERER_KEY_SYSTEMS_CAST_H_

#include <memory>
#include <vector>

namespace media {
class KeySystemProperties;
}

namespace chromecast {
namespace shell {

void AddChromecastKeySystems(
    std::vector<std::unique_ptr<::media::KeySystemProperties>>*
        key_systems_properties);

}  // namespace shell
}  // namespace chromecast

#endif  // CHROMECAST_RENDERER_KEY_SYSTEMS_CAST_H_
