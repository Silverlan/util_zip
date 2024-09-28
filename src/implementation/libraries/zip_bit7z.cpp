/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <memory>
#include <string>
#include <iostream>
#include <sharedutils/util.h>
#include <sharedutils/util_path.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchivewriter.hpp>

module util_zip;

import :zip_bit7z;

static std::string get_7z_binary_path()
{
	auto path = util::Path::CreatePath(util::get_program_path());
#ifdef _WIN32
	path += "bin/7z.dll";
#else
	path += "lib/7z.so";
#endif
	return path.GetString();
}

uzip::Bit7zFile::Bit7zFile() : lib {get_7z_binary_path()}, m_thread {1} {}
std::unique_ptr<uzip::BaseZipFile> uzip::Bit7zFile::Open(const std::string &fileName, OpenMode openMode)
{
	try {
		auto r = std::unique_ptr<Bit7zFile> {new Bit7zFile {}};
		switch(openMode) {
		case OpenMode::Read:
			{
				r->reader = std::make_unique<bit7z::BitArchiveReader>(r->lib, fileName);
				break;
			}
		case OpenMode::Write:
			{
				r->writer = std::make_unique<bit7z::BitArchiveWriter>(r->lib, bit7z::BitFormat::Zip);
				r->fileName = fileName;
				break;
			}
		}
		return r;
	}
	catch(const bit7z::BitException &e) {
		return {};
	}
	return {};
}

struct membuf : std::streambuf {
	membuf(char const *base, size_t size)
	{
		char *p(const_cast<char *>(base));
		this->setg(p, p, p + size);
	}
};
void uzip::Bit7zFile::Flush()
{
	m_thread.stop();

	if(writer) {
		try {
			writer->compressTo(fileName);
		}
		catch(const bit7z::BitException &e) {
			std::cout << "Failed to compress zip-file: " << e.what() << std::endl;
		}
	}
}
uzip::Bit7zFile::~Bit7zFile() {}

bool uzip::Bit7zFile::GetFileList(std::vector<std::string> &outFileList) const
{
	if(!reader)
		return false;
	outFileList.reserve(reader->itemsCount());
	for(auto &item : *reader)
		outFileList.push_back(item.name());
	return true;
}

bool uzip::Bit7zFile::AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite)
{
	if(!writer)
		return false;
	membuf buf {static_cast<const char *>(data), size};
	std::istream istr {&buf};
	writer->addFile(istr, fileName);
	return true;
}

void uzip::Bit7zFile::SetPackProgressCallback(const std::function<void(double)> &progressCallback) { m_progressCallback = progressCallback; }

bool uzip::Bit7zFile::ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &fprogressCallback)
{
	if(!reader)
		return false;
	auto numFiles = reader->filesCount();
	if(numFiles == 0) {
		if(fprogressCallback)
			fprogressCallback(1.f, true);
		return true;
	}
	auto extract = [this, dirName, fprogressCallback]() {
		std::uint64_t totalSize = 0;
		if(fprogressCallback) {
			reader->setTotalCallback([&](std::uint64_t total) { totalSize = total; });
			reader->setProgressCallback([this, fprogressCallback, &totalSize](uint64_t progress) -> bool {
				auto fprogress = progress / static_cast<float>(totalSize);
				auto complete = (fprogress >= 1.f);
				if(m_progressCallback)
					m_progressCallback(fprogress);
				return fprogressCallback(fprogress, complete);
			});
		}
		reader->extract(dirName);
	};
	if(fprogressCallback)
		m_thread.push([extract = std::move(extract)](int id) { extract(); });
	else
		extract();
	return true;
}
bool uzip::Bit7zFile::ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr)
{
	if(!reader) {
		outErr = "Zip-archive has not been opened for reading!";
		return false;
	}
	auto it = reader->find(fileName);
	if(it == reader->end()) {
		outErr = "File not found!";
		return false;
	}
	auto &item = *it;
	outData.resize(item.size());
	reader->extract(outData.data(), outData.size(), item.index());
	return true;
}
