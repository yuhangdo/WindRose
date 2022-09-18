#include"MySQL_WindRose.h"
#include<mysql.h>
#include<iostream>
#include<string>
#include<vector>


PDD uv_transfor_sv(double vx, double vy)    //u风v风转风速风向
{
	//风向为来向
	double speed = sqrt(vx * vx + vy * vy);
	double dir = -90 - atan2(vy, vx) * 57.29577951308230876798154814105;
	if (dir < 0) dir += 360;
	return { speed,dir };
}
void WindRoseMySQL::Bilinear(vector<Points>& points, vector<vector<PDD>>& windsum, vector<PDD>windData, double lon, double lat)
{

	for (int i = 0; i < windsum[0].size(); i++)
	{
		//关于X的单线性插值
		double fxy1_1 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[3][i].spd + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[2][i].spd;   //进行风速插值
		double fxy1_2 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[3][i].dir + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[2][i].dir;  //进行风向插值
		double fxy2_1 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[0][i].spd + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[1][i].spd;
		double fxy2_2 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[0][i].dir + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[1][i].dir;
		//关于Y方向的单线性插值
		//风速插值结果
		double fxy_1 = (points[0].lat - lat) / (points[0].lat - points[3].lat) * fxy1_1 + (lat - points[3].lat) / (points[0].lat - points[3].lat) * fxy2_1;
		//风向插值结果
		double fxy_2 = (points[0].lat - lat) / (points[0].lat - points[3].lat) * fxy1_2 + (lat - points[3].lat) / (points[0].lat - points[3].lat) * fxy2_2;
		windData.push_back({ fxy_1,fxy_2 });
	}
}

WindRoseMySQL::WindRoseMySQL()	//构造函数
{

	this->res = nullptr;	//其他基本类型就不写了，默认初始化即可
}
WindRoseMySQL::~WindRoseMySQL()	//析构函数
{
	///< 释放结果集
	mysql_free_result(res);

	///< 关闭数据库连接
	mysql_close(&mysql);
	this->res = nullptr;
}

void WindRoseMySQL::QueryDirect(int StartYear, int EndYear, int month, double lon, double lat)
{
	ConectMySQL();
	CreateSQL(StartYear, EndYear, month, lon, lat);
	QueryWindData_NB();

}

void WindRoseMySQL::QueryBilinera(int StartYear, int EndYear, int month, double lon, double lat)
{
	ConectMySQL();
	QueryWindData_B(StartYear, EndYear, month, lon, lat);
}

void WindRoseMySQL::OnInit(double minLon, double lonDif, double minLat, double latDif)
{
	this->minLon = minLon;
	this->lonDif = lonDif;
	this->minLat = minLat;
	this->latDif = latDif;
}

void WindRoseMySQL::ConectMySQL()
{
	mysql_init(&mysql);

	//设置字符编码
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");

	//连接数据库
	if (mysql_real_connect(&mysql, "xxxxx", "root", "xxxx", "xxxx", 3306, NULL, 0) == NULL) {
		printf("错误原因： %s\n", mysql_error(&mysql));
		printf("连接失败！\n");
		printf("请检查您要连接数据库的IP地址、主机名、端口号等信息 \n");
		exit(-1);
	}
	else
	{
		printf("恭喜你连接成功\n");
	}
}
bool WindRoseMySQL::DataIsTrue(double lon, double lat)
{
	if (lon >= 0 && lon < 360 && lat >= -90 && lat <= 90) return true;
	return false;
}

