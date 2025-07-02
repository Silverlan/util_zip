// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>

export module util_zip:enums;

export namespace uzip {
	enum class OpenMode : uint8_t { Read = 0u, Write };
};
