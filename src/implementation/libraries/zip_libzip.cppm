// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <zip.h>

export module util_zip:zip_libzip;

export import :base_zip_file;
export import :enums;

export namespace uzip {
	class LibZipFile : public BaseZipFile {
	  public:
		static std::unique_ptr<BaseZipFile> Open(const std::string &fileName, OpenMode openMode);
		static std::unique_ptr<BaseZipFile> Open(const void *zipData, size_t size);
		virtual ~LibZipFile() override;
		virtual bool GetFileList(std::vector<std::string> &outFileList) const override;
		virtual bool AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite = true) override;
		virtual bool ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr) override;
		virtual bool ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &progressCallback = nullptr) override;
		virtual void SetPackProgressCallback(const std::function<void(double)> &progressCallback) override;
	  private:
		LibZipFile(zip *z);
		zip *m_zip;
		std::vector<std::unique_ptr<std::vector<uint8_t>>> m_data;
	};
};
