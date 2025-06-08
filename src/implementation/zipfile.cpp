/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <cstring>
#include <algorithm>
#include <sharedutils/util_string.h>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include "definitions.hpp"

module util_zip;

import :zip_libzip;
#ifdef _WIN32
import :zip_7zpp;
#endif
import :zip_bit7z;

/////////////

std::unique_ptr<uzip::ZIPFile> uzip::ZIPFile::Open(const void *zipData, size_t size, std::string &outErr)
{
	auto baseZip = LibZipFile::Open(zipData, size);
	if(!baseZip)
		return nullptr;
	return std::unique_ptr<uzip::ZIPFile>(new uzip::ZIPFile(std::move(baseZip)));
}
std::unique_ptr<uzip::ZIPFile> uzip::ZIPFile::Open(const std::string &filePath, std::string &outErr, OpenMode openMode)
{
	switch(openMode) {
	case OpenMode::Read:
		{
#if ZIP_READ_LIB == ZIP_LIB_LIBZIP
			auto baseZip = LibZipFile::Open(filePath, openMode);
			if(!baseZip)
				return {};
			return std::unique_ptr<uzip::ZIPFile>(new uzip::ZIPFile(std::move(baseZip)));
#elif ZIP_READ_LIB == ZIP_LIB_7ZPP
			auto baseZip = SevenZipFile::Open(filePath, openMode);
			if(!baseZip)
				return {};
			return std::unique_ptr<uzip::ZIPFile>(new uzip::ZIPFile(std::move(baseZip)));
#elif ZIP_READ_LIB == ZIP_LIB_BIT7Z
			auto baseZip = Bit7zFile::Open(filePath, outErr, openMode);
			if(!baseZip)
				return {};
			return std::unique_ptr<uzip::ZIPFile>(new uzip::ZIPFile(std::move(baseZip)));
#endif
			break;
		}
	case OpenMode::Write:
		{
#if ZIP_WRITE_LIB == ZIP_LIB_LIBZIP
			auto baseZip = LibZipFile::Open(filePath, openMode);
			if(!baseZip)
				return {};
			return std::unique_ptr<uzip::ZIPFile>(new uzip::ZIPFile(std::move(baseZip)));
#elif ZIP_WRITE_LIB == ZIP_LIB_7ZPP
			auto baseZip = SevenZipFile::Open(filePath, openMode);
			if(!baseZip)
				return {};
			return std::unique_ptr<uzip::ZIPFile>(new uzip::ZIPFile(std::move(baseZip)));
#elif ZIP_WRITE_LIB == ZIP_LIB_BIT7Z
			auto baseZip = Bit7zFile::Open(filePath, outErr, openMode);
			if(!baseZip)
				return {};
			return std::unique_ptr<uzip::ZIPFile>(new uzip::ZIPFile(std::move(baseZip)));
#endif
			break;
		}
	}
	return {};
}

uzip::ZIPFile::ZIPFile(std::unique_ptr<BaseZipFile> baseZipFile) : m_baseZipFile(std::move(baseZipFile)) {}

uzip::ZIPFile::~ZIPFile()
{
	m_baseZipFile->Flush();
	m_baseZipFile = nullptr;
}

bool uzip::ZIPFile::ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &progressCallback) { return m_baseZipFile->ExtractFiles(dirName, outErr, progressCallback); }

void uzip::ZIPFile::SetPackProgressCallback(const std::function<void(double)> &progressCallback) { m_baseZipFile->SetPackProgressCallback(progressCallback); }

bool uzip::ZIPFile::GetFileList(std::vector<std::string> &outFileList) { return m_baseZipFile->GetFileList(outFileList); }

bool uzip::ZIPFile::ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr) { return m_baseZipFile->ReadFile(fileName, outData, outErr); }

bool uzip::ZIPFile::AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite) { return m_baseZipFile->AddFile(fileName, data, size, bOverwrite); }

bool uzip::ZIPFile::AddFile(const std::string &fileName, const std::string &data, bool bOverwrite) { return AddFile(fileName, data.data(), data.size(), bOverwrite); }
