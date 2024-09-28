/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <cinttypes>

export module util_zip:enums;

export namespace uzip {
	enum class OpenMode : uint8_t { Read = 0u, Write };
};
