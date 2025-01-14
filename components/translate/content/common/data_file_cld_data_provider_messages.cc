// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// NOT DEAD CODE!
// This code isn't dead, even if it isn't currently being used. Please refer to:
// https://www.chromium.org/developers/how-tos/compact-language-detector-cld-data-source-configuration

// Get basic type definitions.
#define IPC_MESSAGE_IMPL
#include "components/translate/content/common/data_file_cld_data_provider_messages.h"

// Generate constructors.
#include "ipc/struct_constructor_macros.h"
#include "components/translate/content/common/data_file_cld_data_provider_messages.h"

// Generate destructors.
#include "ipc/struct_destructor_macros.h"
#include "components/translate/content/common/data_file_cld_data_provider_messages.h"

// Generate param traits size methods.
#include "ipc/param_traits_size_macros.h"
namespace IPC {
#include "components/translate/content/common/data_file_cld_data_provider_messages.h"
}  // namespace IPC

// Generate param traits write methods.
#include "ipc/param_traits_write_macros.h"
namespace IPC {
#include "components/translate/content/common/data_file_cld_data_provider_messages.h"
}  // namespace IPC

// Generate param traits read methods.
#include "ipc/param_traits_read_macros.h"
namespace IPC {
#include "components/translate/content/common/data_file_cld_data_provider_messages.h"
}  // namespace IPC

// Generate param traits log methods.
#include "ipc/param_traits_log_macros.h"
namespace IPC {
#include "components/translate/content/common/data_file_cld_data_provider_messages.h"
}  // namespace IPC
