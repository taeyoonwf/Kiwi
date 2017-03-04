#pragma once

#include "KForm.h"
#include "KTrie.h"
#include "KModelMgr.h"

typedef pair<wstring, KPOSTag> KWordPair;
typedef pair<vector<KWordPair>, float> KResult;

class Kiwi
{
protected:
	shared_ptr<KModelMgr> mdl;
	shared_ptr<KTrie> kt;
	static KPOSTag identifySpecialChr(wchar_t chr);
	static vector<vector<KWordPair>> splitPart(const wstring& str);
	void enumPossible(KPOSTag prefixTag, const vector<KChunk>& ch, const char* ostr, size_t len, vector<pair<vector<pair<string, KPOSTag>>, float>>& ret) const;
public:
	Kiwi(const char* modelPath = "");
	int loadUserDictionary(const char* userDictPath = "");
	int prepare();
	KResult analyze(const wstring& str) const;
	vector<KResult> analyze(const wstring& str, size_t topN) const;
};
