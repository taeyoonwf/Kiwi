﻿#include "gtest/gtest.h"
#include "common.h"
#include "../src/Combiner.h"

using namespace kiwi;

cmb::CompiledRule& getCompiledRule()
{
	static cmb::CompiledRule rule;
	if (!rule.isReady())
	{
		cmb::RuleSet crs;
		std::ifstream ifs{ MODEL_PATH "/combiningRule.txt" };
		crs.loadRules(ifs);
		rule = crs.compile();
	}
	return rule;
}

TEST(KiwiCppCombiner, Combine)
{
	auto& rule = getCompiledRule();

	EXPECT_EQ(rule.combine(u"이", POSTag::vcp, u"다", POSTag::ec, CondVowel::vowel)[0], u"다");
	EXPECT_EQ(rule.combine(u"이", POSTag::vcp, u"었", POSTag::ec, CondVowel::vowel)[0], u"였");
	EXPECT_EQ(rule.combine(u"이", POSTag::vcp, u"ᆫ지도", POSTag::ec, CondVowel::vowel)[0], u"ᆫ지도");
	EXPECT_EQ(rule.combine(u"이", POSTag::vcp, u"ᆫ가", POSTag::ec, CondVowel::vowel)[0], u"ᆫ가");

	EXPECT_EQ(rule.combine(u"ᆯ", POSTag::p, u"ᆯ", POSTag::etm, CondVowel::vowel)[0], u"ᆯ");

	EXPECT_EQ(rule.combine(u"하", POSTag::vv, u"었", POSTag::ep)[0], u"했");
	EXPECT_EQ(rule.combine(u"시", POSTag::ep, u"었", POSTag::ep)[0], u"셨");

	EXPECT_EQ(rule.combine(u"이르", POSTag::vv, u"어", POSTag::ec)[0], u"이르러");
	EXPECT_EQ(rule.combine(u"푸", POSTag::vv, u"어", POSTag::ec)[0], u"퍼");
	EXPECT_EQ(rule.combine(u"따르", POSTag::vv, u"어", POSTag::ec)[0], u"따라");
	EXPECT_EQ(rule.combine(u"돕", POSTag::vv, u"어", POSTag::ec)[0], u"도와");
	EXPECT_EQ(rule.combine(u"하", POSTag::vv, u"도록", POSTag::ec)[0], u"토록");
	EXPECT_EQ(rule.combine(u"하", POSTag::vv, u"어", POSTag::ec)[0], u"해");

	EXPECT_EQ(rule.combine(u"묻", POSTag::vvi, u"어", POSTag::ec)[0], u"물어");
	EXPECT_EQ(rule.combine(u"묻", POSTag::vv, u"어", POSTag::ec)[0], u"묻어");

	EXPECT_EQ(rule.combine(u"타이르", POSTag::p, u"어", POSTag::ec)[0], u"타일러");
	EXPECT_EQ(rule.combine(u"가르", POSTag::p, u"어", POSTag::ec)[0], u"갈라");

	EXPECT_EQ(rule.combine(u"나", POSTag::np, u"가", POSTag::jks)[0], u"내가");

	EXPECT_EQ(rule.combine(u"시", POSTag::ep, u"어용", POSTag::ef)[0], u"세용");
}

