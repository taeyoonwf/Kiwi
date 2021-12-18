﻿#include "gtest/gtest.h"
#include <cstring>
#include <kiwi/capi.h>
#include "common.h"

kiwi_h reuseKiwiInstance()
{
	static kiwi_h kw = kiwi_init(MODEL_PATH, 0, KIWI_BUILD_DEFAULT);
	return kw;
}

TEST(KiwiC, InitClose) 
{
	kiwi_h kw = kiwi_init(MODEL_PATH, 0, KIWI_BUILD_DEFAULT);
	EXPECT_NE(kw, nullptr);
	EXPECT_EQ(kiwi_close(kw), 0);
}

TEST(KiwiC, BuilderInitClose)
{
	kiwi_builder_h kb = kiwi_builder_init(MODEL_PATH, 0, KIWI_BUILD_DEFAULT);
	EXPECT_NE(kb, nullptr);
	kiwi_h kw = kiwi_builder_build(kb);
	EXPECT_NE(kw, nullptr);
	EXPECT_EQ(kiwi_builder_close(kb), 0);
	EXPECT_EQ(kiwi_close(kw), 0);
}

TEST(KiwiC, BuilderAddWords)
{
	kiwi_builder_h kb = kiwi_builder_init(MODEL_PATH, 0, KIWI_BUILD_DEFAULT);
	EXPECT_NE(kb, nullptr);
	EXPECT_EQ(kiwi_builder_add_word(kb, KWORD8, "NNP", 0.0), 0);
	kiwi_h kw = kiwi_builder_build(kb);
	EXPECT_NE(kw, nullptr);
	EXPECT_EQ(kiwi_builder_close(kb), 0);
	
	kiwi_res_h res = kiwi_analyze(kw, KWORD8, 1, KIWI_MATCH_ALL);
	EXPECT_NE(res, nullptr);
	const char* word = kiwi_res_form(res, 0, 0);
	EXPECT_NE(word, nullptr);
	EXPECT_EQ(strcmp(word, KWORD8), 0);
	EXPECT_EQ(kiwi_res_close(res), 0);
	EXPECT_EQ(kiwi_close(kw), 0);
}

int mt_reader(int idx, char* buf, void* user)
{
	auto& data = *(std::vector<std::string>*)user;
	if (idx >= data.size()) return 0;
	if (buf == nullptr) return data[idx].size() + 1;
	std::memcpy(buf, data[idx].c_str(), data[idx].size() + 1);
	return 0;
}

int mt_receiver(int idx, kiwi_res_h res, void* user)
{
	kiwi_res_close(res);
	return 0;
}

TEST(KiwiC, AnalyzeMultithread)
{
	auto data = loadTestCorpus();
	kiwi_h kw = kiwi_init(MODEL_PATH, 2, KIWI_BUILD_DEFAULT);
	EXPECT_NE(kw, nullptr);
	EXPECT_EQ(kiwi_analyze_m(kw, mt_reader, mt_receiver, &data, 1, KIWI_MATCH_ALL), data.size());
	EXPECT_EQ(kiwi_close(kw), 0);
}

TEST(KiwiC, Issue71_SentenceSplit_u16)
{
	kiwi_h kw = reuseKiwiInstance();

	const char16_t str[] = u"다녀온 후기\n\n강남 토끼정에 다녀왔습니다. 음식도 맛있었어요 다만 역시 토끼정 본점 답죠?ㅎㅅㅎ 그 맛이 크으.. 아주 맛있었음...! ^^";
	const char16_t* ref[] = {
		u"다녀온 후기",
		u"강남 토끼정에 다녀왔습니다.",
		u"음식도 맛있었어요",
		u"다만 역시 토끼정 본점 답죠?ㅎㅅㅎ",
		u"그 맛이 크으..",
		u"아주 맛있었음...! ^^",
	};
	const int ref_len = sizeof(ref) / sizeof(ref[0]);

	kiwi_ss_h res = kiwi_split_into_sentences_w(kw, (const kchar16_t*)str, KIWI_MATCH_ALL_WITH_NORMALIZING, nullptr);
	EXPECT_NE(res, nullptr);
	EXPECT_EQ(kiwi_ss_size(res), ref_len);
	
	for (int i = 0; i < ref_len; ++i)
	{
		std::u16string sent{ str + kiwi_ss_begin_position(res, i), str + kiwi_ss_end_position(res, i) };
		EXPECT_EQ(sent, ref[i]);
	}

	EXPECT_EQ(kiwi_ss_close(res), 0);
}

TEST(KiwiC, Issue71_SentenceSplit_u8)
{
	kiwi_h kw = reuseKiwiInstance();

	const char str[] = u8"다녀온 후기\n\n강남 토끼정에 다녀왔습니다. 음식도 맛있었어요 다만 역시 토끼정 본점 답죠?ㅎㅅㅎ 그 맛이 크으.. 아주 맛있었음...! ^^";
	const char* ref[] = {
		u8"다녀온 후기",
		u8"강남 토끼정에 다녀왔습니다.",
		u8"음식도 맛있었어요",
		u8"다만 역시 토끼정 본점 답죠?ㅎㅅㅎ",
		u8"그 맛이 크으..",
		u8"아주 맛있었음...! ^^",
	};
	const int ref_len = sizeof(ref) / sizeof(ref[0]);

	kiwi_ss_h res = kiwi_split_into_sentences(kw, str, KIWI_MATCH_ALL_WITH_NORMALIZING, nullptr);
	EXPECT_NE(res, nullptr);
	EXPECT_EQ(kiwi_ss_size(res), ref_len);

	for (int i = 0; i < ref_len; ++i)
	{
		std::string sent{ str + kiwi_ss_begin_position(res, i), str + kiwi_ss_end_position(res, i) };
		EXPECT_EQ(sent, ref[i]);
	}

	EXPECT_EQ(kiwi_ss_close(res), 0);
}
