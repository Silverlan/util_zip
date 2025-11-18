// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitarchivewriter.hpp>

export module util_zip:zip_bit7z;

export import :base_zip_file;
export import :enums;
export import pragma.util;

export namespace uzip {
	class Bit7zFile : public BaseZipFile {
	  public:
		static std::unique_ptr<BaseZipFile> Open(const std::string &fileName, std::string &outErr, OpenMode openMode);

		virtual ~Bit7zFile() override;
		virtual bool GetFileList(std::vector<std::string> &outFileList) const override;
		virtual bool AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite = true) override;
		virtual bool ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr) override;
		virtual bool ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &progressCallback = nullptr) override;
		virtual void SetPackProgressCallback(const std::function<void(double)> &progressCallback) override;
		virtual void Flush() override;
	  private:
		Bit7zFile();

		std::string fileName;
		bit7z::Bit7zLibrary lib;
		std::unique_ptr<bit7z::BitArchiveReader> reader;
		std::unique_ptr<bit7z::BitArchiveWriter> writer;

		std::function<void(double)> m_progressCallback;
		BS::light_thread_pool m_thread;
		std::atomic<bool> m_cancelled = false;
		std::unordered_map<std::string, std::string> m_normalizedNameToInternal;
	};
};
