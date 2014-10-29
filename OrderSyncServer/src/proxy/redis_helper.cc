#include "redis_helper.h"
#include "log.h"
#include <string.h>


int
redis_helper_t::Connect(const char* host, 
					  const unsigned short& port, 
					  const unsigned short second)
{
	 if (context)
	 {
		redisFree(context);
	 }
	 
	 struct timeval tv = {second, 0};
	 
	 context = redisConnectWithTimeout(host, port, tv);
	 
	 return context ? 0 : 1;
}

int
redis_helper_t::Connect()
{
	 if (context) {
		redisFree(context);
	 }
	 
	 context = redisConnectWithTimeout(host_.c_str(), port_, timeout);
	 
	 return context ? 0 : 1;
}



int
redis_helper_t::Set(const string& key, const string& value)
{
	 reply = static_cast<redisReply*>(redisCommand(context, "SET %s %s", key.c_str(), value.c_str()));
	 
	 int result = reply ? reply->integer : 1;
	 
	 freeReply();
	 
	 return result;
}

string&
redis_helper_t::Get(const string& key, string& value)
{
	 reply = static_cast<redisReply*>(redisCommand(context, "GET %s", key.c_str()));
	 
	 value = reply->str;
	 
	 freeReply();
	 
	 return value;
}

int
redis_helper_t::Enqueue(const string& queue, const string& value)
{
	reply = static_cast<redisReply*>(redisCommand(context, "rpush %s %s", queue.c_str(), value.c_str()));

	int result = reply ? reply->integer : 1; 

	freeReply();

	return result;
}

string&
redis_helper_t::Dequeue(const string& queue, string& value)
{
	reply = static_cast<redisReply*>(redisCommand(context, "lpop %s", queue.c_str()));

	log_debug("KEY: %s\n", queue.c_str());
	
	if (reply) {
		log_debug("reply->type: %d\n", reply->type);

		if (reply->type == 1) {
			value = reply->str;		
		}

		freeReply();
	}

	return value;
}

int
redis_helper_t::Push(const string& stack, const string& value)
{
	reply = static_cast<redisReply*>(redisCommand(context, "rpush %s %s", stack.c_str(), value.c_str()));

	int result = reply ? reply->integer : 1; 

	freeReply();

	return result;
}

string&
redis_helper_t::Pop(const string& stack, string& value)
{
	reply = static_cast<redisReply*>(redisCommand(context, "rpop %s", stack.c_str()));

	value = reply->str;

	freeReply();

	return value;
}

bool
redis_helper_t::ping()
{
	if (!context)
	{
		return false;
	}
	
	reply = static_cast<redisReply*>(redisCommand(context, "ping"));

	bool IsActviced = false;

	if (reply) {
		IsActviced = strcmp("PONG", reply->str) == 0 ? true : false;

		freeReply();
	}

	return IsActviced;
}
