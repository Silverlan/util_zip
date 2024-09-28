/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <zip.h>
#include <string>
#include <memory>

module util_zip;

import :zip_libzip;

uzip::LibZipFile::LibZipFile(zip *z) : m_zip {z} {}

void uzip::LibZipFile::SetPackProgressCallback(const std::function<void(double)> &progressCallback)
{
	auto *cpy = new std::function<void(double)> {progressCallback};
	zip_register_progress_callback_with_state(
	  m_zip, 0.005, +[](zip_t *zip, double progress, void *ptr) { (*static_cast<std::function<void(double)> *>(ptr))(progress); }, +[](void *ptr) { delete ptr; }, cpy);
}

std::unique_ptr<uzip::BaseZipFile> uzip::LibZipFile::Open(const std::string &fileName, OpenMode openMode)
{
	int flags = 0;
	flags |= ZIP_CREATE;
	if(openMode == OpenMode::Read)
		flags |= ZIP_RDONLY;

	int err = -1;
	auto *z = zip_open(fileName.c_str(), flags, &err);
	if(z == nullptr)
		return nullptr;
	std::unique_ptr<LibZipFile> zipFile {new LibZipFile {z}};
	return zipFile;
}
std::unique_ptr<uzip::BaseZipFile> uzip::LibZipFile::Open(const void *zipData, size_t size)
{
	zip_error_t err;
	auto *zs = zip_source_buffer_create(zipData, size, 0, &err); // Will be released automatically
	if(!zs)
		return nullptr;
	auto *z = zip_open_from_source(zs, 0, &err);
	if(!z)
		return nullptr;
	std::unique_ptr<LibZipFile> zipFile {new LibZipFile {z}};
	return zipFile;
}
bool uzip::LibZipFile::GetFileList(std::vector<std::string> &outFileList) const
{
	auto n = zip_get_num_entries(m_zip, 0);
	outFileList.reserve(n);
	for(zip_uint64_t i = 0; i < n; ++i) {
		auto *name = zip_get_name(m_zip, i, 0); // TODO: This may be a UTF8-string!
		outFileList.push_back(name);
	}
	return true;
}
bool uzip::LibZipFile::AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite)
{
	m_data.push_back(std::make_unique<std::vector<uint8_t>>(size));
	auto &vdata = *m_data.back();
	memcpy(vdata.data(), data, size);

	auto *zipSrc = zip_source_buffer(m_zip, vdata.data(), size, 0);
	zip_flags_t flags = ZIP_FL_ENC_GUESS;
	if(bOverwrite == true)
		flags |= ZIP_FL_OVERWRITE;
	auto normalizedFileName = fileName;
	for(auto &c : normalizedFileName) {
		if(c == '\\')
			c = '/';
	}
	zip_file_add(m_zip, normalizedFileName.c_str(), zipSrc, flags);
	return true;
}
bool uzip::LibZipFile::ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr)
{
	struct zip_stat st;
	zip_stat_init(&st);
	if(zip_stat(m_zip, fileName.data(), 0, &st) != 0)
		return false;
	auto *f = zip_fopen_index(m_zip, st.index, 0);
	if(!f) {
		auto *err = zip_get_error(m_zip);
		if(err)
			outErr = std::to_string(err->zip_err);
		return false;
	}
	outData.resize(st.size);
	zip_fread(f, outData.data(), outData.size());
	return zip_fclose(f) == 0;
}
bool uzip::LibZipFile::ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &progressCallback)
{
	// TODO: Implement this
	return false;
}

uzip::LibZipFile::~LibZipFile() { zip_close(m_zip); }
