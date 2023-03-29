/**
 * @file SwTokenizer.h
 * @author bab2min (bab2min@gmail.com)
 * @brief Subword Tokenizer
 * @version 0.15.0
 * @date 2022-07-28
 *
 *
 */

#pragma once

#include <future>

#include "Types.h"
#include "FrozenTrie.h"
#include "Utils.h"
#include "Trie.hpp"

namespace kiwi
{
	class Kiwi;
	template<class Ty> class RaggedVector;

	enum class SwTokenFlag : uint8_t
	{
		none = 0,
		special = 1,
		glue = 2,
		subword = 3,
		punct,
		chinese,
	};

	struct SwToken
	{
		const char16_t* form = nullptr;
		uint32_t length = 0;
		POSTag pos = POSTag::unknown;
		SwTokenFlag flags = SwTokenFlag::none;

		SwToken(const char16_t* _form = nullptr, size_t _length = 0, 
			POSTag _pos = POSTag::unknown, SwTokenFlag _flags = SwTokenFlag::none)
			: form{ _form }, length{ (uint32_t)_length }, pos{ _pos }, flags{ _flags }
		{
		}
	};

	struct SwTokenizerConfig
	{
		enum SpecialToken
		{
			unk, cls, sep, pad, mask, bos, eos, glue
		};
		std::array<std::string, eos + 1> specialTokens;
		size_t multipleUnkTokens = 0; // not implemented yet
		size_t vocabSize = 0;
		bool doLowercase = false;
		bool splitChinese = true;
		bool wholeTokenUnk = false;
		bool integrateAllomoprh = true; // not implemented yet
		bool splitPunct = true;
		bool simpleTag = true;
		bool splitVerb = true;
		bool splitEomi = true;
		bool useGlueToken = true;
		bool newlineToken = false; // not implemented yet
		bool strict = false; // not implemented yet
		bool fallbackHangul = true; // not implemented yet
		bool fallbackBPE = false; // not implemented yet


		SwTokenizerConfig()
		{
			specialTokens[unk] = "[UNK]";
		}

		size_t numSpecialTokens() const
		{
			size_t ret = 0;
			for (auto& p : specialTokens)
			{
				ret += p.empty() ? 0 : 1;
			}
			return ret;
		}
	};

	struct UnigramSwTrainerConfig
	{
		double chrCoverage = 0.9995;
		bool reduceStrict = false;
		bool removeRepetitive = true;
	};

	class SwTokenizer;

	class SwTokenizerBuilder
	{
		struct Token
		{
			std::string form;
			float lprob;
			POSTag pos = POSTag::unknown;
			SwTokenFlag flags = SwTokenFlag::none;

			Token(const std::string& _form = {}, 
				POSTag _pos = POSTag::unknown, SwTokenFlag _flag = SwTokenFlag::none,
				float _lprob = 0
			)
				: form{ _form }, lprob{ _lprob }, pos{ _pos }, flags{ _flag }
			{
			}
		};

		const Kiwi* kiwi = nullptr;
		SwTokenizerConfig config;
		Vector<Token> tokens;
	public:
		SwTokenizerBuilder(const Kiwi& kiwi, const SwTokenizerConfig& config);
		SwTokenizerBuilder(const SwTokenizerBuilder&);
		SwTokenizerBuilder(SwTokenizerBuilder&&);
		~SwTokenizerBuilder();
		SwTokenizerBuilder& operator=(const SwTokenizerBuilder&);
		SwTokenizerBuilder& operator=(SwTokenizerBuilder&&);

		void addToken(const char* form, POSTag tag, SwTokenFlag flags, float lprob);
		void addToken(const std::string& form, POSTag tag, SwTokenFlag flags, float lprob);

		size_t size() const { return tokens.size(); }

		SwTokenizer build() const;
	};

	class SwTokenizer
	{
		struct Vocab
		{
			Vector<SwToken> vocabs;
			std::u16string vocabStrPool;

			Vocab();
			Vocab(const Vocab&);
			Vocab(Vocab&&);
			~Vocab();
			Vocab& operator=(const Vocab&);
			Vocab& operator=(Vocab&&);
		};

		friend class SwTokenizerBuilder;
		void* dfTokenizeSubword = nullptr;
		void* dfTokenizeSubwordWithOffset = nullptr;
		const Kiwi* kiwi = nullptr;
		SwTokenizerConfig config;
		Vocab vocab;
		utils::FrozenTrie<kchar_t, uint32_t> trie;
		Vector<uint32_t> tokenFallbacks;
		Vector<float> tokenLProbs;
		Vector<uint32_t> morphToSw;
		Vector<uint32_t> swToMorph;
		std::array<size_t, SwTokenizerConfig::glue + 1> specialTokenIds = { { 0, } };
		UnorderedMap<uint32_t, Vector<uint32_t>> splitCands;

		bool tokenizeSubword(
			U16StringView str,
			bool spacePrefix, 
			std::vector<uint32_t>& out,
			std::vector<std::pair<uint32_t, uint32_t>>* offset = nullptr,
			uint32_t offsetBias = 0
		) const;

	public:
		SwTokenizer(ArchType arch = ArchType::default_);
		SwTokenizer(const SwTokenizer&);
		SwTokenizer(SwTokenizer&&);
		~SwTokenizer();
		SwTokenizer& operator=(const SwTokenizer&);
		SwTokenizer& operator=(SwTokenizer&&);

