#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Core/Core.h"
#include <string>
#include <fstream>
#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::experimental;
using std::string;
using std::ofstream;

namespace TestWordCount
{		
	TEST_CLASS(UnitTest1)
	{
	private:
		string writeTestFile(string content) {
			auto fileName = std::tmpnam(nullptr);
			ofstream file(fileName);
			file << content;
			file.close();
			return fileName;
		}

		struct WordCountResult doTest(string in, WordCountConfig * config) {
			auto fileName = writeTestFile(in);
			config->in = fileName.c_str();
			auto ret = CalculateWordCount(*config);
			filesystem::remove(fileName);
			return ret;
		}
	public:
		

		TEST_METHOD(TestBasic)
		{
			auto config = WordCountConfig();
			config.statByPharse = false;
			config.useDifferentWeight = false;
			auto out = doTest("0\nTitle: Monday Tuesday Wednesday Thursday\nAbstract: Friday", &config);
			Assert::AreEqual(out.characters, (size_t)40);
			Assert::AreEqual(out.words, (size_t)5);
			Assert::AreEqual(out.lines, (size_t)2);
			Assert::AreEqual(out.wordAppears[0].word, "friday");
			Assert::AreEqual(out.wordAppears[0].count, (size_t)1);
			Assert::AreEqual(out.wordAppears[1].word, "monday");
			Assert::AreEqual(out.wordAppears[1].count, (size_t)1);
			Assert::AreEqual(out.wordAppears[2].word, "thursday");
			Assert::AreEqual(out.wordAppears[2].count, (size_t)1);
			ClearWordAppear(&out);
		}

		TEST_METHOD(TestPharse)
		{
			auto config = WordCountConfig();
			config.statByPharse = true;
			config.pharseSize = 3;
			config.useDifferentWeight = false;
			auto out = doTest("0\nTitle: Monday Tuesday Wednesday Thursday\nAbstract: Friday", &config);
			Assert::AreEqual(out.characters, (size_t)40);
			Assert::AreEqual(out.words, (size_t)5);
			Assert::AreEqual(out.lines, (size_t)2);
			Assert::AreEqual(out.uniqueWordsOrPharses, (size_t)2);
			Assert::AreEqual(out.wordAppears[0].word, "monday tuesday wednesday");
			Assert::AreEqual(out.wordAppears[0].count, (size_t)1);
			Assert::AreEqual(out.wordAppears[1].word, "tuesday wednesday thursday");
			Assert::AreEqual(out.wordAppears[1].count, (size_t)1);
			ClearWordAppear(&out);
		}


		TEST_METHOD(TestMulti)
		{
			auto config = WordCountConfig();
			config.statByPharse = true;
			config.pharseSize = 3;
			config.useDifferentWeight = true;
			auto out = doTest("0\nTitle: Monday Tuesday Wednesday Thursday\nAbstract: Monday Tuesday Wednesday Thursday Friday", &config);
			Assert::AreEqual(out.characters, (size_t)74);
			Assert::AreEqual(out.words, (size_t)9);
			Assert::AreEqual(out.lines, (size_t)2);
			Assert::AreEqual(out.wordAppears[0].word, "monday tuesday wednesday");
			Assert::AreEqual(out.wordAppears[0].count, (size_t)11);
			Assert::AreEqual(out.wordAppears[1].word, "tuesday wednesday thursday");
			Assert::AreEqual(out.wordAppears[1].count, (size_t)11);
			Assert::AreEqual(out.wordAppears[2].word, "wednesday thursday friday");
			Assert::AreEqual(out.wordAppears[2].count, (size_t)1);
			ClearWordAppear(&out);
		}


	};

	
}