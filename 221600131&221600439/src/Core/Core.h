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


EXTERN WordCountResult CalculateWordCount(const char* fileName);
EXTERN void ClearWordAppear(WordCountResult* resultStruct);

#undef EXTERN