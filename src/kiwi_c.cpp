#include <memory>
#include "core/Kiwi.h"

#include <kiwi/capi.h>

using namespace std;
using namespace kiwi;

const char* kiwi_version()
{
	return Kiwi::getVersion();
}

struct ResultBuffer
{
	vector<string> stringBuf;
};

thread_local exception_ptr currentError;

const char* kiwi_error()
{
	if (currentError)
	{
		try
		{
			rethrow_exception(currentError);
		}
		catch (const exception& e)
		{
			return e.what();
		}
	}
	return nullptr;
}

kiwi_h kiwi_init(const char * modelPath, int numThread, int options)
{
	try
	{
		return (kiwi_h)new Kiwi{ modelPath, 0, (size_t)numThread, (size_t)options };
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

int kiwi_add_user_word(kiwi_h handle, const char * word, const char * pos)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		return kiwi->addUserWord(Kiwi::toU16(word), toPOSTag(Kiwi::toU16(pos)));
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_load_user_dict(kiwi_h handle, const char * dictPath)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		kiwi->loadUserDictionary(dictPath);
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

kiwi_ws_h kiwi_extract_words(kiwi_h handle, kiwi_reader_t reader, void * userData, int minCnt, int maxWordLen, float minScore)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto res = kiwi->extractWords([&]()
		{
			auto idx = make_shared<int>(0);
			return [&]() -> u16string
			{
				string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, &buf[0], userData);
				++*idx;
				return Kiwi::toU16(buf);
			};
		}, minCnt, maxWordLen, minScore);

		return (kiwi_ws_h)new pair<vector<KWordDetector::WordInfo>, ResultBuffer>{ move(res), {} };
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

kiwi_ws_h kiwi_extract_filter_words(kiwi_h handle, kiwi_reader_t reader, void * userData, int minCnt, int maxWordLen, float minScore, float posThreshold)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto res = kiwi->extractWords([&]()
		{
			auto idx = make_shared<int>(0);
			return [&]() -> u16string
			{
				string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, &buf[0], userData);
				++* idx;
				return Kiwi::toU16(buf);
			};
		}, minCnt, maxWordLen, minScore);
		res = kiwi->filterExtractedWords(move(res), posThreshold);
		return (kiwi_ws_h)new pair<vector<KWordDetector::WordInfo>, ResultBuffer>{ move(res),{} };
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

kiwi_ws_h kiwi_extract_add_words(kiwi_h handle, kiwi_reader_t reader, void * userData, int minCnt, int maxWordLen, float minScore, float posThreshold)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto res = kiwi->extractAddWords([&]()
		{
			auto idx = make_shared<int>(0);
			return [&]() -> u16string
			{
				string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, &buf[0], userData);
				++* idx;
				return Kiwi::toU16(buf);
			};
		}, minCnt, maxWordLen, minScore, posThreshold);
		return (kiwi_ws_h)new pair<vector<KWordDetector::WordInfo>, ResultBuffer>{ move(res),{} };
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

kiwi_ws_h kiwi_extract_words_w(kiwi_h handle, kiwi_reader_w_t reader, void * userData, int minCnt, int maxWordLen, float minScore)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto res = kiwi->extractWords([&]()
		{
			auto idx = make_shared<int>(0);
			return [&]()->u16string
			{
				u16string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, (kchar16_t*)&buf[0], userData);
				++* idx;
				return buf;
			};
		}, minCnt, maxWordLen, minScore);

		return (kiwi_ws_h)new pair<vector<KWordDetector::WordInfo>, ResultBuffer>{ move(res),{} };
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

kiwi_ws_h kiwi_extract_filter_words_w(kiwi_h handle, kiwi_reader_w_t reader, void * userData, int minCnt, int maxWordLen, float minScore, float posThreshold)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto res = kiwi->extractWords([&]()
		{
			auto idx = make_shared<int>(0);
			return [&]()->u16string
			{
				u16string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, (kchar16_t*)&buf[0], userData);
				++* idx;
				return buf;
			};
		}, minCnt, maxWordLen, minScore);
		res = kiwi->filterExtractedWords(move(res), posThreshold);
		return (kiwi_ws_h)new pair<vector<KWordDetector::WordInfo>, ResultBuffer>{ move(res),{} };
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

kiwi_ws_h kiwi_extract_add_words_w(kiwi_h handle, kiwi_reader_w_t reader, void * userData, int minCnt, int maxWordLen, float minScore, float posThreshold)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto res = kiwi->extractAddWords([&]()
		{
			auto idx = make_shared<int>(0);
			return [&]()->u16string
			{
				u16string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, (kchar16_t*)&buf[0], userData);
				++* idx;
				return buf;
			};
		}, minCnt, maxWordLen, minScore, posThreshold);
		return (kiwi_ws_h)new pair<vector<KWordDetector::WordInfo>, ResultBuffer>{ move(res),{} };
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

