/* Copyright 2022 The OpenXLA Authors.

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

#include <iostream>
#include <string>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/string_view.h"
#include "xla/status.h"
#include "xla/tools/xla_compile_lib.h"
#include "xla/tsl/util/command_line_flags.h"
#include "tsl/platform/init_main.h"
#include "tsl/platform/types.h"

namespace xla {
namespace xla_compile {

const char kUsageHeader[] =
    "xla_compile performs ahead-of-time compilation of an MHLO, StableHLO or "
    "HLO module,\nresulting in an AotCompilationResult compiled for CPU or GPU."
    "\n"
    "A typical invocation looks like this:\n"
    "\n"
    "   $ xla_compile --module_file=mymodule.mlir --output_file=output "
    "--platform=cpu"
    "\n"
    "For GPU, either the attached GPU or a simulated one may be used. To use "
    "a simulated device, set --gpu_target_config to a textproto file "
    "containing a GpuTargetConfigProto for the device you wish to simulate. To "
    "use the attached GPU, do not set this flag. When compiling with the "
    "attached device, --output_file will contain a text-format HLO module "
    "instead of an AotCompilationResult."
    "\n"
    "HLO may also be looked up in a symbol repository (see symbol_repository.h"
    ") by passing --symbol_repository to a linked-in symbol repository "
    "implementation and setting --symbol_reference to a reference of a symbol "
    "understood by that repository."
    "\n";

}  // end namespace xla_compile
}  // end namespace xla

// Read the input file containing the MHLO module, and write a Serialized
// AotCompilationResult or Executable to the output file.
int main(int argc, char* argv[]) {
  std::string module_path;
  std::string output_path;
  std::string platform;
  std::string gpu_target_config_path;
  std::string autotune_results_path;
  std::string symbol_repository;
  std::string symbol_id;
  std::string optimized_symbol_id;
  bool use_attached_device = false;
  bool wait_for_uploads = false;
  std::string result_output_file;
  std::vector<tsl::Flag> flag_list = {
      tsl::Flag("module_file", &module_path,
                "The path to the HLO, MHLO or StableHLO file"),
      tsl::Flag("output_file", &output_path, "The path to the output file"),
      tsl::Flag("platform", &platform,
                "The platform on which the built executable runs"),
      tsl::Flag("gpu_target_config", &gpu_target_config_path,
                "The path to a text-format GpuTargetConfig. If not provided, "
                "an attached GPU will be used."),
      tsl::Flag("autotune_results", &autotune_results_path,
                "The path to AutotuneResults, optional when compiling for"
                " GPU"),
      tsl::Flag("symbol_repo", &symbol_repository,
                "Which SymbolRepository to look up --symbol_reference in. If "
                "the repository contains a GpuTargetConfig, "
                "--gpu_target_config will take precedence if it is also set."),
      tsl::Flag("symbol_reference", &symbol_id,
                "Symbol ID to look up in a SymbolRepository. Overrides "
                "--module_file."),
      tsl::Flag(
          "optimized_symbol_reference", &symbol_id,
          "Optimized symbol ID to look up in a SymbolRepository. Overrides "
          "--autotune_results_path."),

      tsl::Flag("use_attached_device", &use_attached_device,
                "Whether to use the attached GPU or not. Overrides the "
                "AOT-vs-device-backed inference based on the presence of "
                "--gpu_target_config, which is relevant when a GpuTargetConfig "
                "can be found in the symbol repository."),
      tsl::Flag("wait_for_uploads", &wait_for_uploads,
                "Whether to wait for uploads to a symbol repository to "
                "complete. See export_hlo.h for more on uploads."),
      tsl::Flag("result_output_file", &result_output_file,
                "File to write a serialized xla.CompilationResult proto to."),
  };

  tsl::string usage = xla::xla_compile::kUsageHeader;
  usage += tsl::Flags::Usage(argv[0], flag_list);
  if (argc > 1 && absl::string_view(argv[1]) == "--help") {
    std::cerr << usage << "\n";
    return 0;
  }

  bool parsed_flags_ok = tsl::Flags::Parse(&argc, argv, flag_list);
  QCHECK(parsed_flags_ok) << "\n" << usage;

  tsl::port::InitMain(usage.c_str(), &argc, &argv);

  xla::XlaCompileOptions options;
  options.module_path = module_path;
  options.output_path = output_path;
  options.platform = platform;
  options.result_output_file = result_output_file;

  options.repo_options.symbol_repo = symbol_repository;
  options.repo_options.symbol_id = symbol_id;
  options.repo_options.optimized_symbol_id = optimized_symbol_id;
  options.repo_options.wait_for_uploads = wait_for_uploads;

  options.gpu_options.gpu_target_config_path = gpu_target_config_path;
  options.gpu_options.use_attached_device = use_attached_device;
  options.gpu_options.autotune_results_path = autotune_results_path;

  absl::Status result = xla::XlaCompileMain(options);
  if (!result.ok()) {
    LOG(ERROR) << "Compilation failed: " << result;
    return 1;
  }

  LOG(INFO) << "Compilation succeeded";
  return 0;
}
