#include "SyncFunc.h"
#include "MysqlHelper.h"
#include "RedisHelper.h"
#include "Log.h"
#include "ConfigDef.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

using namespace Helper;

bool is_daemon = false;


static const char* const INI_FILE = "../etc/server.ini";

EMysqlConf mysql_conf = {0};
ERedisConf redis_conf = {0};
EServerConf server_conf = {0};

pEQueueConf queue_conf = (pEQueueConf)malloc(sizeof(EQueueConf));

#if _DAEMON_ 

#define printf(fmt, args...)	log_debug(fmt, ##args)

#endif


static 
int Init();

static
void Show_Conf();

static 
void CDWorkDir(const char* path);

int
main(int argc, char** argv)
{
	char LogName[100];
	
	CDWorkDir(argv[0]);
		
#ifdef _DAEMON_
	if (daemon(1, 0) == 0) {
		log_debug("daemon running.");
		is_daemon = true;
	}
#endif

	snprintf(LogName, 100, "Log_%d", getpid());
	init_log(LogName, "../log");
	log_info("server running.\n");

	Init();
	Show_Conf();

	CMysqlHelper* mysql = new CMysqlHelper(mysql_conf.host, mysql_conf.port, mysql_conf.user, mysql_conf.passwd);
	mysql->Connect();
	mysql->UseDB(mysql_conf.db);
	if (mysql->IsConnected()) {
		printf("mysql server connected.\n");
	} else {
		printf("mysql server not connected.\n");
		return -1;
	}

	CRedisHelper* redis = new CRedisHelper(redis_conf.host, redis_conf.port, redis_conf.timeout);
	if (redis->IsActived()) {
		printf("redis server connected.\n");
	} else {
		printf("redis server not connected.\n");
		return -1;
	}

	while (1) {
		if (redis->IsActived()) {
			printf("redis server connected.\n");
		} else {
			printf("redis server not connected.\n");
			redis->Connect();
		}	

		sync_new_table(mysql); // new table?
		
		sync_game_log(redis, mysql);
		sync_user_bankruptcy_log(redis, mysql);
		sync_user_money_log(redis, mysql);
		sync_user_score_log(redis, mysql);

		usleep(server_conf.sleep_msec * 1000);
	}

	return 0;
}

int
Init()
{
	Load_Mysql_Conf(INI_FILE, &mysql_conf);
	Load_Redis_Conf(INI_FILE, &redis_conf);

	queue_conf->nums = 4;
	queue_conf->names = (char**)malloc(sizeof(char*) * queue_conf->nums);

	for (int i = 0; i < queue_conf->nums; i++)
	{
		queue_conf->names[i] = (char*)malloc(sizeof(char) * 50 ); // *(queue_conf->names + i)
		
		memset(queue_conf->names[i], 0, sizeof(queue_conf->names[i]));
	}
	
	Load_Queue_Conf(INI_FILE, queue_conf);

	Load_Server_Conf(INI_FILE, &server_conf);
}

void
Show_Conf()
{
	printf("-------------------------------\n");
	printf("mysql->host: %s\n", mysql_conf.host);
	printf("mysql->port: %d\n", mysql_conf.port);
	printf("mysql->db: %s\n", mysql_conf.db);
	printf("mysql->user: %s\n", mysql_conf.user);
	printf("mysql->passwd: %s\n", mysql_conf.passwd);
	printf("-------------------------------\n");
	printf("redis->host: %s\n", redis_conf.host);
	printf("redis->port: %d\n", redis_conf.port);
	printf("redis->timeout: %d\n", redis_conf.timeout);
	printf("-------------------------------\n");
	
	for(int i = 0; i < queue_conf->nums; i++)
	{
		printf("queue->queue: %s\n", *(queue_conf->names + i));	
	}
	printf("-------------------------------\n");
	printf("server->version: %s\n", server_conf.version);
	printf("server->sleep_msec: %d\n", server_conf.sleep_msec);
	printf("-------------------------------\n");

}

void CDWorkDir(const char * path)
{
	char* rpath = realpath(path, NULL);

	char* p = dirname(rpath);

	int i = chdir(p);

	free(rpath);
}