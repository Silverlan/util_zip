/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

module;

#include <string>
#include <memory>
#include <7zpp/7zpp.h>
#include <sharedutils/util_string.h>

module util_zip;

import :zip_7zpp;

static std::string program_name(bool bPost = false)
{
	std::string programPath = "";
#ifdef __linux__
	pid_t pid = getpid();
	char buf[20] = {0};
	sprintf(buf, "%d", pid);
	std::string _link = "/proc/";
	_link.append(buf);
	_link.append("/exe");
	char proc[512];
	int ch = readlink(_link.c_str(), proc, 512);
	if(ch != -1) {
		proc[ch] = 0;
		programPath = proc;
		std::string::size_type t = programPath.find_last_of("/");
		programPath = (bPost == false) ? programPath.substr(0, t) : programPath.substr(t + 1, programPath.length());
	}
#else
	char path[MAX_PATH + 1];
	GetModuleFileName(NULL, path, MAX_PATH + 1);

	programPath = path;
	auto br = programPath.rfind("\\");
	programPath = (bPost == false) ? programPath.substr(0, br) : programPath.substr(br + 1, programPath.length());
#endif
	return programPath;
}

std::unique_ptr<uzip::BaseZipFile> uzip::SevenZipFile::Open(const std::string &fileName, OpenMode openMode)
{
	auto r = std::unique_ptr<SevenZipFile> {new SevenZipFile {}};
	auto dllPath = program_name() + "\\";
#ifdef _WIN32
	std::string dllName = "7zip.dll";
#else
	// TODO: Needs to be confirmed!
	std::string dllName = "7z.so";
#endif
	if(!r->lib.Load(dllPath + dllName) && !r->lib.Load(dllPath + "bin/" + dllName))
		return nullptr;

	if(openMode == OpenMode::Read) {
		r->extractor = std::make_unique<SevenZip::SevenZipExtractor>(r->lib, fileName);
		if(!r->extractor->DetectCompressionFormat())
			return nullptr;

		std::vector<std::string> files;
		if(!r->GetFileList(files))
			return nullptr;
		for(auto i = decltype(files.size()) {0u}; i < files.size(); ++i) {
			auto &f = files[i];
			for(auto &c : f)
				c = std::tolower(c);
			auto hash = std::hash<std::string> {}(f);
			r->m_hashToIndex[hash] = i;
		}
	}
	else {
		r->compressor = std::make_unique<SevenZip::SevenZipCompressor>(r->lib, fileName);
		r->compressor->SetCompressionFormat(SevenZip::CompressionFormat::Zip);
	}
	return r;
}

uzip::SevenZipFile::~SevenZipFile() {}

bool uzip::SevenZipFile::GetFileList(std::vector<std::string> &outFileList) const
{
	if(!extractor)
		return false;
	auto itemNames = extractor->GetItemsNames();
	outFileList.reserve(itemNames.size());
	for(auto &f : itemNames)
		outFileList.push_back(ustring::wstring_to_string(f));
	return true;
}

bool uzip::SevenZipFile::AddFile(const std::string &fileName, const void *data, uint64_t size, bool bOverwrite)
{
	return false; // Not yet implemented
}

class SevenZipProgressCallback : public SevenZip::ProgressCallback {
  public:
	SevenZipProgressCallback(const std::function<bool(float, bool)> &progressCallback) : m_progress {progressCallback} {}
	/*
	Called at beginning
	*/
	virtual void OnStartWithTotal(const SevenZip::TString &archivePath, unsigned __int64 totalBytes) override { m_totalBytes = totalBytes; }

	/*
	Called Whenever progress has updated with a bytes complete
	*/
	virtual void OnProgress(const SevenZip::TString &archivePath, unsigned __int64 bytesCompleted) override { UpdateProgress(bytesCompleted); }

	/*
	Called When progress has reached 100%
	*/
	virtual void OnDone(const SevenZip::TString &archivePath) override
	{
		m_mutex.lock();
		m_complete = true;
		m_cond.notify_one();
		m_mutex.unlock();

		UpdateProgress(0, true);
	}

	/*
	Called When single file progress has reached 100%, returns the filepath that completed
	*/
	virtual void OnFileDone(const SevenZip::TString &archivePath, const SevenZip::TString &filePath, unsigned __int64 bytesCompleted) override
	{
		m_bytesExtracted += bytesCompleted;
		UpdateProgress();
	}

	/*
	Called to determine if it's time to abort the zip operation. Return true to abort the current operation.
	*/
	virtual bool OnCheckBreak() override { return m_cancel; }
	void WaitUntilComplete()
	{
		auto ul = std::unique_lock<std::mutex> {m_mutex};
		m_cond.wait(ul, [this]() -> bool { return m_complete; });
	}
  private:
	void UpdateProgress(size_t curFileBytes = 0, bool isComplete = false)
	{
		if(!m_progress)
			return;
		auto progress = (m_totalBytes > 0) ? (static_cast<double>(m_bytesExtracted + curFileBytes) / static_cast<double>(m_totalBytes)) : 0.0;
		m_cancel = m_progress(progress, isComplete);
	}
	std::atomic<bool> m_complete = false;
	std::atomic<bool> m_cancel = false;
	std::function<bool(float, bool)> m_progress = nullptr;
	uint64_t m_totalBytes = 0;
	uint64_t m_bytesExtracted = 0;

	std::condition_variable m_cond;
	std::mutex m_mutex;
};
bool uzip::SevenZipFile::ExtractFiles(const std::string &dirName, std::string &outErr, const std::function<bool(float, bool)> &fprogressCallback)
{
	if(!extractor)
		return false;
	std::vector<uint32_t> fileIndices;
	fileIndices.reserve(m_hashToIndex.size());
	for(auto &pair : m_hashToIndex)
		fileIndices.push_back(pair.second);
	SevenZipProgressCallback progressCallback {fprogressCallback};
	auto res = extractor->ExtractFilesFromArchive(fileIndices.data(), fileIndices.size(), dirName, &progressCallback);
	if(!res)
		return false;
	if(!fprogressCallback)
		progressCallback.WaitUntilComplete();
	return true;
}
bool uzip::SevenZipFile::ReadFile(const std::string &fileName, std::vector<uint8_t> &outData, std::string &outErr)
{
	if(!extractor)
		return false;
	auto lFileName = fileName;
	for(auto &c : lFileName)
		c = std::tolower(c);
	std::replace(lFileName.begin(), lFileName.end(), '/', '\\');
	auto hash = std::hash<std::string> {}(lFileName);
	auto it = m_hashToIndex.find(hash);
	if(it == m_hashToIndex.end())
		return false;
	auto index = it->second;
	SevenZipProgressCallback progressCallback {nullptr};
	auto res = extractor->ExtractFileToMemory(index, outData, &progressCallback);
	if(!res)
		return false;
	if(outData.empty())
		return false; // Probably a directory?
	progressCallback.WaitUntilComplete();
	return true;
}
