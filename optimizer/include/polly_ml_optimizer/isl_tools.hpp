#pragma once

#include "isl/ctx.h"

#include <memory>

struct IslCtxDeleter {
  void operator()(isl_ctx* isl_ctxt) { isl_ctx_free(isl_ctxt); }
};

using IslCtxUPtr = std::unique_ptr<isl_ctx, IslCtxDeleter>;
