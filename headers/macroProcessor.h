#pragma once

#include "lispContext.h"
#include "parser.h"

// Macroprocess whole tree recursevely
void macroProcess(context *ctx, node *tree);
