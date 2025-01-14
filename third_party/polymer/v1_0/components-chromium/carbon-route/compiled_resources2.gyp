# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# NOTE: Created with generate_compiled_resources_gyp.py, please do not edit.
{
  'targets': [
    {
      'target_name': 'carbon-location-extracted',
      'dependencies': [
        '../iron-location/compiled_resources2.gyp:iron-location-extracted',
        '../iron-location/compiled_resources2.gyp:iron-query-params-extracted',
        'carbon-route-converter-extracted',
      ],
      'includes': ['../../../../closure_compiler/compile_js2.gypi'],
    },
    {
      'target_name': 'carbon-route-converter-extracted',
      'includes': ['../../../../closure_compiler/compile_js2.gypi'],
    },
    {
      'target_name': 'carbon-route-extracted',
      'includes': ['../../../../closure_compiler/compile_js2.gypi'],
    },
  ],
}
