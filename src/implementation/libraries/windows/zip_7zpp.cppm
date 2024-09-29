/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <memory>
#include <string>
#include <unordered_map>

// See https://github.com/getnamo/7zip-cpp
#include <7zpp/7zpp.h>

export module util_zip:zip_7zpp;

import :base_zip_file;
import :enums;

export namespace uzip {
	class SevenZipFile : public BaseZipFile {
	  public:
		static std::unique_ptr<BaseZipFile> Open(const std::string &fileName, OpenMode openMode);

		virtual ~SevenZipFile() override;
		virtual bool GetFileList(std::vector<std::string> &outFileList) const override;
		virtual bool AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite = true) override;
		virtual bool ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr) override;
		virtual bool ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &progressCallback = nullptr) override;
	  private:
		SevenZipFile() {}

		SevenZip::SevenZipLibrary lib;
		std::unique_ptr<SevenZip::SevenZipExtractor> extractor = nullptr;
		std::unique_ptr<SevenZip::SevenZipCompressor> compressor = nullptr;
		std::unordered_map<size_t, uint32_t> m_hashToIndex;
	};
};
