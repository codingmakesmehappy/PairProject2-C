#pragma once
#include "stdafx.h"
#define EXTERN extern "C" __declspec(dllexport)

constexpr auto WORDCOUNTRESULT_SUCCEED = 0;
constexpr auto WORDCOUNTRESULT_OPEN_FILE_FAILED = 1;

struct WordCountWordAppear {
	char *word;
	size_t count;
};


struct WordCountResult {
	size_t errorCode = WORDCOUNTRESULT_SUCCEED;
	size_t lines = 0;
	size_t words = 0;
	size_t characters = 0;
	size_t uniqueWords = 0;
	struct WordCountWordAppear *wordAppears;
};

struct WordCountConfig {
	const char* in;
	bool statByPharse = false;
	size_t pharseSize = 0;
	bool useDifferentWeight = false;
};


EXTERN WordCountResult CalculateWordCount(struct WordCountConfig config);
EXTERN void ClearWordAppear(WordCountResult* resultStruct);

#undef EXTERN