// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module util_zip:enums;

export import std.compat;

export namespace uzip {
	enum class OpenMode : uint8_t { Read = 0u, Write };
};
