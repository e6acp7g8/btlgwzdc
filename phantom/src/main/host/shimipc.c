/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, 
 * See LICENSE for licensing information
 */

#include "main/bindings/c/bindings.h"
#include "main/core/support/config_handlers.h"
#include "main/host/shimipc.h"

static bool _useExplicitBlockMessage = true;
ADD_CONFIG_HANDLER(config_getUseExplicitBlockMessage, _useExplicitBlockMessage)
bool shimipc_sendExplicitBlockMessageEnabled() { return _useExplicitBlockMessage; }

static bool _useSeccomp = true;
ADD_CONFIG_HANDLER(config_getUseSeccomp, _useSeccomp)
bool shimipc_getUseSeccomp() { return _useSeccomp; }

static int _spinMax = -1;
ADD_CONFIG_HANDLER(config_getPreloadSpinMax, _spinMax)

ssize_t shimipc_spinMax() { return _spinMax; }