void WindRoseMySQL::findFourPoints(double lon, double lat, vector<Points>& points)
{
	if (DataIsTrue(lon, lat))
	{
		double leftLon = (int(lon * 100) / 25) * 0.25;  //经度左值
		double rightLon = leftLon + 0.25;					   //经度右值
		double leftLat = (int(lat * 100) / 25) * 0.25;   //纬度左值
		double rightLat = leftLat + 0.25;
		points = { {leftLon,rightLat},{rightLon,rightLat},{rightLon,leftLon},{leftLon,leftLat} };

	}
	else
	{
		cout << "您输入的经纬度不合法，请重新输入" << endl;
	}
}
void WindRoseMySQL::CreateSQL(int StartYear, int EndYear, int month, double lon, double lat)
{
	string Query_sql;   //MySQL查询语句
	string Month_sql = (month >= 1 && month < 10) ? ('0' + to_string(month)) : to_string(month);  //月份sql封装

	for (int i = StartYear; i <= EndYear; i++)
	{
		Query_sql = to_string(i) + '_' + Month_sql + ' ';
		sql += "select u10,points[0]0 from " + Query_sql + "where longitude = " + to_string(lon) + " and lat = " + to_string(lat) + " union all ";

	}
	sql = sql.substr(0, sql.size() - 10) + ';';
	cout << "sql语句为：" << sql << endl;
}
void  WindRoseMySQL::QueryWindData_NB()
{
	const char* real_sql = sql.c_str();  //因为下面的mysql_real_query的第二个参数只接受const char*类型，所以要把string转换成const char*
	if (mysql_real_query(&mysql, real_sql, (unsigned int)strlen(real_sql)))
	{
		cout << "查询失败" << ": " << mysql_errno(&mysql) << endl;
	}
	else
	{
		cout << "查询成功" << endl << endl;

		///< 装载结果集
		res = mysql_store_result(&mysql);
		///< 装载最后结果的风速风向
		vector<PDD>windData;
		if (res == nullptr)
		{
			cout << "装载数据失败" << ": " << mysql_errno(&mysql) << endl;
		}
		else
		{
			///< 取出结果集中内容
			while (row = mysql_fetch_row(res))
			{
				cout << "u10风：" << row[0] << "   v0风：" << row[1] << endl;
				windData.push_back(uv_transfor_sv(stod(row[0]), stod(row[1])));
			}
			for (auto& x : windData)
			{
				cout << "风速：" << x.spd << "   风向：" << x.dir << endl;
			}
		}
	}
}

void WindRoseMySQL::QueryWindData_B(int StartYear, int EndYear, int month, double lon, double lat)	//使用双线性插值
{
	vector<Points> points;
	vector<vector<PDD>>windsum;
	vector<PDD>windData;
	findFourPoints(lon, lat, points);
	for (int i = 0; i < 4; i++)
	{
		CreateSQL(StartYear, EndYear, month, points[i].lon, points[i].lat);
		const char* real_sql = sql.c_str();          //因为下面的mysql_real_query的第二个参数只接受const char*类型，所以要把string转换成const char*
					/// 调用查询接口
		res = nullptr;  //因为多次循环，为了防止指针异常，先初始化为空
		if (mysql_real_query(&mysql, real_sql, (unsigned int)strlen(real_sql)))
		{
			cout << "查询失败" << ": " << mysql_errno(&mysql) << endl;
		}
		else
		{
			cout << "查询成功" << endl << endl;

			///< 装载结果集
			res = mysql_store_result(&mysql);

			if (nullptr == res)
			{
				cout << "装载数据失败" << ": " << mysql_errno(&mysql) << endl;
			}
			else
			{
				///< 取出结果集中内容
				while (row = mysql_fetch_row(res))
				{
					cout << row[0] << "  " << row[1] << endl;
					//将读取的行数据中u10和v10风数据存入风数据的数组中
					windsum[i].push_back(uv_transfor_sv(stod(row[0]), stod(row[1])));
				}
			}
		}
	}
	//现在我们就把四条sql查询的结果都加到了windsum数组里，可以调用全局的双线性插值函数了
	Bilinear(points, windsum, windData, lon, lat);
	for (auto x : windData)
	{
		cout << "双线性插值求的风速：" << x.spd << "   双线性插值求的风向：" << x.dir;
	}

}
