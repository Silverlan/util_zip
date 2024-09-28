/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <vector>
#include <string>
#include <cinttypes>
#include <functional>

export module util_zip:base_zip_file;

export namespace uzip {
	class BaseZipFile {
	  public:
		virtual ~BaseZipFile() = default;
		virtual void Flush() {}
		virtual bool GetFileList(std::vector<std::string> &outFileList) const = 0;
		virtual bool AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite = true) = 0;
		bool AddFile(const std::string &fileName, const std::string &data, bool bOverwrite = true) { return AddFile(fileName, data.data(), data.size(), bOverwrite); }
		virtual bool ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr) = 0;
		virtual bool ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &progressCallback = nullptr) = 0;
		virtual void SetPackProgressCallback(const std::function<void(double)> &progressCallback) {}
	};
};
