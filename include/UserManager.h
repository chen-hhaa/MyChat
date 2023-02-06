#pragma once
#include<mysql/mysql.h>
#include<iostream>
#include<string>
#include<vector>

#include "ConfigReader.h"

using namespace std;

typedef struct User
{
	int user_id;
	string user_name;
	string password;
}User;

class UserManager
{
	// 构造和析构函数
	UserManager();
	UserManager(ConfigReader & cfg);
	UserManager(unordered_map<string, string> & cfg_dic);
	~UserManager();
public:
	/*设置单例模式*/
	static UserManager* GetInstance(ConfigReader & cfg) {
		static UserManager UserManager(cfg);
		return &UserManager;
	}
	static UserManager* GetInstance(unordered_map<string, string> & cfg_dic) {
		static UserManager UserManager(cfg_dic);
		return &UserManager;
	}
public:
	/*实现增删查改功能*/

	bool insert_student(User& t);
	bool update_student(User& t);
	bool delete_student(int user_id); // 通过id删除
	vector<User> get_user_info(string condition = "");
	string get_password(string &user_name);
private:
	MYSQL* con;
	string host = "127.0.0.1";
	string  user = "VM-centOS";
	string  pw = "@Faith520";	
	string  database_name = "MyChatServer";
	int  port = 3306;
private:
	void db_connect();
};