int kiwi_prepare(kiwi_h handle)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		kiwi->prepare();
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

DECL_DLL void kiwi_set_option(kiwi_h handle, int option, int value)
{
	if (!handle) return;
	Kiwi* kiwi = (Kiwi*)handle;
	kiwi->setOption(option, value);
}

DECL_DLL int kiwi_get_option(kiwi_h handle, int option)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	return kiwi->getOption(option);
}

kiwi_res_h kiwi_analyze_w(kiwi_h handle, const kchar16_t * text, int topN, int matchOptions)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto result = new pair<vector<KResult>, ResultBuffer>{ kiwi->analyze((const char16_t*)text, topN, matchOptions), {} };
		return (kiwi_res_h)result;
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

kiwi_res_h kiwi_analyze(kiwi_h handle, const char * text, int topN, int matchOptions)
{
	if (!handle) return nullptr;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		auto result = new pair<vector<KResult>, ResultBuffer>{ kiwi->analyze(text, topN, matchOptions),{} };
		return (kiwi_res_h)result;
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

int kiwi_analyze_mw(kiwi_h handle, kiwi_reader_w_t reader, kiwi_receiver_t receiver, void * userData, int topN, int matchOptions)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		int idx = 0;
		kiwi->analyze(topN, [&]() -> u16string
		{
			u16string buf;
			buf.resize((*reader)(idx, nullptr, userData));
			if (buf.empty()) return {};
			(*reader)(idx, (kchar16_t*)&buf[0], userData);
			++idx;
			return buf;
		}, [&](size_t id, vector<KResult>&& res)
		{
			auto result = new pair<vector<KResult>, ResultBuffer>{ res, {} };
			(*receiver)(id, (kiwi_res_h)result, userData);
		}, matchOptions);
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_analyze_m(kiwi_h handle, kiwi_reader_t reader, kiwi_receiver_t receiver, void * userData, int topN, int matchOptions)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		int idx = 0;
		kiwi->analyze(topN, [&]() -> u16string
		{
			string buf;
			buf.resize((*reader)(idx, nullptr, userData));
			if (buf.empty()) return {};
			(*reader)(idx, &buf[0], userData);
			++idx;
			return Kiwi::toU16(buf);
		}, [&](size_t id, vector<KResult>&& res)
		{
			auto result = new pair<vector<KResult>, ResultBuffer>{ res,{} };
			(*receiver)(id, (kiwi_res_h)result, userData);
		}, matchOptions);
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_perform_w(kiwi_h handle, kiwi_reader_w_t reader, kiwi_receiver_t receiver, void * userData, int topN, int matchOptions, int minCnt, int maxWordLen, float minScore, float posThreshold)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		kiwi->perform(topN, [&]()
		{
			auto idx = make_shared<int>(0);
			return [&]() -> u16string
			{
				u16string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, (kchar16_t*)&buf[0], userData);
				++*idx;
				return buf;
			};
		}, [&](size_t id, vector<KResult>&& res)
		{
			auto result = new pair<vector<KResult>, ResultBuffer>{ res,{} };
			(*receiver)(id, (kiwi_res_h)result, userData);
		}, matchOptions, minCnt, maxWordLen, minScore, posThreshold);
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_perform(kiwi_h handle, kiwi_reader_t reader, kiwi_receiver_t receiver, void * userData, int topN, int matchOptions, int minCnt, int maxWordLen, float minScore, float posThreshold)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		kiwi->perform(topN, [&]()
		{
			auto idx = make_shared<int>(0);
			return [&]() -> u16string
			{
				string buf;
				buf.resize((*reader)(*idx, nullptr, userData));
				if (buf.empty()) return {};
				(*reader)(*idx, &buf[0], userData);
				++*idx;
				return Kiwi::toU16(buf);
			};
		}, [&](size_t id, vector<KResult>&& res)
		{
			auto result = new pair<vector<KResult>, ResultBuffer>{ res,{} };
			(*receiver)(id, (kiwi_res_h)result, userData);
		}, matchOptions, minCnt, maxWordLen, minScore, posThreshold);
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_close(kiwi_h handle)
{
	if (!handle) return KIWIERR_INVALID_HANDLE;
	Kiwi* kiwi = (Kiwi*)handle;
	try
	{
		delete kiwi;
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_res_size(kiwi_res_h result)
{
	if (!result) return KIWIERR_INVALID_HANDLE;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		return k->first.size();
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

float kiwi_res_prob(kiwi_res_h result, int index)
{
	if (!result) return 0;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size()) return 0;
		return k->first[index].second;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_res_word_num(kiwi_res_h result, int index)
{
	if (!result) return KIWIERR_INVALID_HANDLE;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size()) return KIWIERR_INVALID_INDEX;
		return k->first[index].first.size();
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

const kchar16_t * kiwi_res_form_w(kiwi_res_h result, int index, int num)
{
	if (!result) return nullptr;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size() || num < 0 || num >= k->first[index].first.size()) return nullptr;
		return (const kchar16_t*)k->first[index].first[num].str.c_str();
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

const kchar16_t * kiwi_res_tag_w(kiwi_res_h result, int index, int num)
{
	if (!result) return nullptr;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size() || num < 0 || num >= k->first[index].first.size()) return nullptr;
		return (const kchar16_t*)tagToKString(k->first[index].first[num].tag);
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

const char * kiwi_res_form(kiwi_res_h result, int index, int num)
{
	if (!result) return nullptr;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size() || num < 0 || num >= k->first[index].first.size()) return nullptr;
		k->second.stringBuf.emplace_back(Kiwi::toU8(k->first[index].first[num].str));
		return k->second.stringBuf.back().c_str();
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

const char * kiwi_res_tag(kiwi_res_h result, int index, int num)
{
	if (!result) return nullptr;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size() || num < 0 || num >= k->first[index].first.size()) return nullptr;
		return tagToString(k->first[index].first[num].tag);
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

int kiwi_res_position(kiwi_res_h result, int index, int num)
{
	if (!result) return -1;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size() || num < 0 || num >= k->first[index].first.size()) return -1;
		return k->first[index].first[num].position;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_res_length(kiwi_res_h result, int index, int num)
{
	if (!result) return -1;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size() || num < 0 || num >= k->first[index].first.size()) return -1;
		return k->first[index].first[num].length;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_res_close(kiwi_res_h result)
{
	if (!result) return KIWIERR_INVALID_HANDLE;
	try
	{
		auto k = (pair<vector<KResult>, ResultBuffer>*)result;
		delete k;
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

int kiwi_ws_size(kiwi_ws_h result)
{
	if (!result) return KIWIERR_INVALID_HANDLE;
	try
	{
		auto k = (pair<vector<KWordDetector::WordInfo>, ResultBuffer>*)result;
		return k->first.size();
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

const kchar16_t * kiwi_ws_form_w(kiwi_ws_h result, int index)
{
	if (!result) return nullptr;
	try
	{
		auto k = (pair<vector<KWordDetector::WordInfo>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size()) return nullptr;
		return (const kchar16_t*)k->first[index].form.c_str();
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

const char * kiwi_ws_form(kiwi_ws_h result, int index)
{
	if (!result) return nullptr;
	try
	{
		auto k = (pair<vector<KWordDetector::WordInfo>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size()) return nullptr;
		k->second.stringBuf.emplace_back(Kiwi::toU8(k->first[index].form));
		return k->second.stringBuf.back().c_str();
	}
	catch (...)
	{
		currentError = current_exception();
		return nullptr;
	}
}

float kiwi_ws_score(kiwi_ws_h result, int index)
{
	if (!result) return NAN;
	try
	{
		auto k = (pair<vector<KWordDetector::WordInfo>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size()) return NAN;
		return k->first[index].score;
	}
	catch (...)
	{
		currentError = current_exception();
		return NAN;
	}
}

int kiwi_ws_freq(kiwi_ws_h result, int index)
{
	if (!result) return KIWIERR_INVALID_HANDLE;
	try
	{
		auto k = (pair<vector<KWordDetector::WordInfo>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size()) return KIWIERR_INVALID_INDEX;
		return k->first[index].freq;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

float kiwi_ws_pos_score(kiwi_ws_h result, int index)
{
	if (!result) return NAN;
	try
	{
		auto k = (pair<vector<KWordDetector::WordInfo>, ResultBuffer>*)result;
		if (index < 0 || index >= k->first.size()) return NAN;
		return k->first[index].posScore[POSTag::nnp];
	}
	catch (...)
	{
		currentError = current_exception();
		return NAN;
	}
}

int kiwi_ws_close(kiwi_ws_h result)
{
	if (!result) return KIWIERR_INVALID_HANDLE;
	try
	{
		auto k = (pair<vector<KWordDetector::WordInfo>, ResultBuffer>*)result;
		delete k;
		return 0;
	}
	catch (...)
	{
		currentError = current_exception();
		return KIWIERR_FAIL;
	}
}

