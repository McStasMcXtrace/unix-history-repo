//===-- CoreMedia.h -----------------------------------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_CoreMedia_h_
#define liblldb_CoreMedia_h_

#include "lldb/Core/ValueObject.h"
#include "lldb/DataFormatters/TypeSummary.h"
#include "lldb/Utility/Stream.h"

namespace lldb_private {
namespace formatters {

bool CMTimeSummaryProvider(ValueObject &valobj, Stream &stream,
                           const TypeSummaryOptions &options);
} // namespace formatters
} // namespace lldb_private

#endif // liblldb_CF_h_
