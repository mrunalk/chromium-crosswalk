// Copyright 2016 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "snapshot/win/module_snapshot_win.h"

#include <stdlib.h>
#include <string.h>

#include <string>

#include "base/files/file_path.h"
#include "build/build_config.h"
#include "client/crashpad_info.h"
#include "client/simple_address_range_bag.h"
#include "gtest/gtest.h"
#include "snapshot/win/process_snapshot_win.h"
#include "test/paths.h"
#include "test/win/child_launcher.h"
#include "util/file/file_io.h"
#include "util/win/process_info.h"

namespace crashpad {
namespace test {
namespace {

enum TestType {
  // Don't crash, just test the CrashpadInfo interface.
  kDontCrash = 0,

  // The child process should crash by __debugbreak().
  kCrashDebugBreak,
};

void TestExtraMemoryRanges(TestType type,
                           const base::string16& directory_modification) {
  // Spawn a child process, passing it the pipe name to connect to.
  base::FilePath test_executable = Paths::Executable();
  std::wstring child_test_executable =
      test_executable.DirName()
          .Append(directory_modification)
          .Append(test_executable.BaseName().RemoveFinalExtension().value() +
                  L"_extra_memory_ranges.exe")
          .value();
  ChildLauncher child(child_test_executable, L"");
  child.Start();

  // Wait for the child process to indicate that it's done setting up its
  // annotations via the CrashpadInfo interface.
  char c;
  CheckedReadFile(child.stdout_read_handle(), &c, sizeof(c));

  ProcessSnapshotWin snapshot;
  ASSERT_TRUE(snapshot.Initialize(
      child.process_handle(), ProcessSuspensionState::kRunning, 0));

  // Verify the extra memory ranges set via the CrashpadInfo interface.
  std::set<CheckedRange<uint64_t>> all_ranges;
  for (const auto* module : snapshot.Modules()) {
    for (const auto& range : module->ExtraMemoryRanges())
      all_ranges.insert(range);
  }

  EXPECT_EQ(5u, all_ranges.size());
  EXPECT_NE(all_ranges.end(), all_ranges.find(CheckedRange<uint64_t>(0, 1)));
  EXPECT_NE(all_ranges.end(), all_ranges.find(CheckedRange<uint64_t>(1, 0)));
  EXPECT_NE(all_ranges.end(),
            all_ranges.find(CheckedRange<uint64_t>(1234, 5678)));
  EXPECT_NE(all_ranges.end(),
            all_ranges.find(CheckedRange<uint64_t>(0x1000000000ULL, 0x1000)));
  EXPECT_NE(all_ranges.end(),
            all_ranges.find(CheckedRange<uint64_t>(0x2000, 0x2000000000ULL)));

  // Tell the child process to continue.
  DWORD expected_exit_code;
  switch (type) {
    case kDontCrash:
      c = ' ';
      expected_exit_code = 0;
      break;
    case kCrashDebugBreak:
      c = 'd';
      expected_exit_code = STATUS_BREAKPOINT;
      break;
    default:
      FAIL();
  }
  CheckedWriteFile(child.stdin_write_handle(), &c, sizeof(c));

  EXPECT_EQ(expected_exit_code, child.WaitForExit());
}

TEST(ExtraMemoryRanges, DontCrash) {
  TestExtraMemoryRanges(kDontCrash, FILE_PATH_LITERAL("."));
}

TEST(ExtraMemoryRanges, CrashDebugBreak) {
  TestExtraMemoryRanges(kCrashDebugBreak, FILE_PATH_LITERAL("."));
}

#if defined(ARCH_CPU_64_BITS)
TEST(ExtraMemoryRanges, DontCrashWOW64) {
#ifndef NDEBUG
  TestExtraMemoryRanges(kDontCrash, FILE_PATH_LITERAL("..\\..\\out\\Debug"));
#else
  TestExtraMemoryRanges(kDontCrash, FILE_PATH_LITERAL("..\\..\\out\\Release"));
#endif
}

TEST(ExtraMemoryRanges, CrashDebugBreakWOW64) {
#ifndef NDEBUG
  TestExtraMemoryRanges(kCrashDebugBreak,
                        FILE_PATH_LITERAL("..\\..\\out\\Debug"));
#else
  TestExtraMemoryRanges(kCrashDebugBreak,
                        FILE_PATH_LITERAL("..\\..\\out\\Release"));
#endif
}
#endif  // ARCH_CPU_64_BITS

}  // namespace
}  // namespace test
}  // namespace crashpad
