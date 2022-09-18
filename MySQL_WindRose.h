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
struct PDD  //��ʾ���ٷ����һ���ṹ��
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
	void OnInit(double minLon, double lonDif, double minLat, double latDif);    //��ʼ����γ����صı���
	bool DataIsTrue(double lon, double lat);   //�жϾ�γ�������Ƿ�Ϸ�
	void Bilinear(vector<Points>& points, vector<vector<PDD>>& windsum, vector<PDD>windData, double lon, double lat);		//˫���Բ�ֵ
	void findFourPoints(double lon, double lat, vector<Points>& points);

	void ConectMySQL();					   //����mysql
	void CreateSQL(int StartYear, int EndYear, int month, double lon, double lat);  //��������װsql���

	void QueryWindData_NB();			   //��ʹ��˫���Բ�ֵ��ѯ�ض���γ�ȷ��ٷ�������
	void QueryWindData_B(int StartYear, int EndYear, int month, double lon, double lat);				   //ʹ��˫���Բ�ֵ��ѯ�ض���γ�ȵķ��ٷ�������




private:
	//WindRoseMySQL* m_sqlData
	double minLon;
	double lonDif;
	double minLat;
	double latDif;
	MYSQL mysql;    //���ݿ���
	///< �������ݿ��Ӧ�ṹ��
	MYSQL_RES* res = nullptr;
	///< ������Ž���Ľṹ��
	MYSQL_ROW row;
	//sql���
	string sql;
};
