// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <memory>
#include <string>
#include <vector>
#include <functional>

export module util_zip:zipfile;

import :enums;

#undef ReplaceFile

export namespace uzip {
	class BaseZipFile;
	class ZIPFile {
	  public:
		static std::unique_ptr<ZIPFile> Open(const std::string &filePath, std::string &outErr, OpenMode openFlags = OpenMode::Read);
		static std::unique_ptr<ZIPFile> Open(const void *zipData, size_t size, std::string &outErr);
		~ZIPFile();
		bool AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite = true);
		bool AddFile(const std::string &fileName, const std::string &data, bool bOverwrite = true);
		bool ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr);
		bool GetFileList(std::vector<std::string> &outFileList);
		bool ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &progressCallback = nullptr);
		void SetPackProgressCallback(const std::function<void(double)> &progressCallback);
	  private:
		ZIPFile(std::unique_ptr<BaseZipFile> baseZipFile);
		std::unique_ptr<BaseZipFile> m_baseZipFile;
	};
};
