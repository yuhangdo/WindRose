#include"MySQL_WindRose.h"
#include<mysql.h>
#include<iostream>
#include<string>
#include<vector>


PDD uv_transfor_sv(double vx, double vy)    //u��v��ת���ٷ���
{
	//����Ϊ����
	double speed = sqrt(vx * vx + vy * vy);
	double dir = -90 - atan2(vy, vx) * 57.29577951308230876798154814105;
	if (dir < 0) dir += 360;
	return { speed,dir };
}
void WindRoseMySQL::Bilinear(vector<Points>& points, vector<vector<PDD>>& windsum, vector<PDD>windData, double lon, double lat)
{

	for (int i = 0; i < windsum[0].size(); i++)
	{
		//����X�ĵ����Բ�ֵ
		double fxy1_1 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[3][i].spd + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[2][i].spd;   //���з��ٲ�ֵ
		double fxy1_2 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[3][i].dir + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[2][i].dir;  //���з����ֵ
		double fxy2_1 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[0][i].spd + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[1][i].spd;
		double fxy2_2 = (points[1].lon - lon) / (points[1].lon - points[0].lon) * windsum[0][i].dir + (lon - points[0].lon) / (points[1].lon - points[0].lon) * windsum[1][i].dir;
		//����Y����ĵ����Բ�ֵ
		//���ٲ�ֵ���
		double fxy_1 = (points[0].lat - lat) / (points[0].lat - points[3].lat) * fxy1_1 + (lat - points[3].lat) / (points[0].lat - points[3].lat) * fxy2_1;
		//�����ֵ���
		double fxy_2 = (points[0].lat - lat) / (points[0].lat - points[3].lat) * fxy1_2 + (lat - points[3].lat) / (points[0].lat - points[3].lat) * fxy2_2;
		windData.push_back({ fxy_1,fxy_2 });
	}
}

WindRoseMySQL::WindRoseMySQL()	//���캯��
{

	this->res = nullptr;	//�����������;Ͳ�д�ˣ�Ĭ�ϳ�ʼ������
}
WindRoseMySQL::~WindRoseMySQL()	//��������
{
	///< �ͷŽ����
	mysql_free_result(res);

	///< �ر����ݿ�����
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

	//�����ַ�����
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");

	//�������ݿ�
	if (mysql_real_connect(&mysql, "xxxxx", "root", "xxxx", "xxxx", 3306, NULL, 0) == NULL) {
		printf("����ԭ�� %s\n", mysql_error(&mysql));
		printf("����ʧ�ܣ�\n");
		printf("������Ҫ�������ݿ��IP��ַ�����������˿ںŵ���Ϣ \n");
		exit(-1);
	}
	else
	{
		printf("��ϲ�����ӳɹ�\n");
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
		double leftLon = (int(lon * 100) / 25) * 0.25;  //������ֵ
		double rightLon = leftLon + 0.25;					   //������ֵ
		double leftLat = (int(lat * 100) / 25) * 0.25;   //γ����ֵ
		double rightLat = leftLat + 0.25;
		points = { {leftLon,rightLat},{rightLon,rightLat},{rightLon,leftLon},{leftLon,leftLat} };

	}
	else
	{
		cout << "������ľ�γ�Ȳ��Ϸ�������������" << endl;
	}
}
void WindRoseMySQL::CreateSQL(int StartYear, int EndYear, int month, double lon, double lat)
{
	string Query_sql;   //MySQL��ѯ���
	string Month_sql = (month >= 1 && month < 10) ? ('0' + to_string(month)) : to_string(month);  //�·�sql��װ

	for (int i = StartYear; i <= EndYear; i++)
	{
		Query_sql = to_string(i) + '_' + Month_sql + ' ';
		sql += "select u10,points[0]0 from " + Query_sql + "where longitude = " + to_string(lon) + " and lat = " + to_string(lat) + " union all ";

	}
	sql = sql.substr(0, sql.size() - 10) + ';';
	cout << "sql���Ϊ��" << sql << endl;
}
void  WindRoseMySQL::QueryWindData_NB()
{
	const char* real_sql = sql.c_str();  //��Ϊ�����mysql_real_query�ĵڶ�������ֻ����const char*���ͣ�����Ҫ��stringת����const char*
	if (mysql_real_query(&mysql, real_sql, (unsigned int)strlen(real_sql)))
	{
		cout << "��ѯʧ��" << ": " << mysql_errno(&mysql) << endl;
	}
	else
	{
		cout << "��ѯ�ɹ�" << endl << endl;

		///< װ�ؽ����
		res = mysql_store_result(&mysql);
		///< װ��������ķ��ٷ���
		vector<PDD>windData;
		if (res == nullptr)
		{
			cout << "װ������ʧ��" << ": " << mysql_errno(&mysql) << endl;
		}
		else
		{
			///< ȡ�������������
			while (row = mysql_fetch_row(res))
			{
				cout << "u10�磺" << row[0] << "   v0�磺" << row[1] << endl;
				windData.push_back(uv_transfor_sv(stod(row[0]), stod(row[1])));
			}
			for (auto& x : windData)
			{
				cout << "���٣�" << x.spd << "   ����" << x.dir << endl;
			}
		}
	}
}

void WindRoseMySQL::QueryWindData_B(int StartYear, int EndYear, int month, double lon, double lat)	//ʹ��˫���Բ�ֵ
{
	vector<Points> points;
	vector<vector<PDD>>windsum;
	vector<PDD>windData;
	findFourPoints(lon, lat, points);
	for (int i = 0; i < 4; i++)
	{
		CreateSQL(StartYear, EndYear, month, points[i].lon, points[i].lat);
		const char* real_sql = sql.c_str();          //��Ϊ�����mysql_real_query�ĵڶ�������ֻ����const char*���ͣ�����Ҫ��stringת����const char*
					/// ���ò�ѯ�ӿ�
		res = nullptr;  //��Ϊ���ѭ����Ϊ�˷�ָֹ���쳣���ȳ�ʼ��Ϊ��
		if (mysql_real_query(&mysql, real_sql, (unsigned int)strlen(real_sql)))
		{
			cout << "��ѯʧ��" << ": " << mysql_errno(&mysql) << endl;
		}
		else
		{
			cout << "��ѯ�ɹ�" << endl << endl;

			///< װ�ؽ����
			res = mysql_store_result(&mysql);

			if (nullptr == res)
			{
				cout << "װ������ʧ��" << ": " << mysql_errno(&mysql) << endl;
			}
			else
			{
				///< ȡ�������������
				while (row = mysql_fetch_row(res))
				{
					cout << row[0] << "  " << row[1] << endl;
					//����ȡ����������u10��v10�����ݴ�������ݵ�������
					windsum[i].push_back(uv_transfor_sv(stod(row[0]), stod(row[1])));
				}
			}
		}
	}
	//�������ǾͰ�����sql��ѯ�Ľ�����ӵ���windsum��������Ե���ȫ�ֵ�˫���Բ�ֵ������
	Bilinear(points, windsum, windData, lon, lat);
	for (auto x : windData)
	{
		cout << "˫���Բ�ֵ��ķ��٣�" << x.spd << "   ˫���Բ�ֵ��ķ���" << x.dir;
	}

}
