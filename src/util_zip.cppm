// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module util_zip;
export import :enums;
export import :zipfile;
import :zip_libzip;
#ifdef _WIN32
import :zip_7zpp;
#endif
import :zip_bit7z;
