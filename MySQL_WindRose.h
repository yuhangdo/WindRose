#pragma once
#include<iostream>
#include<mysql.h>
#include<vector>

using namespace std;

struct Points
{
	double lon;
	double lat;
};
struct PDD  //表示风速风向的一个结构体
{
	double spd;
	double dir;
};
class WindRoseMySQL
{
public:

	WindRoseMySQL();
	~WindRoseMySQL();
	void QueryDirect(int StartYear, int EndYear, int month, double lon, double lat);
	void QueryBilinera(int StartYear, int EndYear, int month, double lon, double lat);

private:
	void OnInit(double minLon, double lonDif, double minLat, double latDif);    //初始化经纬度相关的变量
	bool DataIsTrue(double lon, double lat);   //判断经纬度输入是否合法
	void Bilinear(vector<Points>& points, vector<vector<PDD>>& windsum, vector<PDD>windData, double lon, double lat);		//双线性插值
	void findFourPoints(double lon, double lat, vector<Points>& points);

	void ConectMySQL();					   //连接mysql
	void CreateSQL(int StartYear, int EndYear, int month, double lon, double lat);  //创建并封装sql语句

	void QueryWindData_NB();			   //不使用双线性插值查询特定经纬度风速风向数据
	void QueryWindData_B(int StartYear, int EndYear, int month, double lon, double lat);				   //使用双线性插值查询特定经纬度的风速风向数据




private:
	//WindRoseMySQL* m_sqlData
	double minLon;
	double lonDif;
	double minLat;
	double latDif;
	MYSQL mysql;    //数据库句柄
	///< 创建数据库回应结构体
	MYSQL_RES* res = nullptr;
	///< 创建存放结果的结构体
	MYSQL_ROW row;
	//sql语句
	string sql;
};
