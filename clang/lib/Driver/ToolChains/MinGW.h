//===--- MinGW.h - MinGW ToolChain Implementations --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_MINGW_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_MINGW_H

#include "Cuda.h"
#include "Gnu.h"
#include "ROCm.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"
#include "llvm/Support/ErrorOr.h"

namespace clang {
namespace driver {
namespace tools {

/// MinGW -- Directly call GNU Binutils assembler and linker
namespace MinGW {
class LLVM_LIBRARY_VISIBILITY Assembler : public Tool {
public:
  Assembler(const ToolChain &TC) : Tool("MinGW::Assemble", "assembler", TC) {}

  bool hasIntegratedCPP() const override { return false; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
};

class LLVM_LIBRARY_VISIBILITY Linker : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("MinGW::Linker", "linker", TC) {}

  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }

  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;

private:
  void AddLibGCC(const llvm::opt::ArgList &Args,
                 llvm::opt::ArgStringList &CmdArgs) const;
};
} // end namespace MinGW
} // end namespace tools

namespace toolchains {

class LLVM_LIBRARY_VISIBILITY MinGW : public ToolChain {
public:
  MinGW(const Driver &D, const llvm::Triple &Triple,
        const llvm::opt::ArgList &Args);

  bool HasNativeLLVMSupport() const override;

  bool IsIntegratedAssemblerDefault() const override;
  bool IsUnwindTablesDefault(const llvm::opt::ArgList &Args) const override;
  bool isPICDefault() const override;
  bool isPIEDefault() const override;
  bool isPICDefaultForced() const override;

  SanitizerMask getSupportedSanitizers() const override;

  llvm::ExceptionHandling GetExceptionModel(
      const llvm::opt::ArgList &Args) const override;

  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;
  void AddClangCXXStdlibIncludeArgs(
      const llvm::opt::ArgList &DriverArgs,
      llvm::opt::ArgStringList &CC1Args) const override;

  void AddCudaIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                          llvm::opt::ArgStringList &CC1Args) const override;
  void AddHIPIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                         llvm::opt::ArgStringList &CC1Args) const override;

  void printVerboseInfo(raw_ostream &OS) const override;

protected:
  Tool *getTool(Action::ActionClass AC) const override;
  Tool *buildLinker() const override;
  Tool *buildAssembler() const override;

private:
  CudaInstallationDetector CudaInstallation;
  RocmInstallationDetector RocmInstallation;

  std::string Base;
  std::string GccLibDir;
  std::string Ver;
  std::string Arch;
  mutable std::unique_ptr<tools::gcc::Preprocessor> Preprocessor;
  mutable std::unique_ptr<tools::gcc::Compiler> Compiler;
  void findGccLibDir();
  llvm::ErrorOr<std::string> findGcc();
  llvm::ErrorOr<std::string> findClangRelativeSysroot();

  bool NativeLLVMSupport;
};

} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_MINGW_H