		size_t size() const { return vocab.vocabs.size(); }
		const SwToken& getVocab(size_t id) const { return vocab.vocabs[id]; }

		bool ready() const { return dfTokenizeSubword; }
		
		bool getWholeWordUnk() const { return config.wholeTokenUnk; }
		void setWholeWordUnk(bool v) { config.wholeTokenUnk = v; }

		void encode(std::vector<uint32_t>& out, const std::string& str, std::vector<std::pair<uint32_t, uint32_t>>* offset = nullptr) const;
		std::vector<uint32_t> encode(const std::string& str, std::vector<std::pair<uint32_t, uint32_t>>* offset = nullptr) const;
		std::string decode(const std::vector<uint32_t>& ids) const;

		std::future<std::vector<uint32_t>> asyncEncode(const std::string& str) const;
		std::future<std::pair<std::vector<uint32_t>, std::vector<std::pair<uint32_t, uint32_t>>>> asyncEncodeOffset(const std::string& str) const;

		const SwTokenizerConfig& getConfig() const { return config; }
		const std::string& getSpecialToken(SwTokenizerConfig::SpecialToken token) const { return config.specialTokens[token]; }
		size_t getSpecialTokenId(SwTokenizerConfig::SpecialToken token) const { return specialTokenIds[token]; }

		std::ostream& save(std::ostream& ostr) const;
		static SwTokenizer load(const Kiwi& kiwi, std::istream& istr);
	};

	class UnigramSwTrainer
	{
		enum class PrefixAvailability : uint8_t;

		struct WordCand
		{
			const Morpheme* morph;
			const Morpheme* suffix;
			const Morpheme* baseEomi = nullptr;
			bool hasBoundaries = false;
			HiddenMember<RaggedVector<int32_t>, sizeof(Vector<size_t>) * 2> tokenizations;

			WordCand(const Morpheme* _morph = nullptr, const Morpheme* _suffix = nullptr);
			WordCand(const WordCand&);
			WordCand(WordCand&&);
			WordCand& operator=(const WordCand&);
			WordCand& operator=(WordCand&&);
			~WordCand();
		};

		const Kiwi* kiwi = nullptr;
		SwTokenizerConfig config;
		UnigramSwTrainerConfig trainConfig;
		size_t knownPrefixSize = 0;
		size_t currentVocabSize = 0;

		UnorderedMap<std::u16string, size_t> wordMap;
		Vector<std::pair<const std::u16string, size_t>*> invWordMap;
		Vector<size_t> wordCnts;
		UnorderedMap<size_t, WordCand> wordSuffix;
		UnorderedMap<std::pair<KString, POSTag>, const Morpheme*> reprMorphMap;
		HiddenMember<RaggedVector<int32_t>, sizeof(Vector<size_t>) * 2> sents;
		Vector<size_t> tokenFreqs;

		Vector<std::u16string> chrPrefix;
		utils::FrozenTrie<char16_t, size_t> chrTrie;
		Vector<Vector<uint32_t>> wordBestTokenizations;

		Vector<uint32_t> prefixFreqs;
		Vector<double> prefixLProbs;
		Vector<PrefixAvailability> prefixAvailable;

		void addWord(const std::u16string& s, const Vector<const Morpheme*>& morphs, const Vector<size_t>& boundaries, bool spacePrefix);

		template<class Feeder>
		size_t _addSentences(Feeder&& feeder);

		Vector<uint32_t> tokenizeShort(U16StringView s, bool spacePrefix = false) const;
		Vector<uint32_t> tokenizeShort(U16StringView s, const Vector<int32_t>& boundaries) const;
		std::pair<Vector<uint32_t>, float> tokenizeBest(U16StringView s, bool spacePrefix = false, const Vector<int32_t>* boundaries = nullptr) const;
		std::pair<Vector<uint32_t>, float> tokenizeBest(const WordCand& m) const;

		const Morpheme* toReprMorph(const Morpheme* m);

	public:
		UnigramSwTrainer(const Kiwi& kiwi, const SwTokenizerConfig& config, const UnigramSwTrainerConfig& trainConfig);
		UnigramSwTrainer(const UnigramSwTrainer&);
		UnigramSwTrainer(UnigramSwTrainer&&);
		~UnigramSwTrainer();
		UnigramSwTrainer& operator=(const UnigramSwTrainer&);
		UnigramSwTrainer& operator=(UnigramSwTrainer&&);

		size_t addSentences(const std::function<std::string()>& feeder);
		size_t addSentences(const std::function<std::u16string()>& feeder);

		float buildSubwordVocabs(const size_t minCnt = 5, const size_t maxPrefixLength = 15);
		float updateProb(bool init = false);
		size_t reduceVocab(float ratio, size_t minVocabSize = 0);
		void updateTokenization();
		
		std::vector<std::u16string> getUnkExamples() const;

		size_t getCurrentVocabSize() const { return currentVocabSize + config.numSpecialTokens(); }

		std::ostream& writeVocabs(std::ostream& os) const;
		
		SwTokenizer build() const;
		
		std::ostream& writeTokenizer(std::ostream& os) const;
	};
}
