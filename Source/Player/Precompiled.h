#pragma once

#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <windowsx.h>

#include "Windows/Res/resource.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <functional>

#include "MathHeaders.h"
#include "UtilHeaders.h"
#include "DDrawHeaders.h"

#include "eCommandType.h"
#include "Canvas.h"
#include "Event.h"
#include "Renderer.h"