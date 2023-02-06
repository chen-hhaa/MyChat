#include "UserManager.h"

UserManager::UserManager()
{
	db_connect();
}

UserManager::UserManager(ConfigReader & cfg)
{
	
	host = cfg.getValue("dbHost");
	user = cfg.getValue("dbUser");
	pw = cfg.getValue("dbPassword");
	database_name = cfg.getValue("dbName");
	port = atoi(cfg.getValue("dbPort").c_str());

	db_connect();
}

UserManager::UserManager(unordered_map<string, string> & cfg_dic){
	host = cfg_dic["dbHost"];
	user = cfg_dic["dbUser"];
	pw = cfg_dic["dbPassword"];
	database_name = cfg_dic["dbName"];
	port = atoi(cfg_dic["dbPort"].c_str());

	db_connect();
}


UserManager::~UserManager()
{
	mysql_close(con);
}

bool UserManager::insert_student(User& t)
{
	char sql[1024];
	sprintf(sql, "insert into user_info (user_name, password) values('%s', '%s')",
		t.user_name.c_str(), t.password.c_str());

	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to insert data : Error: %s\n", mysql_error(con));
		return false;
	}
	return true;
}

bool UserManager::update_student(User& t)
{
	char sql[1024];
	sprintf(sql, "UPDATE user_info SET user_name = '%s', password = '%s'"
		"where user_id = %d", t.user_name.c_str(), t.password.c_str(), 
		t.user_id);

	if (mysql_query(con, sql)) 
	{
		fprintf(stderr, "Failed to update data : Error: %s\n", mysql_error(con));
		return false;
	}
	return true;
}

bool UserManager::delete_student(int user_id)
{
	// 通过主键删除
	char sql[1024];
	sprintf(sql, "DELETE FROM user_info WHERE student_id=%d", user_id);

	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to delete data : Error: %s\n", mysql_error(con));
		return false;
	}
	return true;
}

vector<User> UserManager::get_user_info(string condition)
{
	vector<User> stuList;
	char sql[1024];
	sprintf(sql, "SELECT * FROM user_info %s", condition.c_str());

	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to get data : Error: %s\n", mysql_error(con));
		return {};
	}
	MYSQL_RES* res = mysql_store_result(con);
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(res))) 
	{
		User user;
		user.user_id = atoi(row[0]);
		user.user_name = row[1];
		user.password = row[2];
		stuList.push_back(user);
	}

	return stuList;
}

string UserManager::get_password(string &user_name)
{
	char sql[1024];
	sprintf(sql, "SELECT password FROM user_info where user_name='%s'", user_name.c_str());
	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to get data : Error: %s\n", mysql_error(con));
		return {};
	}
	MYSQL_RES* res = mysql_store_result(con);
	MYSQL_ROW row;

	//TODO： 判断返回结果的数量
	if((row = mysql_fetch_row(res)))
	{
		return row[0];
	}
	else
	{
		return "";
	}
}

void UserManager::db_connect()
{
	// 初始化一个MYSQL连接
	con = mysql_init(NULL);
	// 设置字符编码方式
	mysql_options(con, MYSQL_SET_CHARSET_NAME, "UTF8");
	// 开始连接MySql
	if (!mysql_real_connect(con, host.c_str(), user.c_str(), pw.c_str(), 
	database_name.c_str(), port, NULL, 0))
	{
		fprintf(stderr, "Failed to connect to database Error: %s\n", mysql_error(con));
		exit(1);
	}
	else cout << "mysql 数据库连接成功！" << endl;
}
