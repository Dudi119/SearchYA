#include "SearchModule.h"
#include <string>
#include <memory>
#include <iostream>
#include "Core/src/Enviorment.h"
#include "Core/src/CommandLine.h"
#include "Core/src/Assert.h"
#include "Core/src/Logger.h"
#include "Core/src/TraceListener.h"
#include "Core/src/Deleter.h"
#include "Communication/Serializor.h"
#include "DocumentsIndexer.h"
#define REDIS_EXTERN
#include "Redis/redismodule.h"

using namespace std;
using namespace core;

extern "C"
{
	int GetWordTopKDocument(RedisModuleCtx* ctx, RedisModuleString** argv, int argc)
	{
		TRACE_INFO("Command - {Get Top K} was received");
		//command name + word + k == 3
		VERIFY(argc == 3, "GetWordTopKDocument arg list cannot be larger than 2");
		size_t binaryDataLength = 0;
		const char* wordRawBuffer = RedisModule_StringPtrLen(argv[1], &binaryDataLength);
		Serializor wordSerializor(wordRawBuffer, binaryDataLength);
		string word = Serializor::DeserializeString(wordSerializor);

		const char* kRawBuffer = RedisModule_StringPtrLen(argv[2], &binaryDataLength);
        Serializor kSerializor(kRawBuffer, binaryDataLength);
		int k = Serializor::DeserializeInt(kSerializor)	;
		TRACE_INFO("%d - %s", k, word.c_str());
		vector<Document> topKDocuments = DocumentsIndexer::Instance().GetTopKDocumentsPerWord(ctx, word, k);

		RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
		for(const auto& document : topKDocuments)
		{
			string serializedDocument = Document::Serialize(document);
			RedisModule_ReplyWithStringBuffer(ctx, serializedDocument.c_str(), serializedDocument.length());
		}
		RedisModule_ReplySetArrayLength(ctx, topKDocuments.size());
		return REDISMODULE_OK;
	}

	int AddDocument(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
	{
        TRACE_INFO("Command - {Add Document} was received");
		//command name + document serialized data == 2
		VERIFY(argc == 2, "AddDocument arg list cannot be larger than 2");
		size_t stringLength = 0;
		const char* binaryData = RedisModule_StringPtrLen(argv[1], &stringLength);
		Document receivedDocument = std::move(Document::Deserialize(binaryData, (int)stringLength));
		DocumentsIndexer::Instance().AddDocument(ctx, receivedDocument);
		RedisModule_ReplyWithSimpleString(ctx,"OK");
		return REDISMODULE_OK;
	}

	int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
	{
		if (RedisModule_Init(ctx, "Search", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
			    return REDISMODULE_ERR;
		VERIFY(argc == 1, "AddDocument arg list cannot be larger than 1");
		size_t stringLength;
		const char* binaryData = RedisModule_StringPtrLen(argv[0], &stringLength);
		CommandLine::Instance().Parse(argc, &binaryData);

		Enviorment::Instance().Init();
		string workingDir = Enviorment::Instance().GetProcessPath() + "/" + CommandLine::Instance().GetArgument("workingdir");
		Logger::Instance().AddListener(make_shared<FileRotationListener>(TraceSeverity::Info, workingDir + "/SearchModule", 50 * 1024 * 1024, 20));
		Logger::Instance().Start(TraceSeverity::Info);

		if(RedisModule_CreateCommand(ctx, "Search.AddDocument", &AddDocument, "", 0, 0, 0) == REDISMODULE_ERR)
				return REDISMODULE_ERR;
		if(RedisModule_CreateCommand(ctx, "Search.GetTopKDocuments", &GetWordTopKDocument, "", 0, 0, 0) == REDISMODULE_ERR)
			return REDISMODULE_ERR;


		TRACE_INFO("Redis Search Module is up");
		return REDISMODULE_OK;
	}
}

