// Core.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Core.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

#define EXTERN extern "C" __declspec(dllexport)

constexpr size_t wordAtLeastCharacterCount = 4;

constexpr bool isAlphabet(const char c) {
	return (c >= 'a' && c <= 'z');
}

constexpr bool isNumber(const char c) {
	return (c >= '0' && c <= '9');
}

constexpr bool isLetter(const char c) {
	return (isAlphabet(c) || isNumber(c));
}

constexpr bool isLf(const char c) {
	return (c == '\n');
}

constexpr bool isCr(const char c) {
	return (c == '\r');
}

constexpr bool isSeparator(const char c) {
	return (isCr(c) || isLf(c) || c == ' ' || c == '\t');
}

constexpr bool isCharacter(const char c) {
	return (isSeparator(c) || isLetter(c));
}


typedef std::pair<std::string, size_t> WordCountPair;

enum ReadingStatus {
	ALREADY,
	DONE,
	READING_PAPER_INDEX,
	WAITING_FOR_TITLE,
	READ_WORDS,
	WAITING_FOR_ABSTRACT,
};

enum WordStatus {
	NONE,
	TITLE,
	ABSTRACT
};

EXTERN WordCountResult CalculateWordCount(struct WordCountConfig config)
{
	auto ret = WordCountResult();
	bool runStateMachine = true;
	char prev = 0, c = 0;
	std::ifstream file(config.in);
	std::string token = ""; // 不想做动态分配内存，std::string省事 
	size_t wordLength = 0; // <= wordAtLeastCharacterCount，超过则不再计数
	bool isWord = false;
	auto map = std::map<std::string, size_t>();
	file >> std::noskipws;

	if (!file.good()) {
		ret.errorCode = WORDCOUNTRESULT_OPEN_FILE_FAILED;
		return ret;
	}

	ReadingStatus currentStatus = ReadingStatus::ALREADY;
	WordStatus wordStatus = WordStatus::NONE;

	while (runStateMachine) {
		prev = c;
		file >> c;
		if (file.eof()) {
			runStateMachine = false; // 文件读取结束，不立即退出，处理一下之前未整理干净的状态
			c = 0;
		}

		if (c >= 'A' && c <= 'Z') {
			c = c - 'A' + 'a';
		}

		bool switchStatus = true;
		while (switchStatus) {
			switchStatus = false;
			switch (currentStatus) {
			case ReadingStatus::ALREADY:
				if (isNumber(c)) {
					currentStatus = READING_PAPER_INDEX;
					switchStatus = true;
					continue;
				}
				else if (isSeparator(c)) { // 正常， do nothing
				}
				else { // 此处要抛错
				}
				break;
			case ReadingStatus::READING_PAPER_INDEX:
				if (isNumber(c)) {
					token += c;
				}
				else if (isSeparator(c)) { // 编号读完，状态转换开始
					token = ""; // 这个编号数据没啥用，我也不知道读了干啥
					currentStatus = WAITING_FOR_TITLE;
				}
				else { // 此处要抛错
				}
				break;
			case ReadingStatus::WAITING_FOR_TITLE:
				if (isSeparator(c)) { // 可能是还没读完Title，也可能是已经读完了
					if (token == "title:") { // 读完了
						currentStatus = ReadingStatus::READ_WORDS;
						wordStatus = WordStatus::TITLE;
						token = "";
					}
				}
				else {
					token += c; // 暂不判断title:是否完全正确，假设其规范；之后加入错误提示
				}
				break;
			case ReadingStatus::WAITING_FOR_ABSTRACT:
				if (isSeparator(c)) { // 同title
					if (token == "abstract:") { // 读完了
						currentStatus = ReadingStatus::READ_WORDS;
						wordStatus = WordStatus::ABSTRACT;
						token = "";
					}
				}
				else {
					token += c;
				}
				break;
			case ReadingStatus::READ_WORDS: // 不考虑错误处理的情况下，直接把read token和parse做在一起比较方便
				if (isCharacter(c)) {
					ret.characters++;
					if (isLetter(c)) {
						token += c;
						if (!isLetter(prev)) { // 如果当前确定这个字符是单词的第一个字母，就确定它是单词
							isWord = true;
						}
						if (isWord && wordLength <= wordAtLeastCharacterCount) {
							if (!isAlphabet(c)) { // abc123不算单词
								isWord = false;
							}
							else {
								wordLength++;
							}
						}
					}
				}
				if ((isCharacter(c) && !isLetter(c)) || !runStateMachine) {
					if (wordLength >= wordAtLeastCharacterCount) {
						if (map.find(token) == map.end()) {
							map[token] = 0;
							ret.uniqueWords++;
						}

						if (config.useDifferentWeight) {
							if (wordStatus == WordStatus::TITLE) {
								map[token] += 10;
							}
							else {
								map[token] += 1;
							}
						}
						else {
							map[token]++;
						}

						ret.words++;
					}
					token = "";
					wordLength = 0;
					isWord = false;
				}
				if (isLf(c) || !runStateMachine) {
					ret.lines++;
					if (wordStatus == WordStatus::TITLE) {
						currentStatus = ReadingStatus::WAITING_FOR_ABSTRACT;
					}
					else {
						currentStatus = ReadingStatus::ALREADY;
					}
				}
				break;
			}
		}
	}

	file.close();

	auto sortedMap = std::vector<WordCountPair>(map.begin(), map.end());
	std::sort(sortedMap.begin(), sortedMap.end(), [](const WordCountPair& lhs, const WordCountPair& rhs) {
		if (lhs.second == rhs.second) {
			return lhs.first < rhs.first;
		}
		return lhs.second > rhs.second;
	});
	ret.wordAppears = new WordCountWordAppear[ret.uniqueWords];
	size_t i = 0;
	for (auto &it : sortedMap) {
		ret.wordAppears[i].word = new char[it.first.length() + 1];
		strcpy_s(ret.wordAppears[i].word, it.first.length() + 1, it.first.c_str());
		ret.wordAppears[i].count = it.second;
		i++;
	}

	return ret;

}


EXTERN void ClearWordAppear(WordCountResult * resultStruct)
{
	for (size_t i = 0; i < resultStruct->uniqueWords; i++) {
		delete resultStruct->wordAppears[i].word;
	}
	delete resultStruct->wordAppears;
}
