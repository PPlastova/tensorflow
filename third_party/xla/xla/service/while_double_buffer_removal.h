/* Copyright 2024 The OpenXLA Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef XLA_SERVICE_WHILE_DOUBLE_BUFFER_REMOVAL_H_
#define XLA_SERVICE_WHILE_DOUBLE_BUFFER_REMOVAL_H_

#include <functional>
#include <utility>

#include "xla/hlo/ir/hlo_module.h"
#include "xla/service/hlo_pass_interface.h"
#include "xla/statusor.h"

namespace xla {

// This pass recognizes the following patterns in nested loops:
// TODO: write the pattern
// The buffer must be allocated inside the outer loop in order to be replaced.
class WhileDoubleBufferRemoval : public HloModulePass {
 public:
  ~WhileDoubleBufferRemoval() override = default;

  // Default unroll_factor of -1 indicates full unrolling
  explicit WhileDoubleBufferRemoval(int64_t module_id)
      : module_id_(module_id) {}

  absl::string_view name() const override {
    return "while-double-buffer-removal";
  }
  using HloPassInterface::Run;
  absl::StatusOr<bool> Run(
      HloModule* module,
      const absl::flat_hash_set<absl::string_view>& execution_threads) override;

  const int64_t module_id_;
};

}  // namespace xla

#endif  // XLA_SERVICE_WHILE_DOUBLE_BUFFER_REMOVAL_H_