TEST(KiwiCppCombiner, Joiner)
{
	auto& rule = getCompiledRule();

	auto joiner = rule.newJoiner();
	joiner.add(u"하", POSTag::vv);
	joiner.add(u"었", POSTag::ep);
	joiner.add(u"다", POSTag::ef);
	EXPECT_EQ(joiner.getU16(), u"했다");

	joiner = rule.newJoiner();
	joiner.add(u"하", POSTag::vv);
	joiner.add(u"시", POSTag::ep);
	joiner.add(u"었", POSTag::ep);
	joiner.add(u"다", POSTag::ef);
	EXPECT_EQ(joiner.getU16(), u"하셨다");

	joiner = rule.newJoiner();
	joiner.add(u"돕", POSTag::vv);
	joiner.add(u"어서", POSTag::ec);
	EXPECT_EQ(joiner.getU16(), u"도와서");

	joiner = rule.newJoiner();
	joiner.add(u"아름답", POSTag::vai);
	joiner.add(u"어", POSTag::ef);
	EXPECT_EQ(joiner.getU16(), u"아름다워");

	joiner = rule.newJoiner();
	joiner.add(u"다시", POSTag::mag);
	joiner.add(u"시동", POSTag::nng);
	joiner.add(u"을", POSTag::jko);
	joiner.add(u"잽싸", POSTag::va);
	joiner.add(u"게", POSTag::ec);
	joiner.add(u"걸", POSTag::vv);
	joiner.add(u"었", POSTag::ep);
	joiner.add(u"다", POSTag::ef);
	joiner.add(u".", POSTag::sf);
	EXPECT_EQ(joiner.getU16(), u"다시 시동을 잽싸게 걸었다.");

	joiner = rule.newJoiner();
	joiner.add(u"작", POSTag::va);
	joiner.add(u"은", POSTag::etm);
	joiner.add(u"소리", POSTag::nng);
	joiner.add(u"이", POSTag::vcp);
	joiner.add(u"라도", POSTag::ec);
	joiner.add(u"듣", POSTag::vvi);
	joiner.add(u"어", POSTag::ef);
	joiner.add(u"!", POSTag::sf);
	EXPECT_EQ(joiner.getU16(), u"작은 소리라도 들어!");
}

TEST(KiwiCppCombiner, Allomorph)
{
	auto& rule = getCompiledRule();

	rule.addAllomorph({
		{ u"를", CondVowel::vowel}, { u"을", CondVowel::non_vowel}
	}, POSTag::jko);

	rule.addAllomorph({
		{ u"가", CondVowel::vowel}, { u"이", CondVowel::non_vowel}
	}, POSTag::jks);

	rule.addAllomorph({
		{ u"로", CondVowel::vocalic}, { u"으로", CondVowel::non_vowel}
	}, POSTag::jkb);

	auto joiner = rule.newJoiner();
	joiner.add(u"시동", POSTag::nng);
	joiner.add(u"를", POSTag::jko);
	EXPECT_EQ(joiner.getU16(), u"시동을");

	joiner = rule.newJoiner();
	joiner.add(u"시도", POSTag::nng);
	joiner.add(u"를", POSTag::jko);
	EXPECT_EQ(joiner.getU16(), u"시도를");

	joiner = rule.newJoiner();
	joiner.add(u"바다", POSTag::nng);
	joiner.add(u"가", POSTag::jks);
	EXPECT_EQ(joiner.getU16(), u"바다가");

	joiner = rule.newJoiner();
	joiner.add(u"바닥", POSTag::nng);
	joiner.add(u"가", POSTag::jks);
	EXPECT_EQ(joiner.getU16(), u"바닥이");

	joiner = rule.newJoiner();
	joiner.add(u"불", POSTag::nng);
	joiner.add(u"으로", POSTag::jkb);
	EXPECT_EQ(joiner.getU16(), u"불로");

	joiner = rule.newJoiner();
	joiner.add(u"북", POSTag::nng);
	joiner.add(u"으로", POSTag::jkb);
	EXPECT_EQ(joiner.getU16(), u"북으로");

	rule.addAllomorph({
		{ u"면", CondVowel::vocalic}, { u"으면", CondVowel::non_vowel}
	}, POSTag::ec);

	joiner = rule.newJoiner();
	joiner.add(u"갈", POSTag::vv);
	joiner.add(u"면", POSTag::ec);
	EXPECT_EQ(joiner.getU16(), u"갈면");

	joiner = rule.newJoiner();
	joiner.add(u"갈", POSTag::vv);
	joiner.add(u"시", POSTag::ep);
	joiner.add(u"았", POSTag::ep);
	joiner.add(u"면", POSTag::ec);
	EXPECT_EQ(joiner.getU16(), u"가셨으면");

	joiner = rule.newJoiner();
	joiner.add(u"하", POSTag::vv);
	joiner.add(u"았", POSTag::ep);
	joiner.add(u"다", POSTag::ef);
	EXPECT_EQ(joiner.getU16(), u"했다");

	joiner = rule.newJoiner();
	joiner.add(u"날", POSTag::vv);
	joiner.add(u"어", POSTag::ef);
	EXPECT_EQ(joiner.getU16(), u"날아");
}